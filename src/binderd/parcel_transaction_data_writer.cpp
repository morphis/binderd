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

#include "binderd/parcel_transaction_data_writer.h"
#include "binderd/writable_transaction_data.h"
#include "binderd/binder_api.h"
#include "binderd/constants.h"
#include "binderd/common/binary_writer.h"
#include "binderd/common/utils.h"
#include "binderd/local_object.h"
#include "binderd/remote_object.h"
#include "binderd/client.h"
#include "binderd/logger.h"

namespace binderd {
ParcelTransactionDataWriter::ParcelTransactionDataWriter() :
  data_{CreateBufferWithSize(0)},
  data_writer_{data_},
  objects_{CreateBufferWithSize(0)},
  objects_writer_{objects_} {}

ParcelTransactionDataWriter::~ParcelTransactionDataWriter() {}

void ParcelTransactionDataWriter::SetCode(uint32_t code) {
  code_ = code;
}

void ParcelTransactionDataWriter::WriteInt32(int32_t value) {
  data_writer_.WriteInt32(value);
}

void ParcelTransactionDataWriter::WriteInterfaceToken(const std::string &name, int32_t strict_mode_policy) {
  data_writer_.WriteInt32(kStrictModePenaltyGather | strict_mode_policy);
  data_writer_.WriteString(utils::to_string16(name));
}

void ParcelTransactionDataWriter::WriteString16(const std::string &str) {
  WriteString16(utils::to_string16(str));
}

void ParcelTransactionDataWriter::WriteString16(const std::u16string &str) {
  data_writer_.WriteString(str);
}

void ParcelTransactionDataWriter::WriteObject(const std::shared_ptr<Client> &client, const std::shared_ptr<Object> &object) {
  flat_binder_object flat{};
  flat.flags = 0x7f | FLAT_BINDER_FLAG_ACCEPTS_FDS;

  auto object_pool = client->GetObjectPool();

  switch (object->GetType()) {
  case Object::Type::Local: {
    auto *local = static_cast<LocalObject*>(object.get());
    const auto cookie = reinterpret_cast<uintptr_t>(local);
    flat.type = BINDER_TYPE_BINDER;
    flat.cookie = cookie;
    flat.binder = 0;
    flat.handle = 0;

    // We add the object here to the pool but keep its reference
    // floating until the server comes back and acquires a reference.
    if (!object_pool->Contains(cookie))
      object_pool->AdoptObject(object);

    break;
  }
  case Object::Type::Remote: {
    auto *remote = static_cast<RemoteObject*>(object.get());
    flat.type = BINDER_TYPE_HANDLE;
    flat.handle = remote->GetHandle();
    break;
  }
  default:
    break;
  }

  objects_writer_.WriteUint64(data_writer_.GetBytesWritten());
  data_writer_.WriteData(reinterpret_cast<uint8_t*>(&flat), sizeof(flat_binder_object));

  // Keep a reference for ourself so the object doesn't disappear
  // as long as we live
  local_objects_.push_back(object);
}

std::unique_ptr<TransactionData> ParcelTransactionDataWriter::Finalize() {
  auto data = std::make_unique<WritableTransactionData>();
  data->SetCode(code_);
  data->SetData(data_);
  data->SetObjectOffsets(objects_);
  return std::move(data);
}
} // namespace binderd
