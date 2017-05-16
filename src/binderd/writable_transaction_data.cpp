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

#include "binderd/writable_transaction_data.h"
#include "binderd/binder_api.h"

namespace binderd {
WritableTransactionData::WritableTransactionData() {}

WritableTransactionData::~WritableTransactionData() {}

uintptr_t WritableTransactionData::GetBinder() const {
  return binder_;
}

uintptr_t WritableTransactionData::GetCookie() const {
  return cookie_;
}

uint32_t WritableTransactionData::GetCode() const {
  return code_;
}

bool WritableTransactionData::IsOneWay() const {
  return is_one_way_;
}

bool WritableTransactionData::HasStatus() const {
  return false;
}

Status WritableTransactionData::GetStatus() const {
  return static_cast<Status>(*reinterpret_cast<const int32_t*>(GetData()));
}

const uint8_t* WritableTransactionData::GetData() const {
  return GetMutableData();
}

uint8_t* WritableTransactionData::GetMutableData() const {
  if (!data_)
    return nullptr;
  return data_->GetData();
}

size_t WritableTransactionData::GetDataSize() const {
  if (!data_)
    return 0;
  return data_->GetSize();
}

const uint8_t* WritableTransactionData::GetObjectOffsets() const {
  if (!objects_)
    return nullptr;
  return objects_->GetData();
}

size_t WritableTransactionData::GetNumObjectOffsets() const {
  if (!objects_)
    return 0;
  return objects_->GetSize() / sizeof(binder_size_t);
}

void WritableTransactionData::SetData(const BufferPtr &buffer) {
  data_ = buffer;
}

void WritableTransactionData::SetObjectOffsets(const BufferPtr &buffer) {
  objects_ = buffer;
}
} // namespace binderd
