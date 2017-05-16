/*
 * Copyright (C) 2017 Simon Fels <morphis@gravedo.de>
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

#include <gtest/gtest.h>

#include "binderd/parcel_transaction_data_writer.h"
#include "binderd/client.h"
#include "binderd/local_object.h"
#include "binderd/binder_api.h"
#include "binderd/common/utils.h"
#include "binderd/logger.h"

TEST(ParcelTransactionDataWriter, CanWriteInt32) {
  binderd::ParcelTransactionDataWriter writer;
  writer.WriteInt32(0x11223344);
  auto data = writer.Finalize();
  ASSERT_NE(nullptr, data);
  ASSERT_EQ(4, data->GetDataSize());
  ASSERT_EQ(0x11223344, *reinterpret_cast<const int32_t*>(data->GetData()));
}

TEST(ParcelTransactionDataWriter, CanWriteObject) {
  binderd::ParcelTransactionDataWriter writer;
  auto client = binderd::Client::Create("", false);
  auto obj = std::make_shared<binderd::LocalObject>(nullptr);

  // Write an integer first to ensure we have to operate with an offset
  writer.WriteInt32(10);

  writer.WriteObject(client, obj);
  ASSERT_EQ(1, client->GetObjectPool()->GetReferencesCount());
  auto data = writer.Finalize();

  ASSERT_EQ(sizeof(int32_t) + sizeof(flat_binder_object), data->GetDataSize());
  ASSERT_EQ(1, data->GetNumObjectOffsets());
  const auto offset = *reinterpret_cast<const uint64_t*>(data->GetObjectOffsets());
  ASSERT_EQ(sizeof(int32_t), offset);

  auto flat = reinterpret_cast<const flat_binder_object*>(data->GetData() + offset);
  ASSERT_EQ(BINDER_TYPE_BINDER, flat->type);
  ASSERT_EQ(reinterpret_cast<uintptr_t>(obj.get()), flat->cookie);
}
