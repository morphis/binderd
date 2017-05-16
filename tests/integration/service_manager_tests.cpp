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
#include "binderd/service_manager_proxy.h"
#include "binderd/service_manager_service.h"
#include "binderd/local_object.h"
#include "binderd/logger.h"
#include "binderd/parcel_transaction_data_writer.h"

#include <thread>
#include <atomic>
#include <mutex>

#include <boost/filesystem.hpp>

#include "tests/common/standalone_server.h"

namespace fs = boost::filesystem;

namespace {
class ServiceManagerTest : public ::testing::Test {
 public:
  ServiceManagerTest() {
    binderd::Log().Init(binderd::Logger::Severity::kDebug);
  }

  virtual void SetUp() {
    server = std::make_shared<binderd::testing::StandaloneServer>();
    server->RunAsync();
  }

  virtual void TearDown() {
    server.reset();
  }

 protected:
  std::shared_ptr<binderd::testing::StandaloneServer> server;
};

class TestService : public binderd::LocalObject::TransactionHandler {
 public:
  binderd::Status OnTransact(const binderd::ClientPtr &client,
                             std::unique_ptr<binderd::TransactionData> request,
                             std::unique_ptr<binderd::TransactionData> *reply) override {
    (void) client;
    (void) request;
    (void) reply;

    return binderd::Status::FdsNotAllowed;
  }
};
}

TEST_F(ServiceManagerTest, NoServicesAvailableWhenNoRegistered) {
  auto host = binderd::Client::Create(server->GetSocketPath());
  ASSERT_EQ(host->GetState(), binderd::Client::State::Connected);
  host->BecomeContextManager();
  host->SetContextObject(std::make_shared<binderd::LocalObject>(std::make_unique<binderd::ServiceManagerService>()));

  auto client = binderd::Client::Create(server->GetSocketPath());
  ASSERT_EQ(client->GetState(), binderd::Client::State::Connected);

  binderd::ServiceManagerProxy proxy;
  std::vector<std::string> services;
  ASSERT_EQ(binderd::Status::OK, proxy.ListServices(client, services));
  ASSERT_EQ(0, services.size());
}

TEST_F(ServiceManagerTest, CanAddAndListService) {
  auto host = binderd::Client::Create(server->GetSocketPath());
  ASSERT_EQ(host->GetState(), binderd::Client::State::Connected);
  host->BecomeContextManager();
  host->SetContextObject(std::make_shared<binderd::LocalObject>(std::make_unique<binderd::ServiceManagerService>()));

  auto client = binderd::Client::Create(server->GetSocketPath());
  ASSERT_EQ(client->GetState(), binderd::Client::State::Connected);

  binderd::ServiceManagerProxy proxy;
  proxy.AddService(client, "Foo", std::make_shared<binderd::LocalObject>(std::make_unique<TestService>()), 0);

  std::vector<std::string> services;
  ASSERT_EQ(binderd::Status::OK, proxy.ListServices(client, services));
  ASSERT_EQ(1, services.size());
  ASSERT_EQ("Foo", services[0]);
}

TEST_F(ServiceManagerTest, CanAddAndGetService) {
  auto sm_host = binderd::Client::Create(server->GetSocketPath());
  ASSERT_EQ(sm_host->GetState(), binderd::Client::State::Connected);
  sm_host->BecomeContextManager();
  sm_host->SetContextObject(std::make_shared<binderd::LocalObject>(std::make_unique<binderd::ServiceManagerService>()));

  auto client = binderd::Client::Create(server->GetSocketPath());
  ASSERT_EQ(client->GetState(), binderd::Client::State::Connected);

  binderd::ServiceManagerProxy sm;
  ASSERT_EQ(binderd::Status::OK, sm.AddService(client, "Foo", std::make_shared<binderd::LocalObject>(std::make_unique<TestService>()), 0));

  std::shared_ptr<binderd::Object> proxy;
  ASSERT_EQ(binderd::Status::OK, sm.GetService(client, "Foo", &proxy));
  ASSERT_NE(nullptr, proxy);
  ASSERT_EQ(binderd::Object::Type::Remote, proxy->GetType());
}

TEST_F(ServiceManagerTest, CanAddAndTalkToLocalService) {
  auto sm_host = binderd::Client::Create(server->GetSocketPath());
  ASSERT_EQ(sm_host->GetState(), binderd::Client::State::Connected);
  sm_host->BecomeContextManager();
  sm_host->SetContextObject(std::make_shared<binderd::LocalObject>(std::make_unique<binderd::ServiceManagerService>()));

  auto service_client = binderd::Client::Create(server->GetSocketPath());
  ASSERT_EQ(service_client->GetState(), binderd::Client::State::Connected);
  binderd::ServiceManagerProxy sm;
  sm.AddService(service_client, "Foo", std::make_shared<binderd::LocalObject>(std::make_unique<TestService>()), 0);

  auto client = binderd::Client::Create(server->GetSocketPath());
  ASSERT_EQ(client->GetState(), binderd::Client::State::Connected);

  std::shared_ptr<binderd::Object> proxy;
  ASSERT_EQ(binderd::Status::OK, sm.GetService(client, "Foo", &proxy));
  ASSERT_NE(nullptr, proxy);

  auto worker_thread = std::thread([&]() { service_client->ProcessAndExecuteCommand(); });

  binderd::ParcelTransactionDataWriter writer;
  writer.SetCode(2);

  std::unique_ptr<binderd::TransactionData> reply;
  ASSERT_EQ(binderd::Status::FdsNotAllowed, proxy->Transact(client, std::move(writer.Finalize()), &reply));

  worker_thread.join();
}
