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

#ifndef BINDERD_WRITABLE_TRANSACTION_DATA_H_
#define BINDERD_WRITABLE_TRANSACTION_DATA_H_

#include "binderd/transaction_data.h"
#include "binderd/macros.h"

namespace binderd {
class WritableTransactionData : public TransactionData {
 public:
  explicit WritableTransactionData();
  ~WritableTransactionData() override;

  uintptr_t GetBinder() const override;
  uintptr_t GetCookie() const override;
  uint32_t GetCode() const override;
  bool IsOneWay() const override;
  bool HasStatus() const override;
  Status GetStatus() const override;
  const uint8_t* GetData() const override;
  uint8_t* GetMutableData() const override;
  size_t GetDataSize() const override;
  const uint8_t* GetObjectOffsets() const override;
  size_t GetNumObjectOffsets() const override;

  void SetBinder(uintptr_t binder) { binder_ = binder; }
  void SetCookie(uintptr_t cookie) { cookie_ = cookie; }
  void SetCode(uint32_t code) { code_ = code; }
  void SetIsOneWay(bool is_one_way) { is_one_way_ = is_one_way; }
  void SetData(const BufferPtr &buffer);
  void SetObjectOffsets(const BufferPtr &buffer);

 private:
  uintptr_t binder_ = 0;
  uintptr_t cookie_ = 0;
  uint32_t code_ = 0;
  bool is_one_way_ = false;
  BufferPtr data_;
  BufferPtr objects_;

  DISALLOW_COPY_AND_ASSIGN(WritableTransactionData);
};
} // namespace binderd

#endif
