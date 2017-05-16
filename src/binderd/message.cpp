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

#include "binderd/message.h"
#include "binderd/logger.h"
#include "binderd/common/utils.h"

#include <memory.h>

namespace binderd {
struct Message::Implementation {
  Implementation(const Message::Type &type) : type{type}, cookie{0}, destination{0}, data{CreateBufferWithSize(0)} {}

  Message::Type type;
  std::uint64_t cookie;
  std::uint64_t destination;
  BufferPtr data;
};

std::shared_ptr<Message> Message::Create(const Type &type) {
  return std::shared_ptr<Message>(new Message(type));
}

std::shared_ptr<Message> Message::CreateFromData(const BufferPtr &data, const std::size_t &offset) {
  auto msg = Create();
  msg->Unpack(data, offset);
  return msg;
}

Message::Message(const Type &type) : impl{new Message::Implementation{type}} {}

Message::~Message() {}

Message::Type Message::GetType() const { return impl->type; }

std::uint64_t Message::GetCookie() const { return impl->cookie; }

std::uint64_t Message::GetDestination() const { return impl->destination; }

BinaryReader Message::GetReader() const { return BinaryReader(impl->data, 0); }

BinaryWriter Message::GetWriter() { return BinaryWriter(impl->data); }

void Message::SetCookie(std::uint64_t cookie) {
  impl->cookie = cookie;
}

void Message::SetDestination(std::uint64_t destination) {
  impl->destination = destination;
}

std::size_t Message::Unpack(const BufferPtr &data, const std::size_t &offset) {
  BinaryReader reader(data, offset);
  impl->type = static_cast<Type>(reader.ReadUint32());
  impl->destination = reader.ReadUint64();
  impl->cookie = reader.ReadUint64();
  impl->data = reader.ReadSizedData();
  return reader.GetBytesRead();
}

BufferPtr Message::Pack() const {
  auto data = CreateBufferWithSize(0);
  BinaryWriter writer(data);
  writer.WriteUint32(static_cast<std::uint32_t>(impl->type));
  writer.WriteUint64(impl->destination);
  writer.WriteUint64(impl->cookie);
  writer.WriteSizedData(impl->data);
  return data;
}

std::ostream& operator<<(std::ostream &out, const Message::Type &type) {
  switch (type) {
  case Message::Type::Unknown:
    out << "unknown";
    break;
  case Message::Type::Transaction:
    out << "transaction";
    break;
  case Message::Type::TransactionReply:
    out << "transaction-reply";
    break;
  case Message::Type::Acquire:
    out << "acquire";
    break;
  case Message::Type::Release:
    out << "release";
    break;
  case Message::Type::RequestDeathNotification:
    out << "request-death-notification";
    break;
  case Message::Type::ClearDeathNotification:
    out << "clear-death-notification";
    break;
  case Message::Type::DeadBinder:
    out << "dead-binder";
    break;
  case Message::Type::SetContextMgr:
    out << "set-context-mgr";
    break;
  case Message::Type::Version:
    out << "version";
    break;
  case Message::Type::LogEntry:
    out << "log-entry";
    break;
  case Message::Type::Status:
    out << "status";
    break;
  default:
    break;
  }
  return out;
}

std::ostream& operator<<(std::ostream &out, const Message &msg) {
  out << utils::string_format("Type=%s Cookie=%d Destination=%d", msg.GetType(), msg.GetCookie(), msg.GetDestination()) << std::endl;
  auto reader = msg.GetReader();
  auto data = reader.ReadData(reader.GetBytesLeft());
  if (data)
    out << utils::string_format("MESSAGE:\n%s", utils::hex_dump(data->GetData(), data->GetSize()));
  return out;
}
} // namespace binderd
