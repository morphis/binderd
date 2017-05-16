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

#ifndef ANDROID_TRANSACTION_DATA_FROM_PARCEL_H_
#define ANDROID_TRANSACTION_DATA_FROM_PARCEL_H_

#include "binderd/transaction_data.h"
#include "binderd/buffer.h"
#include "binderd/client.h"

#include "binder/Parcel.h"

namespace android {
void InitParcelFromTransactionData(android::Parcel &p, const std::unique_ptr<binderd::TransactionData> &data);

class TransactionDataFromParcel : public binderd::TransactionData {
 public:
  explicit TransactionDataFromParcel(binderd::Client *client,
                                     const uint32_t &code,
                                     const Parcel *parcel,
                                     bool one_way = false);
  ~TransactionDataFromParcel() override;

  uintptr_t GetBinder() const override;
  uintptr_t GetCookie() const override;
  uint32_t GetCode() const override;
  bool IsOneWay() const override;
  bool HasStatus() const override;
  binderd::Status GetStatus() const override;
  const uint8_t* GetData() const override;
  uint8_t* GetMutableData() const override;
  size_t GetDataSize() const override;
  const uint8_t* GetObjectOffsets() const override;
  size_t GetNumObjectOffsets() const override;

 private:
  uint32_t code_;
  binderd::BufferPtr data_;
  binderd::BufferPtr objects_;
  bool one_way_ = false;
};
} // namespace android

#endif
