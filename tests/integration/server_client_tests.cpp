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

#include "binderd/client.h"
#include "binderd/server.h"
#include "binderd/status.h"
#include "binderd/writable_transaction_data.h"
#include "binderd/transaction_data_from_message.h"
#include "binderd/logger.h"

#include "binder/Parcel.h"
#include "binder/Status.h"
#include "binder/IServiceManager.h"

#include <thread>

#include <boost/filesystem.hpp>

#include "tests/common/standalone_server.h"

namespace fs = boost::filesystem;

namespace {
class ServerClientTest : public ::testing::Test {
 public:
  ServerClientTest() {
    binderd::Log().Init(binderd::Logger::Severity::kDebug);
  }

  virtual void SetUp() {
    server = std::make_shared<binderd::testing::StandaloneServer>();
    server->RunAsync();
  }

  virtual void TearDown() {
    server.reset();
  }

  void ExpectStatusReply(const binderd::ClientPtr &client, const binderd::Status &status) {
    auto reply = client->DequeueMessage();
    ASSERT_EQ(reply->GetType(), binderd::Message::Type::Status);
    auto reader = reply->GetReader();
    ASSERT_EQ(static_cast<binderd::Status>(reader.ReadInt32()), status);
  }

  void BecomeContextManager(const binderd::ClientPtr &client, const binderd::Status &expected_status = binderd::Status::OK) {
    auto msg = binderd::Message::Create(binderd::Message::Type::SetContextMgr);
    ASSERT_NE(nullptr, msg);
    auto reply = client->Transact(msg);

    ASSERT_NE(nullptr, reply);
    ASSERT_EQ(binderd::Message::Type::Status, reply->GetType());
    ASSERT_EQ(msg->GetCookie(), reply->GetCookie());

    auto reader = reply->GetReader();
    auto status = static_cast<binderd::Status>(reader.ReadInt32());
    ASSERT_EQ(expected_status, status);
  }

 protected:
  std::shared_ptr<binderd::testing::StandaloneServer> server;
};
}

TEST_F(ServerClientTest, ClientCanConnectAndDisconect) {
  for (auto n = 0; n < 4; n++) {
    auto client = binderd::Client::Create(server->GetSocketPath());
    ASSERT_EQ(client->GetState(), binderd::Client::State::Connected);
    std::this_thread::sleep_for(std::chrono::milliseconds{10});
  }
}

TEST_F(ServerClientTest, ReturnsErrorForInvalidTransactions) {
  auto client = binderd::Client::Create(server->GetSocketPath());
  ASSERT_EQ(client->GetState(), binderd::Client::State::Connected);

  auto msg = binderd::Message::Create(binderd::Message::Type::Transaction);
  client->QueueMessage(msg);

  ExpectStatusReply(client, binderd::Status::BadValue);
}

TEST_F(ServerClientTest, ReportsNameNotFoundForInvalidHandle) {
  auto client = binderd::Client::Create(server->GetSocketPath());
  ASSERT_EQ(client->GetState(), binderd::Client::State::Connected);

  auto msg = binderd::Message::Create(binderd::Message::Type::Transaction);
  msg->SetDestination(1);

  binderd::WritableTransactionData tr;
  auto writer = msg->GetWriter();
  writer.WriteData(tr.Pack());

  client->QueueMessage(msg);

  ExpectStatusReply(client, binderd::Status::NameNotFound);
}

TEST_F(ServerClientTest, CanBecomeContextMgr) {
  // Try multiple times to ensure that another client can become
  // the context manager when the previous one died.
  for (auto n = 0; n < 5; n++) {
    auto client = binderd::Client::Create(server->GetSocketPath());
    ASSERT_EQ(binderd::Client::State::Connected, client->GetState());
    BecomeContextManager(client);
  }
}

TEST_F(ServerClientTest, SetContextMgrFailsWhenOneExistsAlready) {
  auto client1 = binderd::Client::Create(server->GetSocketPath());
  ASSERT_EQ(binderd::Client::State::Connected, client1->GetState());

  BecomeContextManager(client1);

  auto client2 = binderd::Client::Create(server->GetSocketPath());
  ASSERT_EQ(binderd::Client::State::Connected, client2->GetState());

  BecomeContextManager(client2, binderd::Status::AlreadyExists);
}

TEST_F(ServerClientTest, CanCallContextManagerRegardlessOfCookieOrBinder) {
  auto context_mgr = binderd::Client::Create(server->GetSocketPath());
  ASSERT_EQ(context_mgr->GetState(), binderd::Client::State::Connected);

  BecomeContextManager(context_mgr);

  auto client = binderd::Client::Create(server->GetSocketPath());
  ASSERT_EQ(client->GetState(), binderd::Client::State::Connected);

  {
    const auto transaction_cookie = client->NewCookie();

    auto msg = binderd::Message::Create(binderd::Message::Type::Transaction);
    msg->SetCookie(transaction_cookie);
    msg->SetDestination(binderd::kContextManagerHandle);

    binderd::WritableTransactionData tr;
    tr.SetCookie(1);
    tr.SetBinder(2);
    tr.SetCode(3);

    msg->GetWriter().WriteData(tr.Pack());

    client->QueueMessage(msg);

    auto status_msg = client->DequeueMessage();
    ASSERT_EQ(binderd::Message::Type::Status, status_msg->GetType());
    ASSERT_EQ(transaction_cookie, status_msg->GetCookie());
    auto reader = status_msg->GetReader();
    ASSERT_EQ(binderd::Status::OK, static_cast<binderd::Status>(reader.ReadInt32()));

    auto rmsg = context_mgr->DequeueMessage();
    ASSERT_EQ(binderd::Message::Type::Transaction, rmsg->GetType());
    binderd::TransactionDataFromMessage tmsg(rmsg);
    ASSERT_EQ(0, tmsg.GetBinder());
    ASSERT_EQ(0, tmsg.GetCookie());
    ASSERT_EQ(3, tmsg.GetCode());
  }
}
