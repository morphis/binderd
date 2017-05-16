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

#include <binder/Parcel.h>
#include <binder/IServiceManager.h>
#include <utils/String8.h>

#include "external/android/binder/TransactionDataFromParcel.h"

#include "binderd/logger.h"
#include "binderd/parcel_transaction_data_reader.h"

using namespace android;
using namespace binderd;

namespace {
class TestService : public BBinder {
 public:
  TestService() {}
  ~TestService() {}

  virtual status_t onTransact(uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags = 0) {
    (void) code;
    (void) flags;
    (void) data;
    (void) reply;
    return NO_ERROR;
  }
};
}

TEST(Parcel, PassthroughMessageIsCorrect) {
  Parcel data;
  data.writeInterfaceToken(String16("android.os.IServiceManager"));
  data.writeString16(String16("test.Service"));
  sp<TestService> service = new TestService;
  data.writeStrongBinder(service);
  data.writeInt32(1);

  std::unique_ptr<TransactionData> td(new TransactionDataFromParcel(nullptr, 0, &data));
  ASSERT_NE(nullptr, td->GetData());

  ASSERT_LT(0, td->GetDataSize());
  ASSERT_NE(nullptr, td->GetObjectOffsets());
  ASSERT_LT(0, td->GetNumObjectOffsets());

  Parcel out;
  InitParcelFromTransactionData(out, td);
  ASSERT_TRUE(out.enforceInterface(String16("android.os.IServiceManager")));
  String8 name(out.readString16());
  ASSERT_EQ("test.Service", std::string(name.string()));
  ASSERT_NE(nullptr, out.readStrongBinder().get());
}

TEST(Parcel, CanBeReadByMessage) {
  Parcel data;
  data.writeInterfaceToken(String16("android.os.IServiceManager"));
  data.writeString16(String16("test.Service"));
  sp<TestService> service = new TestService;
  data.writeStrongBinder(service);
  data.writeInt32(1);

  std::unique_ptr<TransactionData> td(new TransactionDataFromParcel(nullptr, 0, &data));
  ASSERT_NE(nullptr, td->GetData());

  ParcelTransactionDataReader reader(td);
  ASSERT_TRUE(reader.EnforceInterface("android.os.IServiceManager"));
  const auto name = utils::to_string8(reader.ReadString16());
  ASSERT_EQ("test.Service", name);
  auto obj = reader.ReadObject(nullptr);
  ASSERT_NE(nullptr, obj);
  const auto allow_isolated = reader.ReadInt32();
  ASSERT_EQ(1, allow_isolated);
}
