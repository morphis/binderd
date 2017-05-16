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

#include "binderd/parcel_transaction_data_reader.h"
#include "binderd/writable_transaction_data.h"
#include "binderd/binder_api.h"
#include "binderd/constants.h"
#include "binderd/common/binary_reader.h"
#include "binderd/common/utils.h"
#include "binderd/remote_object.h"
#include "binderd/local_object.h"
#include "binderd/logger.h"
#include "binderd/client.h"

#include <codecvt>
#include <locale>

namespace binderd {
struct ParcelTransactionDataReader::Implementation {
  Implementation(std::unique_ptr<TransactionData> &td) :
    transaction_data{std::move(td)},
    data{new Buffer(transaction_data->GetMutableData(), transaction_data->GetDataSize())},
    data_reader(data) {}

  std::unique_ptr<TransactionData> transaction_data;
  BufferPtr data;
  BinaryReader data_reader;
};

ParcelTransactionDataReader::ParcelTransactionDataReader(std::unique_ptr<TransactionData> &data) :
  impl{new Implementation{data}} {}

ParcelTransactionDataReader::~ParcelTransactionDataReader() {}

bool ParcelTransactionDataReader::EnforceInterface(const std::string &name) {
  const auto strict_mode_policy = impl->data_reader.ReadInt32();
  WARNING("FIXME: provided strict mode policy (%d) is not yet handled", strict_mode_policy);
  const auto provided_name = utils::to_string8(impl->data_reader.ReadString16());
  return (provided_name == name);
}

int32_t ParcelTransactionDataReader::ReadInt32() {
  return impl->data_reader.ReadInt32();
}

std::u16string ParcelTransactionDataReader::ReadString16() {
  return impl->data_reader.ReadString16();
}

std::shared_ptr<Object> ParcelTransactionDataReader::ReadObject(const std::shared_ptr<Client> &client) {
  auto buffer = impl->data_reader.ReadData(sizeof(flat_binder_object));
  auto flat = reinterpret_cast<flat_binder_object*>(buffer->GetData());

  switch (flat->type) {
  case BINDER_TYPE_HANDLE:
    return RemoteObject::Create(client, flat->handle);
  case BINDER_TYPE_BINDER: {
    if (!client)
      return std::make_shared<LocalObject>(nullptr);

    auto object_pool = client->GetObjectPool();
    return object_pool->GetObject(flat->cookie);
  }
  default:
    break;
  }

  return nullptr;
}
} // namespace binderd
