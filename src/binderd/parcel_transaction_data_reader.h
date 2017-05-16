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

#ifndef BINDERD_PARCEL_TRANSACTION_DATA_READER_H_
#define BINDERD_PARCEL_TRANSACTION_DATA_READER_H_

#include "binderd/macros.h"
#include "binderd/transaction_data.h"
#include "binderd/object.h"

#include <memory>

namespace binderd {
class ParcelTransactionDataReader {
 public:
  explicit ParcelTransactionDataReader(std::unique_ptr<TransactionData> &data);
  ~ParcelTransactionDataReader();

  bool EnforceInterface(const std::string &name);
  int32_t ReadInt32();
  std::u16string ReadString16();
  std::shared_ptr<Object> ReadObject(const std::shared_ptr<Client> &client);

 private:
  struct Implementation;
  std::unique_ptr<Implementation> impl;
  DISALLOW_COPY_AND_ASSIGN(ParcelTransactionDataReader);
};
} // namespace binderd

#endif
