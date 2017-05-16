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

#ifndef BINDERD_PARCEL_TRANSACTION_DATA_WRITER_H_
#define BINDERD_PARCEL_TRANSACTION_DATA_WRITER_H_

#include "binderd/transaction_data.h"
#include "binderd/macros.h"
#include "binderd/object.h"
#include "binderd/common/binary_writer.h"
#include "binderd/common/binary_reader.h"

#include <memory>

namespace binderd {
class Client;
class ParcelTransactionDataWriter {
 public:
  explicit ParcelTransactionDataWriter();
  ~ParcelTransactionDataWriter();

  void SetCode(uint32_t code);

  void WriteInt32(int32_t value);
  void WriteInterfaceToken(const std::string &name, int32_t strict_mode_policy);
  void WriteString16(const std::u16string &str);
  void WriteString16(const std::string &str);
  void WriteObject(const std::shared_ptr<Client> &client, const std::shared_ptr<Object> &object);

  std::unique_ptr<TransactionData> Finalize();

 private:
  uint32_t code_ = 0;
  BufferPtr data_;
  BinaryWriter data_writer_;
  BufferPtr objects_;
  BinaryWriter objects_writer_;
  std::vector<std::shared_ptr<Object>> local_objects_;

  DISALLOW_COPY_AND_ASSIGN(ParcelTransactionDataWriter);
};
} // namespace binderd

#endif
