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

#include "binderd/client_impl.h"
#include "binderd/buffer.h"
#include "binderd/message_parser.h"
#include "binderd/remote_object.h"
#include "binderd/logger.h"

using namespace binderd;

using ::testing::_;
using ::testing::SaveArg;
using ::testing::Return;
using ::testing::AtLeast;
using ::testing::Invoke;

namespace {
class MockMessenger : public Messenger {
 public:
  ~MockMessenger() override { }

  MOCK_METHOD1(Connect, bool(const std::string&));
  MOCK_METHOD0(Close, void());
  MOCK_METHOD0(Flush, void());

  MOCK_METHOD1(ReadMessageAsync, void(const ReadHandler&));
  MOCK_METHOD1(WriteData, void(const BufferPtr&));
};

class MockObject : public Object {
 public:
  ~MockObject() override { }

  MOCK_CONST_METHOD0(GetType, Type());

  MOCK_METHOD2(LinkToDeath, Status(const ClientPtr &client,
                                   const std::shared_ptr<Object::DeathRecipient>&));
  MOCK_METHOD2(UnlinkToDeath, Status(const ClientPtr &client,
                                     const std::shared_ptr<Object::DeathRecipient>&));

  // As we can't mock the real Transact method as it's std::unique_ptr
  // parameter isn't copyable (which is required for mocking) we implement
  // a second variant which passes a raw pointer instead.
  MOCK_METHOD3(Transact, Status(const ClientPtr&,
                                TransactionData*,
                                std::unique_ptr<binderd::TransactionData> *reply));

  binderd::Status Transact(const binderd::ClientPtr &client,
                           std::unique_ptr<binderd::TransactionData> request,
                           std::unique_ptr<binderd::TransactionData> *reply) override {
    return Transact(client, request.get(), reply);
  }
};

class MockDeathRecipient : public RemoteObject::DeathRecipient {
 public:
  ~MockDeathRecipient() override {}

  MOCK_METHOD1(OnObjectDied, void(const Object*));
};

class ClientTest : public ::testing::Test {
 public:
  ClientTest() :
    io{std::make_shared<asio::io_service>()} {}

  void SetUp() override {
    messenger = std::make_shared<MockMessenger>();

    EXPECT_CALL(*messenger, Connect("/dev/null"))
        .Times(1)
        .WillOnce(::testing::Return(true));

    EXPECT_CALL(*messenger, Close())
        .Times(1);

    EXPECT_CALL(*messenger, ReadMessageAsync(::testing::_))
        .Times(AtLeast(1))
        .WillRepeatedly(SaveArg<0>(&read_handler));

    client = std::make_shared<ClientImpl>(io, messenger, "/dev/null");
    ASSERT_EQ(Client::State::Connected, client->GetState());
    ASSERT_NE(nullptr, read_handler);

    ::testing::Mock::AllowLeak(messenger.get());
  }

  void TearDown() override {
    client.reset();
    messenger.reset();
  }

  void SendMessage(const MessagePtr &msg) {
    read_handler(asio::error_code(0, asio::system_category()), msg->Pack());
  }

  std::shared_ptr<RemoteObject> CreateRemoteObject(uint64_t handle) {
    EXPECT_CALL(*messenger, WriteData(_))
        .Times(1)
        .WillOnce(Invoke([&](const BufferPtr &buffer) {
      MessageParser parser(buffer, buffer->GetSize());
      auto msg = parser.Next();
      ASSERT_EQ(Message::Type::Acquire, msg->GetType());

      auto reader = msg->GetReader();
      const auto read_handle = reader.ReadUint64();
      ASSERT_EQ(handle, read_handle);

      auto reply = Message::Create(Message::Type::TransactionReply);
      reply->SetCookie(msg->GetCookie());
      auto writer = reply->GetWriter();
      writer.WriteInt32(static_cast<int32_t>(Status::OK));
      SendMessage(reply);
    }));

    // This will call Client::AddReference internally to acquire a reference
    // on the server side
    return RemoteObject::Create(client, handle);
  }

  MessagePtr CreateDeathNotification(uint64_t object_id) {
    auto msg = Message::Create(Message::Type::DeadBinder);
    msg->SetCookie(client->NewCookie());
    auto writer = msg->GetWriter();
    writer.WriteUint64(object_id);
    return msg;
  }

