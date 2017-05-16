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

#include "binderd/transaction_data_from_message.h"
#include "binderd/binder_api.h"
#include "binderd/logger.h"

namespace binderd {
struct TransactionDataFromMessage::Implementation {
  Implementation(const MessagePtr &msg) : msg{msg} {
    auto reader = msg->GetReader();

    reader.SetPadding(false);

    binder = static_cast<uintptr_t>(reader.ReadUint64());
    cookie = static_cast<uintptr_t>(reader.ReadUint64());
    code = reader.ReadUint32();
    flags = reader.ReadUint32();
    data = reader.ReadSizedData();
    objects = reader.ReadSizedData();
    num_objects = objects->GetSize() / sizeof(binder_size_t);
  }

  MessagePtr msg;
  uintptr_t binder = 0;
  uintptr_t cookie = 0;
  uint32_t code = 0;
  uint32_t flags = 0;
  BufferPtr data;
  BufferPtr objects;
  size_t num_objects = 0;
};

TransactionDataFromMessage::TransactionDataFromMessage(const MessagePtr &msg) :
  impl{new Implementation{msg}} {}

TransactionDataFromMessage::~TransactionDataFromMessage() {}

uint8_t* TransactionDataFromMessage::GetMutableData() const {
  return impl->data->GetData();
}

uintptr_t TransactionDataFromMessage::GetBinder() const {
  return impl->binder;
}

uintptr_t TransactionDataFromMessage::GetCookie() const {
  return impl->cookie;
}

uint32_t TransactionDataFromMessage::GetCode() const {
  return impl->code;
}

bool TransactionDataFromMessage::IsOneWay() const {
  return (impl->flags & static_cast<uint32_t>(Flags::OneWay));
}

bool TransactionDataFromMessage::HasStatus() const {
  return (impl->flags & static_cast<uint32_t>(Flags::HasStatus));
}

Status TransactionDataFromMessage::GetStatus() const {
  return static_cast<Status>(*reinterpret_cast<const int32_t*>(GetData()));
}

const uint8_t* TransactionDataFromMessage::GetData() const {
  return impl->data->GetData();
}

size_t TransactionDataFromMessage::GetDataSize() const {
  return impl->data->GetSize();
}

const uint8_t* TransactionDataFromMessage::GetObjectOffsets() const {
  return impl->objects->GetData();
}

size_t TransactionDataFromMessage::GetNumObjectOffsets() const {
  return impl->num_objects;
}
} // namespace binderd
