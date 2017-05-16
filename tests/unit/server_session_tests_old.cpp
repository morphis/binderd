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

#include "binderd/server_session.h"
#include "binderd/registry.h"
#include "binderd/message_parser.h"

using namespace binderd;

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

/*
class ServerSessionTest : public ::testing::Test {
 public:
  ServerSessionTest() :
    io{std::make_shared<asio::io_service>()} {}

  void SetUp() override {

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

  std::shared_ptr<asio::io_service> io;
  std::shared_ptr<MockMessenger> messenger;
  Messenger::ReadHandler read_handler = nullptr;
  std::shared_ptr<ServerSessions> sessions;
  std::shared_ptr<ServerSession> session;
  std::shared_ptr<Registry> registry;
};*/
}

/*
TEST_F(ServerSessionTest, AcceptsContextManager) {
  messenger = std::make_shared<MockMessenger>();

  ::testing::Mock::AllowLeak(messenger.get());

  EXPECT_CALL(*messenger, Connect("/dev/null"))
      .Times(1)
      .WillOnce(::testing::Return(true));

  EXPECT_CALL(*messenger, Close())
      .Times(1);

  EXPECT_CALL(*messenger, ReadMessageAsync(::testing::_))
      .Times(AtLeast(1))
      .WillRepeatedly(SaveArg<0>(&read_handler));

  registry = Registry::Create();
  sessions = std::make_shared<ServerSessions>();

  session = ServerSession::Create(sessions, io, messenger, registry);
  sessions->Add(session);
  session->Start();

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
*/

TEST(SS, Test) {
  ::testing::FLAGS_gmock_verbose = "info";

  auto messenger = std::make_shared<MockMessenger>();

  EXPECT_CALL(*messenger, Connect("/dev/null"))
      .Times(2)
      .WillOnce(::testing::Return(true));

  EXPECT_CALL(*messenger, Close())
      .Times(2);

  Messenger::ReadHandler read_handler = nullptr;

  EXPECT_CALL(*messenger, ReadMessageAsync(::testing::_))
      .Times(::testing::AtLeast(1))
      .WillRepeatedly(::testing::SaveArg<0>(&read_handler));

  auto registry = Registry::Create();
  auto sessions = std::make_shared<ServerSessions>();

  auto io = std::make_shared<asio::io_service>();
  auto session = ServerSession::Create(sessions, io, messenger, registry);
  sessions->Add(session);
  session->Start();

  const auto expected_handle = 1337;
  const auto expected_id = 1;

  EXPECT_CALL(*messenger, WriteData(::testing::_))
      .Times(2)
      .WillOnce(::testing::Invoke([&](const BufferPtr &buffer) {
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
  writer.WriteUint64(expected_handle);
  writer.WriteUint64(expected_id);
  //SendMessage(msg);
}
