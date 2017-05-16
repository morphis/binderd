/*
 * Copyright (C) 2016 Simon Fels <morphis@gravedo.de>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "binderd/server_session.h"
#include "binderd/server.h"
#include "binderd/message.h"
#include "binderd/message_parser.h"
#include "binderd/logger.h"
#include "binderd/registry.h"
#include "binderd/transaction_status.h"
#include "binderd/transaction_data_from_message.h"
#include "binderd/writable_transaction_data.h"

#include "binderd/binder_api.h"

namespace {
static constexpr std::size_t default_buffer_size{1024};
static std::atomic<unsigned int> next_session_id{0};
}

namespace binderd {
std::shared_ptr<ServerSession> ServerSession::Create(
    const std::shared_ptr<ServerSessions> &sessions,
    const std::shared_ptr<asio::io_service> &io,
    const std::shared_ptr<Messenger> &messenger,
    const std::shared_ptr<Registry> &registry) {
  auto session = std::shared_ptr<ServerSession>(new ServerSession(sessions, io, messenger, registry));
  return session;
}

ServerSession::ServerSession(
    const std::shared_ptr<ServerSessions> &sessions,
    const std::shared_ptr<asio::io_service> &io,
    const std::shared_ptr<Messenger> &messenger,
    const std::shared_ptr<Registry> &registry) :
  sessions_{sessions},
  messenger_{messenger},
  read_strand_(*io),
  registry_(registry),
  next_transaction_cookie_{0},
  next_node_handle_{0},
  id_{next_session_id++} {}

ServerSession::~ServerSession() {}

unsigned int ServerSession::GetId() const { return id_; }

void ServerSession::Start() {
  ReadNextMessage();
}

void ServerSession::Terminate() {
  // Tell everyone that all objects we provide are dead now
  for (const auto &iter : objects_)
    registry_->RemoveHandle(iter.first);

  if (registry_->GetContextManager() == shared_from_this())
    registry_->SetContextManager(nullptr);

  if (registry_->GetMonitor() == shared_from_this())
    registry_->SetMonitor(nullptr);

  messenger_->Flush();
  messenger_->Close();
  sessions_->Remove(GetId());
}

void ServerSession::ReadNextMessage() {
  messenger_->ReadMessageAsync([this](const asio::error_code &err, const BufferPtr &buffer) {
    if (err) {
      Terminate();
      return;
    }

    MessageParser parser(buffer, buffer->GetSize());
    while (auto msg = parser.Next())
      ProcessMessage(msg);

    ReadNextMessage();
  });
}

void ServerSession::WriteMessage(const MessagePtr &msg) {
  messenger_->WriteData(msg->Pack());
  ForwardToMonitor(msg);
}

void ServerSession::OnSetContextManager(const MessagePtr &msg) {
  ForwardToMonitor(msg);

  auto status = Status::AlreadyExists;
  if (!registry_->GetContextManager()) {
    registry_->SetContextManager(shared_from_this());
    status = Status::OK;
  }

  SendStatus(msg, status);
}

void ServerSession::OnSetMonitor(const MessagePtr &msg) {
  auto status = Status::AlreadyExists;
  if (!registry_->GetMonitor()) {
    registry_->SetMonitor(shared_from_this());
    status = Status::OK;
  }

  SendStatus(msg, status);
}

Status ServerSession::PostTransaction(const TransactionRecord &record) {
  const auto tr_cookie = next_transaction_cookie_++;

  auto old_msg = record.message;

  WritableTransactionData new_tr;
  auto new_msg = Message::Create(Message::Type::Transaction);
  new_msg->SetCookie(tr_cookie);
  new_msg->SetDestination(old_msg->GetDestination());

  auto iter = objects_.find(old_msg->GetDestination());
  if (iter != objects_.end()) {
    auto obj = iter->second;
    new_tr.SetBinder(obj.binder);
    new_tr.SetCookie(obj.cookie);
  } else if (old_msg->GetDestination() == 0) {
    // The context manager as destination is a special case as we don't
    // store the object handle for it but just forward all messages we
    // get without binder/cookie set.
  } else {
    // If this happens then there is really something with the synchronization
    // between the registry and the object map wrong. A handle which doesn't
    // point to any object anymore should have been remove from the registry.
    WARNING("Cannot find object for handle %d", old_msg->GetDestination());
    return Status::InvalidOperation;
  }

  TransactionDataFromMessage old_tr(old_msg);
  new_tr.SetCode(old_tr.GetCode());
  new_tr.SetData(Buffer::Create(reinterpret_cast<uint8_t*>(old_tr.GetMutableData()),
                                old_tr.GetDataSize()));
  // FIXME we do not really want a const_cast<> here..
  auto objects = Buffer::Create(const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(old_tr.GetObjectOffsets())),
                                old_tr.GetNumObjectOffsets() * sizeof(binder_size_t));
  new_tr.SetObjectOffsets(objects);

  auto status = TranslateObjects(new_tr);
  if (status != Status::OK)
    return status;

  auto writer = new_msg->GetWriter();
  writer.WriteData(new_tr.Pack());

  pending_transactions_.insert({tr_cookie, record});

  WriteMessage(new_msg);

  return Status::OK;
}

Status ServerSession::TranslateObjects(TransactionData &tr) {
  if (tr.GetNumObjectOffsets() == 0)
    return Status::OK;

  // FIXME: We really don't want the const_cast<> here.
  Buffer objects(const_cast<uint8_t*>(tr.GetObjectOffsets()),
                 tr.GetNumObjectOffsets() * sizeof(binder_size_t));

  auto current = objects.GetBegin();
  while (current != objects.GetEnd()) {
    auto offset = static_cast<binder_size_t>(*current);
    auto obj = reinterpret_cast<flat_binder_object*>(tr.GetMutableData() + offset);

    switch (obj->type) {
    case BINDER_TYPE_BINDER: {
      const auto handle = registry_->NewObjectHandle(shared_from_this());
      objects_.insert({handle, BinderObject{obj->binder, obj->cookie}});
      obj->type = BINDER_TYPE_HANDLE;
      obj->binder = 0;
      obj->cookie = 0;
      obj->handle = handle;
      break;
    }
    case BINDER_TYPE_HANDLE: {
      // Check if we own the handle. If we don't own it we just pass
      // it the callee which then can use it for a proxy again.
      if (objects_.find(obj->handle) == objects_.end())
        break;

      const auto b = objects_[obj->handle];
      obj->type = BINDER_TYPE_WEAK_BINDER;
      obj->binder = b.binder;
      obj->cookie = b.cookie;
      obj->handle = 0;
      break;
    }
    case BINDER_TYPE_WEAK_BINDER:
    case BINDER_TYPE_WEAK_HANDLE:
      WARNING("Got a weak binder/handle which we can't handle yet");
      break;
    case BINDER_TYPE_FD:
      break;
    default:
      WARNING("Invalid object type %d", obj->type);
      return Status::InvalidOperation;
    }

    current += sizeof(binder_size_t);
  }

  return Status::OK;
}

void ServerSession::SendStatus(const MessagePtr &msg, const Status &status) {
  auto reply = Message::Create(Message::Type::Status);
  reply->SetCookie(msg->GetCookie());
  auto writer = reply->GetWriter();
  writer.WriteInt32(static_cast<int32_t>(status));
  WriteMessage(reply);
}

void ServerSession::OnTransaction(const MessagePtr &msg) {
  try {
    TransactionDataFromMessage data(msg);
    TranslateObjects(data);

    auto target = registry_->FindSessionByHandle(msg->GetDestination());
    if (!target) {
      WARNING("Cannot find a target for handle %d", msg->GetDestination());
      SendStatus(msg, Status::NameNotFound);
      return;
    }

    TransactionRecord record{shared_from_this(), msg->GetCookie(), msg};
    auto status = target->PostTransaction(record);

    // Let the caller know that we delivered the transaction
    // to the target. This keeps in mind that delivery might
    // have given a problem which is reflected by the status
    // we send back here.
    SendStatus(msg, status);
  } catch (std::out_of_range&) {
    SendStatus(msg, Status::BadValue);
  } catch (std::exception &err) {
    SendStatus(msg, Status::UnknownError);
  }
}

void ServerSession::OnReply(const MessagePtr &msg) {
  auto iter = pending_transactions_.find(msg->GetCookie());
  if (iter == pending_transactions_.end())
    return;

  auto record = iter->second;
  pending_transactions_.erase(iter);

  msg->SetCookie(record.reply_cookie);
  record.caller->WriteMessage(msg);
}

void ServerSession::OnReference(const MessagePtr &msg) {
  auto reader = msg->GetReader();

  const auto handle = reader.ReadUint64();

  auto status = Status::DeadObject;
  switch (msg->GetType()) {
  case Message::Type::Acquire:
    status = registry_->AddReferenceForHandle(shared_from_this(), handle);
    break;
  case Message::Type::Release:
    status = registry_->RemoveReferenceForHandle(shared_from_this(), handle);
    break;
  default:
    break;
  }

  SendStatus(msg, status);
}

void ServerSession::OnDeathNotification(const MessagePtr &msg) {
  auto reader = msg->GetReader();

  const auto handle = reader.ReadUint64();
  const auto object_id = reader.ReadUint64();

  auto status = Status::DeadObject;
  switch (msg->GetType()) {
  case Message::Type::RequestDeathNotification: {
    auto iter = death_notifications_.find(handle);
    if (iter != death_notifications_.end()) {
      status = Status::AlreadyExists;
      break;
    }
    death_notifications_.insert({handle, object_id});
    status = Status::OK;
    break;
  }
  case Message::Type::ClearDeathNotification: {
    auto iter = death_notifications_.find(handle);
    if (iter == death_notifications_.end()) {
      status = Status::BadValue;
      break;
    }
    death_notifications_.erase(iter);
    status = Status::OK;
    break;
  }
  default:
    status = Status::InvalidOperation;
    break;
  }

  SendStatus(msg, status);
}

void ServerSession::ForwardToMonitor(const MessagePtr &msg) {
  auto monitor = registry_->GetMonitor();
  if (!monitor)
    return;

  auto logged_msg = Message::Create(Message::Type::LogEntry);
  auto writer = logged_msg->GetWriter();
  writer.WriteSizedData(msg->Pack());
  monitor->SendMessage(logged_msg);
}

void ServerSession::SendMessage(const MessagePtr &msg) {
  messenger_->WriteData(msg->Pack());
}

void ServerSession::ProcessMessage(const MessagePtr &msg) {
  switch (msg->GetType()) {
  case Message::Type::SetContextMgr:
    OnSetContextManager(msg);
    break;
  case Message::Type::SetMonitor:
    OnSetMonitor(msg);
    break;
  case Message::Type::Transaction:
    OnTransaction(msg);
    break;
  case Message::Type::Status:
  case Message::Type::TransactionReply:
    OnReply(msg);
    break;
  case Message::Type::Acquire:
  case Message::Type::Release:
    OnReference(msg);
    break;
  case Message::Type::RequestDeathNotification:
  case Message::Type::ClearDeathNotification:
    OnDeathNotification(msg);
    break;
  default:
    WARNING("Unsupported message type %s", msg->GetType());
    SendStatus(msg, Status::InvalidOperation);
    break;
  }
}

void ServerSession::SendDeathNotification(uint64_t handle) {
  auto iter = death_notifications_.find(handle);
  if (iter == death_notifications_.end())
    return;
  auto msg = Message::Create(Message::Type::DeadBinder);
  auto writer = msg->GetWriter();
  writer.WriteUint64(handle);
  writer.WriteUint64(iter->second);
  WriteMessage(msg);
}
} // namespace binderd