  void LinkToDeath(const ObjectPtr &object,
                   std::shared_ptr<MockDeathRecipient> &death_recipient,
                   uint64_t expected_handle,
                   uint64_t &object_id) {
    EXPECT_CALL(*messenger, WriteData(_))
        .Times(1)
        .WillOnce(Invoke([&](const BufferPtr &buffer) {
      MessageParser parser(buffer, buffer->GetSize());
      auto msg = parser.Next();
      ASSERT_EQ(Message::Type::RequestDeathNotification, msg->GetType());

      auto reader = msg->GetReader();
      const auto handle = reader.ReadUint64();
      ASSERT_EQ(expected_handle, handle);
      object_id = reader.ReadUint64();
      ASSERT_LE(0, object_id);

      auto reply = Message::Create(Message::Type::Status);
      reply->SetCookie(msg->GetCookie());
      auto writer = reply->GetWriter();
      writer.WriteInt32(static_cast<int32_t>(Status::OK));
      SendMessage(reply);
    }));

    const auto status = object->LinkToDeath(client, death_recipient);
    ASSERT_EQ(Status::OK, status);
  }

  void ExpectStatusSent(const Status &status) {
    EXPECT_CALL(*messenger, WriteData(_))
        .Times(1)
        .WillOnce(Invoke([&](const BufferPtr &buffer) {
      MessageParser parser(buffer, buffer->GetSize());
      auto msg = parser.Next();
      ASSERT_EQ(Message::Type::Status, msg->GetType());
      auto reader = msg->GetReader();
      ASSERT_EQ(status, static_cast<Status>(reader.ReadInt32()));
    }));
  }

  std::shared_ptr<asio::io_service> io;
  std::shared_ptr<MockMessenger> messenger;
  std::shared_ptr<Client> client;
  Messenger::ReadHandler read_handler = nullptr;
};
}

TEST_F(ClientTest, MessageIsQueuedAndSent) {
  auto msg = Message::Create(Message::Type::Acquire);
  msg->SetCookie(0x1);

  BufferPtr buffer;

  EXPECT_CALL(*messenger, WriteData(::testing::_))
      .Times(1)
      .WillOnce(SaveArg<0>(&buffer));

  client->QueueMessage(msg);

  ASSERT_NE(nullptr, buffer);
  ASSERT_THAT(std::vector<uint8_t>(buffer->GetData(), buffer->GetData() + buffer->GetSize()),
              ::testing::ElementsAreArray({ 0x3, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                                            0x0, 0x0, 0x0, 0x0, 0x1, 0x0, 0x0, 0x0,
                                            0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 }));
}

TEST_F(ClientTest, DequeueIncomingMessage) {
  auto msg = Message::Create(Message::Type::Acquire);
  SendMessage(msg);

  auto received_msg = client->DequeueMessage();
  ASSERT_NE(nullptr, received_msg);
  ASSERT_EQ(Message::Type::Acquire, received_msg->GetType());
}

TEST_F(ClientTest, ContextObjReceivesIncomingTransaction) {
  auto object = std::make_shared<MockObject>();
  client->SetContextObject(object);

  // Object gets released when client is destroyed in TearDown()
  ::testing::Mock::AllowLeak(object.get());

  auto msg = Message::Create(Message::Type::Transaction);
  msg->SetCookie(0x12);

  EXPECT_CALL(*object, Transact(client, _, _))
      .Times(1)
      .WillOnce(Return(Status::OK));

  SendMessage(msg);

  ASSERT_EQ(0, io->run_one());

  ::testing::Mock::VerifyAndClearExpectations(object.get());
}

TEST_F(ClientTest, AddReference) {
  EXPECT_CALL(*messenger, WriteData(_))
      .Times(1)
      .WillOnce(Invoke([&](const BufferPtr &buffer) {
    MessageParser parser(buffer, buffer->GetSize());
    auto msg = parser.Next();
    ASSERT_EQ(Message::Type::Acquire, msg->GetType());

    auto reader = msg->GetReader();
    const auto handle = reader.ReadUint64();
    ASSERT_EQ(1337, handle);

    auto reply = Message::Create(Message::Type::TransactionReply);
    auto writer = reply->GetWriter();
    writer.WriteInt32(static_cast<int32_t>(Status::OK));
    SendMessage(reply);
  }));

  const auto status = client->AddReference(1337);
  ASSERT_EQ(Status::OK, status);
}

TEST_F(ClientTest, ReferenceCanBeReleased) {
  EXPECT_CALL(*messenger, WriteData(_))
      .Times(1)
      .WillOnce(Invoke([&](const BufferPtr &buffer) {
    MessageParser parser(buffer, buffer->GetSize());
    auto msg = parser.Next();
    ASSERT_EQ(Message::Type::Release, msg->GetType());

    auto reader = msg->GetReader();
    const auto handle = reader.ReadUint64();
    ASSERT_EQ(1337, handle);

    auto reply = Message::Create(Message::Type::TransactionReply);
    auto writer = reply->GetWriter();
    writer.WriteInt32(static_cast<int32_t>(Status::OK));
    SendMessage(reply);
  }));

  const auto status = client->ReleaseReference(1337);
  ASSERT_EQ(Status::OK, status);
}

