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

#include "binderd/transaction_status.h"

namespace binderd {
TransactionStatus::TransactionStatus(Status status) : status_{status} {}

TransactionStatus::~TransactionStatus() {}

uintptr_t TransactionStatus::GetBinder() const { return 0; }

uintptr_t TransactionStatus::GetCookie() const { return 0; }

uint32_t TransactionStatus::GetCode() const { return 0; }

bool TransactionStatus::IsOneWay() const { return false; }

bool TransactionStatus::HasStatus() const { return true; }

Status TransactionStatus::GetStatus() const { return status_; }

const uint8_t* TransactionStatus::GetData() const { return reinterpret_cast<const uint8_t*>(&status_); }

uint8_t* TransactionStatus::GetMutableData() const { return nullptr; }

size_t TransactionStatus::GetDataSize() const { return sizeof(status_); }

const uint8_t* TransactionStatus::GetObjectOffsets() const { return nullptr; }

size_t TransactionStatus::GetNumObjectOffsets() const { return 0; }
} // namespace binderd
