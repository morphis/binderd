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

#ifndef BINDERD_TRANSACTION_DATA_H_
#define BINDERD_TRANSACTION_DATA_H_

#include "binderd/status.h"
#include "binderd/buffer.h"

namespace binderd {
class TransactionData {
 public:
  enum class Flags : std::uint32_t {
    OneWay = 0x1,
    RootObject = 0x4,
    HasStatus = 0x8,
    AcceptFds = 0x10,
  };

  virtual ~TransactionData() {}

  virtual uintptr_t GetBinder() const = 0;
  virtual uintptr_t GetCookie() const = 0;
  virtual uint32_t GetCode() const = 0;
  virtual bool IsOneWay() const = 0;
  virtual bool HasStatus() const = 0;
  virtual Status GetStatus() const = 0;
  virtual const uint8_t* GetData() const = 0;
  virtual uint8_t* GetMutableData() const = 0;
  virtual size_t GetDataSize() const = 0;
  virtual const uint8_t* GetObjectOffsets() const = 0;
  virtual size_t GetNumObjectOffsets() const = 0;

  BufferPtr Pack() const;
};
std::ostream& operator<<(std::ostream &out, const TransactionData &data);
} // namespace binderd

#endif
