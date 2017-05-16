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

#include "binderd/client_impl.h"
#include "binderd/message_parser.h"
#include "binderd/logger.h"
#include "binderd/common/variable_length_array.h"
#include "binderd/transaction_data_from_message.h"
#include "binderd/transaction_status.h"
#include "binderd/binder_api.h"
#include "binderd/remote_object.h"

#include <future>
#include <functional>
namespace {
constexpr const unsigned int max_binder_threads{15};
}

namespace binderd {
ClientImpl::ClientImpl(const std::shared_ptr<asio::io_service> &io,
                       const std::shared_ptr<Messenger> &messenger,
                       const std::string &socket_path,
                       bool connect,
                       std::size_t num_workers) :
  state_(ClientImpl::State::Disconnected),
  io_(io),
  strand_(*io_),
  messenger_(messenger),
  next_cookie_{0},
  object_pool_{std::make_shared<ObjectPool>()},
  object_translator_{std::make_shared<ObjectTranslator>()} {

  if (!connect)
    return;

  Connect(socket_path);

  for (unsigned int i = 0; i < num_workers; i++) {
    io_workers_.push_back(std::thread{[this]() {
      while (!should_exit_) {
       try {
         io_->run();
       } catch (std::exception &err) {
         ERROR("%s", err.what());
       }
      }
    }});
  }

  running_ = true;
}

ClientImpl::~ClientImpl() {
  if (running_) {
    Terminate();
    for (auto &worker : io_workers_) {
      if (worker.joinable())
        worker.join();
    }
  }
}

void ClientImpl::Connect(const std::string &socket_path) {
  if (!messenger_->Connect(socket_path)) {
    ERROR("Failed to connect to binder server");
    return;
  }
  state_ = ClientImpl::State::Connected;
  ReadNextMessage();
}

void ClientImpl::Terminate() {
  should_exit_ = true;
  messenger_->Close();
  can_read_.notify_all();
  io_->stop();
}

void ClientImpl::ReadNextMessage() {
  messenger_->ReadMessageAsync(std::bind(&ClientImpl::OnNewMessage, this,
                                         std::placeholders::_1,
                                         std::placeholders::_2));
}

void ClientImpl::OnNewMessage(const asio::error_code &err, const BufferPtr &buffer) {
  if (err) {
    Terminate();
    return;
  }

  std::unique_lock<std::mutex> l(read_lock_);
  MessageParser parser(buffer, buffer->GetSize());
  while (auto msg = parser.Next()) {
    if (msg->GetType() == Message::Type::LogEntry) {
      if (log_message_handler_) {
        auto reader = msg->GetReader();
        auto buffer = reader.ReadSizedData();
        auto real_msg = Message::CreateFromData(buffer);
        log_message_handler_->OnLogMessage(real_msg);
      }
      continue;
    }

    const auto reply_cookie = msg->GetCookie();
    auto pending = pending_calls_.find(reply_cookie);
    if (pending != pending_calls_.end()) {
      auto call = std::get<1>(*pending);

      // Only transaction messages are acknowledged on reception on the server
      // side with a status message but still need to await their response from
      // the remote object.
      if (msg->GetType() == Message::Type::Status &&
          call.msg_type == Message::Type::Transaction &&
          !call.one_way) {
        auto reader = msg->GetReader();
        if (static_cast<Status>(reader.ReadInt32()) == Status::OK)
          continue;
      }

      auto promise = call.promise;
      promise->set_value(msg);
      strand_.post([this, reply_cookie]() { FinishCall(reply_cookie); });
    } else if (msg->GetType() == Message::Type::Transaction && context_obj_) {
      strand_.post([this, msg]() { HandleContextObjectMessage(msg); });
    } else {
      in_messages_.push_back(msg);
      can_read_.notify_one();
    }
  }

  ReadNextMessage();
}

void ClientImpl::HandleContextObjectMessage(const MessagePtr &msg) {
  if (msg->GetType() != Message::Type::Transaction)
    return;

  auto request = std::make_unique<TransactionDataFromMessage>(msg);
  std::unique_ptr<TransactionData> reply;
  auto status = context_obj_->Transact(shared_from_this(),
                                       std::move(request), &reply);


  auto rmsg = Message::Create(Message::Type::TransactionReply);
  rmsg->SetCookie(msg->GetCookie());
  auto writer = rmsg->GetWriter();

  if (status != Status::OK) {
    TransactionStatus status_reply(status);
    writer.WriteData(status_reply.Pack());
  } else {
    writer.WriteData(reply->Pack());
  }

  messenger_->WriteData(rmsg->Pack());
}

void ClientImpl::SendStatus(const MessagePtr &msg, const Status &status) {
  auto reply = Message::Create(Message::Type::Status);
  reply->SetCookie(msg->GetCookie());
  auto writer = reply->GetWriter();
  writer.WriteInt32(static_cast<int32_t>(status));
  messenger_->WriteData(reply->Pack());
}

void ClientImpl::FinishCall(std::uint64_t cookie) {
  pending_calls_.erase(cookie);
}

ClientImpl::CallPromisePtr ClientImpl::StartCall(const MessagePtr &msg, bool one_way) {
  msg->SetCookie(NewCookie());

  if (pending_calls_.find(msg->GetCookie()) != pending_calls_.end()) {
    // FIXME maybe we should do a bit better here and try a few more
    // handles until we find one we don't track yet. The uint64_t should
    // flip at somepoint (if we have a overrun) and simply start again.
    throw std::runtime_error("Message with same cookie is already waiting for a reply");
  }

  auto p = std::make_shared<std::promise<MessagePtr>>();
  pending_calls_.insert({msg->GetCookie(), {p, msg->GetType(), one_way}});

  return p;
}

ClientImpl::State ClientImpl::GetState() const {
  return state_;
}

void ClientImpl::QueueMessage(const MessagePtr &msg) {
  if (should_exit_ || !running_)
    throw std::runtime_error("Not connected");

  messenger_->WriteData(msg->Pack());
}

MessagePtr ClientImpl::DequeueMessage() {
  std::unique_lock<std::mutex> l(read_lock_);
  while (in_messages_.size() == 0 && !should_exit_)
    can_read_.wait(l);
  if (should_exit_)
    return nullptr;
  auto msg = in_messages_.front();
  in_messages_.pop_front();
  return msg;
}

MessagePtr ClientImpl::Transact(const MessagePtr &msg, bool one_way) {
  if (state_ != State::Connected)
    return nullptr;

  auto p = StartCall(msg, one_way);

  QueueMessage(msg);

  auto ft = p->get_future();
  ft.wait();

  return ft.get();
}

Status ClientImpl::Transact(int32_t handle, std::unique_ptr<TransactionData> request, std::unique_ptr<TransactionData> *reply) {
  if (!request || (!request->IsOneWay() && !reply))
    return Status::BadValue;

  auto msg = Message::Create(Message::Type::Transaction);
  msg->SetDestination(handle);
  msg->SetCookie(NewCookie());

  object_translator_->ProcessTransaction(request.get(), ObjectTranslator::Direction::Out);

  msg->GetWriter().WriteData(request->Pack());
  auto rmsg = Transact(msg, request->IsOneWay());
  if (!rmsg)
    return Status::FailedTransaction;
  else if (request->IsOneWay() && rmsg->GetType() == Message::Type::Status) {
    auto reader = rmsg->GetReader();
    return static_cast<Status>(reader.ReadInt32());
  }

  if (rmsg->GetType() != Message::Type::TransactionReply)
    return Status::BadValue;

  auto reply_data = std::make_unique<TransactionDataFromMessage>(rmsg);
  if (reply_data->HasStatus())
    return reply_data->GetStatus();

  object_translator_->ProcessTransaction(reply_data.get(), ObjectTranslator::Direction::In);

  *reply = std::move(reply_data);

  return Status::OK;
}

uint64_t ClientImpl::NewCookie() {
  return next_cookie_++;
}

Status ClientImpl::AddReference(uint64_t handle) {
  auto msg = Message::Create(Message::Type::Acquire);
  if (!msg) {
    WARNING("Failed to acquire reference for handle %d: no memory", handle);
    return Status::NoMemory;
  }

  auto writer = msg->GetWriter();
  writer.WriteUint64(handle);

  auto reply = Transact(msg);
  if (!reply) {
    WARNING("Failed to acquire reference for handle %d: received no reply", handle);
    return Status::FailedTransaction;
  }

  auto reader = reply->GetReader();
  return static_cast<Status>(reader.ReadInt32());
}

Status ClientImpl::ReleaseReference(uint64_t handle) {
  auto msg = Message::Create(Message::Type::Release);
  if (!msg) {
    WARNING("Failed to acquire reference for handle %d: no memory", handle);
    return Status::NoMemory;
  }

  auto writer = msg->GetWriter();
  writer.WriteUint64(handle);

  auto reply = Transact(msg);
  if (!reply) {
    WARNING("Failed to acquire reference for handle %d: received no reply", handle);
    return Status::FailedTransaction;
  }

  auto reader = reply->GetReader();
  return static_cast<Status>(reader.ReadInt32());
}

Client::ReleaseReferenceFunc ClientImpl::GetReleaseReferenceClosure(uint64_t handle) {
  std::weak_ptr<ClientImpl> client = shared_from_this();
  return [client, handle]() {
    if (auto inst = client.lock())
      return inst->ReleaseReference(handle);
    return Status::DeadObject;
  };
}

Status ClientImpl::BecomeContextManager() {
  auto msg = Message::Create(Message::Type::SetContextMgr);
  if (!msg)
    return Status::NoMemory;

  auto reply = Transact(msg);
  if (!reply)
    return Status::UnknownError;

  auto reader = reply->GetReader();
  auto status = static_cast<Status>(reader.ReadInt32());
  return status;
}

void ClientImpl::SetContextObject(const std::shared_ptr<Object> &object) {
  context_obj_ = object;
}

Status ClientImpl::RequestDeathNotification(uint64_t handle, const std::shared_ptr<Object> &proxy) {
  auto msg = Message::Create(Message::Type::RequestDeathNotification);
  if (!msg)
    return Status::NoMemory;

  auto writer = msg->GetWriter();
  writer.WriteUint64(handle);

  auto id = object_translator_->TranslateOrAddObject(reinterpret_cast<uintptr_t>(proxy.get()));
  auto iter = std::find_if(dead_recipients_.begin(), dead_recipients_.end(),
                           [id](const auto &item) { return item.id == id; });
  if (iter == dead_recipients_.end())
    dead_recipients_.push_back({ id, proxy, { handle }});
  else
    iter->handles.push_back(handle);

  writer.WriteUint64(id);

  auto reply = Transact(msg);
  if (!reply)
    return Status::UnknownError;

  auto reader = reply->GetReader();
  return static_cast<Status>(reader.ReadInt32());
}

Status ClientImpl::ClearDeathNotification(uint64_t handle, const std::shared_ptr<Object> &proxy) {
  auto id = object_translator_->TranslateObject(reinterpret_cast<uintptr_t>(proxy.get()));
  if (id == ObjectTranslator::Invalid)
    return Status::DeadObject;

  auto iter = std::find_if(dead_recipients_.begin(), dead_recipients_.end(),
                           [id](const auto &item) { return item.id == id; });
  if (iter == dead_recipients_.end())
    return Status::DeadObject;

  // Drop the handle from the recipient as we're not going to
  // get any further death notifications for it
  auto handle_iter = std::find(iter->handles.begin(),
                               iter->handles.end(),
                               handle);
  if (handle_iter != iter->handles.end())
    iter->handles.erase(handle_iter);

  if (iter->handles.size() == 0)
    dead_recipients_.erase(iter);

  auto msg = Message::Create(Message::Type::ClearDeathNotification);
  auto writer = msg->GetWriter();
  writer.WriteUint64(handle);
  writer.WriteUint64(id);

  auto reply = Transact(msg);
  if (!reply)
    return Status::UnknownError;

  auto reader = reply->GetReader();
  return static_cast<Status>(reader.ReadInt32());
}

Status ClientImpl::BecomeMonitor() {
  auto msg = Message::Create(Message::Type::SetMonitor);
  if (!msg)
    return Status::NoMemory;

  auto reply = Transact(msg);
  if (!reply)
    return Status::UnknownError;

  auto reader = reply->GetReader();
  auto status = static_cast<Status>(reader.ReadInt32());
  return status;
}

void ClientImpl::SetLogMessageHandler(const std::shared_ptr<LogMessageHandler> &handler) {
  log_message_handler_ = handler;
}

void ClientImpl::StartThreadPool() {
  for (int n = 0; n < max_binder_threads; n++) {
    workers_.push_back(std::thread([this]() {
      while (!should_exit_)
        ProcessAndExecuteCommand();
    }));
  }
}

void ClientImpl::JoinThreadPool() {
  for (auto &worker : workers_) {
    if (worker.joinable())
      worker.join();
  }
}

bool ClientImpl::ProcessAndExecuteCommand() {
  auto msg = DequeueMessage();
  if (!msg)
    return false;

  switch (msg->GetType()) {
  case Message::Type::Transaction: {
    auto request = std::make_unique<TransactionDataFromMessage>(msg);

    // Translate all passed objects back to real objects if we
    // own them. Otherwise we just leave things as they are.
    object_translator_->ProcessTransaction(request.get(), ObjectTranslator::Direction::In);

    // Find the right object we hand the transaction to. This
    // is either specified by the cookie or the context object
    // if we don't have a valid cookie.
    auto cookie = object_translator_->TranslateCookie(request->GetCookie());
    std::shared_ptr<Object> obj;
    if (cookie)
      obj = object_pool_->GetObject(cookie);
    else if (context_obj_)
      obj = context_obj_;

    if (!obj) {
      // UnknownTransaction is what IPCThreadState sends when no object is found
      WARNING("Didn't found an object for transaction (cookie=%d)", request->GetCookie());
      SendStatus(msg, Status::UnknownTransaction);
      break;
    }

    std::unique_ptr<TransactionData> reply;
    std::shared_ptr<Client> c(this, [](auto p) { });
    const auto status = obj->Transact(shared_from_this(), std::move(request), &reply);

    auto rmsg = Message::Create(Message::Type::TransactionReply);
    rmsg->SetCookie(msg->GetCookie());

    auto writer = rmsg->GetWriter();
    if (status != Status::OK) {
      TransactionStatus data(static_cast<Status>(status));
      writer.WriteData(data.Pack());
    } else {
      if (reply) {
        // If the reply passes any objects to the other side, we have
        // to ensure they are properly translated so that we never
        // pass real memory addresses.
        object_translator_->ProcessTransaction(reply.get(), ObjectTranslator::Direction::Out);
      } else {
        reply = std::make_unique<TransactionStatus>(Status::OK);
      }
      writer.WriteData(reply->Pack());
    }

    QueueMessage(rmsg);
    break;
  }
  case Message::Type::DeadBinder: {
    auto reader = msg->GetReader();
    const auto handle = reader.ReadUint64();
    const auto id = static_cast<uintptr_t>(reader.ReadUint64());

    auto status = Status::DeadObject;
    auto iter = dead_recipients_.begin();
    while (iter != dead_recipients_.end()) {
      if (iter->id != id) {
        iter++;
        continue;
      }

      auto obj = iter->proxy;
      if (!obj || obj->GetType() != Object::Type::Remote)
        break;

      obj->SendObituary();
      status = Status::OK;
      dead_recipients_.erase(iter);
      break;
    }

    SendStatus(msg, status);
    break;
  }
  default:
    break;
  }

  return true;
}

ObjectPoolPtr ClientImpl::GetObjectPool() {
  return object_pool_;
}
} // namespace binderd
