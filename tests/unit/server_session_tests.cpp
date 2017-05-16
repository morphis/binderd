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

#include <gmock/gmock.h>

#include "binderd/server_session.h"
#include "binderd/registry.h"
#include "binderd/message_parser.h"

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

class ServerSessionTest : public ::testing::Test {
 public:
  ServerSessionTest() :
    io{std::make_shared<asio::io_service>()} {}

  void SetUp() override {
    messenger = std::make_shared<MockMessenger>();

    ::testing::Mock::AllowLeak(messenger.get());

    EXPECT_CALL(*messenger, ReadMessageAsync(::testing::_))
        .Times(AtLeast(1))
        .WillRepeatedly(SaveArg<0>(&read_handler));

    registry = Registry::Create();
    sessions = std::make_shared<ServerSessions>();

    session = ServerSession::Create(sessions, io, messenger, registry);
    session->Start();

  }

  void TearDown() override {
    session.reset();
    registry.reset();
    sessions.reset();
    messenger.reset();
  }

  void SendMessage(const MessagePtr &msg) {
    read_handler(asio::error_code(0, asio::system_category()), msg->Pack());
  }

  void RequestDeathNotification(const uint64_t &handle, const uint64_t &object_id) {
    EXPECT_CALL(*messenger, WriteData(_))
        .Times(1)
        .WillOnce(Invoke([&](const BufferPtr &buffer) {
      MessageParser parser(buffer, buffer->GetSize());
      auto msg = parser.Next();
      ASSERT_EQ(Message::Type::Status, msg->GetType());
      auto reader = msg->GetReader();
      const auto status = static_cast<Status>(reader.ReadInt32());
      ASSERT_EQ(Status::OK, status);
    }));

    auto msg = Message::Create(Message::Type::RequestDeathNotification);
    msg->SetCookie(2);
    auto writer = msg->GetWriter();
    writer.WriteUint64(handle);
    writer.WriteUint64(object_id);
    SendMessage(msg);
  }

  std::shared_ptr<asio::io_service> io;
  std::shared_ptr<MockMessenger> messenger;
  Messenger::ReadHandler read_handler = nullptr;
  std::shared_ptr<ServerSessions> sessions;
  std::shared_ptr<ServerSession> session;
  std::shared_ptr<Registry> registry;
};
}

TEST_F(ServerSessionTest, AcceptsContextManager) {
  EXPECT_CALL(*messenger, WriteData(_))
      .Times(1)
      .WillOnce(Invoke([&](const BufferPtr &buffer) {
    MessageParser parser(buffer, buffer->GetSize());
    auto msg = parser.Next();
    ASSERT_EQ(Message::Type::Status, msg->GetType());
    auto reader = msg->GetReader();
    const auto status = static_cast<Status>(reader.ReadInt32());
    ASSERT_EQ(Status::OK, status);
  }));

  auto msg = Message::Create(Message::Type::SetContextMgr);
  msg->SetCookie(1);
  SendMessage(msg);

  ASSERT_EQ(session, registry->GetContextManager());

  // Trying again to become context manager will always fail
  EXPECT_CALL(*messenger, WriteData(_))
      .Times(1)
      .WillOnce(Invoke([&](const BufferPtr &buffer) {
    MessageParser parser(buffer, buffer->GetSize());
    auto msg = parser.Next();
    ASSERT_EQ(Message::Type::Status, msg->GetType());
    auto reader = msg->GetReader();
    const auto status = static_cast<Status>(reader.ReadInt32());
    ASSERT_EQ(Status::AlreadyExists, status);
  }));

  msg = Message::Create(Message::Type::SetContextMgr);
  msg->SetCookie(2);
  SendMessage(msg);
}

TEST_F(ServerSessionTest, AcceptsDeathNotificationRegistration) {
  const auto handle = 1337;
  const auto object_id = 1;
  RequestDeathNotification(handle, object_id);
}

TEST_F(ServerSessionTest, DoesNotReportDeathWithNoRegistration) {
  EXPECT_CALL(*messenger, WriteData(_))
      .Times(0);
  session->SendDeathNotification(1);
}

TEST_F(ServerSessionTest, ClientReceivesDeathNotificationWhenRegistered) {
  const auto handle = 1337;
  const auto object_id = 1;
  RequestDeathNotification(handle, object_id);

  EXPECT_CALL(*messenger, WriteData(_))
      .Times(1)
      .WillOnce(Invoke([&](const BufferPtr &buffer) {
    MessageParser parser(buffer, buffer->GetSize());
    auto msg = parser.Next();
    ASSERT_EQ(Message::Type::DeadBinder, msg->GetType());
    auto reader = msg->GetReader();
    ASSERT_EQ(handle, reader.ReadUint64());
    ASSERT_EQ(object_id, reader.ReadUint64());
  }));

  session->SendDeathNotification(1337);
}
