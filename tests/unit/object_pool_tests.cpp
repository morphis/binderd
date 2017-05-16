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
#include <gmock/gmock.h>

#include "binderd/object_pool.h"

using namespace ::testing;

namespace {
class TestObject : public binderd::Object {
 public:
  TestObject() {}

  MOCK_CONST_METHOD0(GetType, Type());

  MOCK_METHOD2(LinkToDeath, binderd::Status(const binderd::ClientPtr &client,
                                            const std::shared_ptr<binderd::Object::DeathRecipient>&));
  MOCK_METHOD2(UnlinkToDeath, binderd::Status(const binderd::ClientPtr &client,
                                              const std::shared_ptr<binderd::Object::DeathRecipient>&));

  binderd::Status Transact(const binderd::ClientPtr &client,
                           std::unique_ptr<binderd::TransactionData> request,
                           std::unique_ptr<binderd::TransactionData> *reply) override {
    (void) client;
    (void) request;
    (void) reply;
    return binderd::Status::OK;
  }
};
}

TEST(ObjectPool, CanConstructObject) {
  binderd::ObjectPool pool;
  auto obj = pool.New<TestObject>();
  ASSERT_NE(obj, nullptr);
  EXPECT_CALL(*obj, GetType())
      .Times(2)
      .WillOnce(Return(binderd::Object::Type::Local));
  ASSERT_EQ(obj->GetType(), binderd::Object::Type::Local);
  ASSERT_EQ(pool.GetReferencesCount(), 1);
}

TEST(ObjectPool, CanAdoptObject) {
  std::shared_ptr<binderd::Object> obj(new TestObject);
  binderd::ObjectPool pool;
  pool.AdoptObject(obj);
  ASSERT_EQ(pool.GetReferencesCount(), 1);
}
