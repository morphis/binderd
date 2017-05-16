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

#ifndef BINDERD_TRANSACTION_STATUS_H_
#define BINDERD_TRANSACTION_STATUS_H_

#include "binderd/transaction_data.h"
#include "binderd/macros.h"

namespace binderd {
class TransactionStatus : public TransactionData {
 public:
  explicit TransactionStatus(Status status);
  ~TransactionStatus() override;

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

 private:
  Status status_;
  DISALLOW_COPY_AND_ASSIGN(TransactionStatus);
};
} // namespace binderd

#endif