TEST_F(ClientTest, ReferenceCanBeReleasedWithClosure) {
  EXPECT_CALL(*messenger, WriteData(_))
      .Times(1)
      .WillOnce(Invoke([&](const BufferPtr &buffer) {
    MessageParser parser(buffer, buffer->GetSize());
    auto msg = parser.Next();
    ASSERT_EQ(Message::Type::Release, msg->GetType());

    auto reader = msg->GetReader();
    const auto handle = reader.ReadUint64();
    ASSERT_EQ(1337, handle);

    auto reply = Message::Create(Message::Type::TransactionReply);
    auto writer = reply->GetWriter();
    writer.WriteInt32(static_cast<int32_t>(Status::OK));
    SendMessage(reply);
  }));

  auto closure = client->GetReleaseReferenceClosure(1337);
  const Status status = closure();
  ASSERT_EQ(Status::OK, status);
}

TEST_F(ClientTest, CanBecomeContextManager) {
  EXPECT_CALL(*messenger, WriteData(_))
      .Times(1)
      .WillOnce(Invoke([&](const BufferPtr &buffer) {
    MessageParser parser(buffer, buffer->GetSize());
    auto msg = parser.Next();
    ASSERT_EQ(Message::Type::SetContextMgr, msg->GetType());

    auto reply = Message::Create(Message::Type::TransactionReply);
    auto writer = reply->GetWriter();
    writer.WriteInt32(static_cast<int32_t>(Status::OK));
    SendMessage(reply);
  }));

  const Status status = client->BecomeContextManager();
  ASSERT_EQ(Status::OK, status);
}

TEST_F(ClientTest, CanReceiveDeathNotification) {
  const uint64_t expected_handle = 1337;

  auto object = CreateRemoteObject(expected_handle);

  auto death_recipient = std::make_shared<MockDeathRecipient>();
  uint64_t object_id = 0;
  LinkToDeath(object, death_recipient, expected_handle, object_id);

  // As we're now registered we should get a notification that
  // the object is dead when the serve says so.
  auto death_notification = CreateDeathNotification(object_id);
  SendMessage(death_notification);

  // Sending the death notification for the same object should
  // not lead to the death being reported twice
  SendMessage(death_notification);

  EXPECT_CALL(*death_recipient, OnObjectDied(object.get()))
      .Times(1);

  ExpectStatusSent(Status::OK);

  client->ProcessAndExecuteCommand();
}

TEST_F(ClientTest, AfterUnlinkedDeathNotificationRecipientIsntNotified) {
  const uint64_t expected_handle = 1337;

  auto object = CreateRemoteObject(expected_handle);

  auto death_recipient = std::make_shared<MockDeathRecipient>();
  uint64_t object_id = 0;
  LinkToDeath(object, death_recipient, expected_handle, object_id);

  EXPECT_CALL(*messenger, WriteData(_))
      .Times(1)
      .WillOnce(Invoke([&](const BufferPtr &buffer) {
    MessageParser parser(buffer, buffer->GetSize());
    auto msg = parser.Next();
    ASSERT_EQ(Message::Type::ClearDeathNotification, msg->GetType());

    auto reader = msg->GetReader();
    ASSERT_EQ(expected_handle, reader.ReadUint64());
    ASSERT_LE(object_id, reader.ReadUint64());

    auto reply = Message::Create(Message::Type::Status);
    reply->SetCookie(msg->GetCookie());
    auto writer = reply->GetWriter();
    writer.WriteInt32(static_cast<int32_t>(Status::OK));
    SendMessage(reply);
  }));

  const auto status = object->UnlinkToDeath(client, death_recipient);
  ASSERT_EQ(Status::OK, status);

  // When the death notification is sent and processed it should not
  // be forwarded to the object anymore.
  auto death_notification = CreateDeathNotification(object_id);
  SendMessage(death_notification);

  ExpectStatusSent(Status::DeadObject);

  client->ProcessAndExecuteCommand();

  // The remote object will be release once the test is over
  EXPECT_CALL(*messenger, WriteData(_))
      .Times(1)
      .WillOnce(Invoke([&](const BufferPtr &buffer) {
    MessageParser parser(buffer, buffer->GetSize());
    auto msg = parser.Next();
    ASSERT_EQ(Message::Type::Release, msg->GetType());

    auto reply = Message::Create(Message::Type::TransactionReply);
    reply->SetCookie(msg->GetCookie());
    auto writer = reply->GetWriter();
    writer.WriteInt32(static_cast<int32_t>(Status::OK));
    SendMessage(reply);
  }));
}

