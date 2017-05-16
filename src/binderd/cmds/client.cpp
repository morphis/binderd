/*
 * Copyright (C) 2017 Simon Fels <morphis@gravedo.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "binderd/cmds/client.h"
#include "binderd/client.h"
#include "binderd/local_object.h"
#include "binderd/service_manager_proxy.h"
#include "binderd/parcel_transaction_data_writer.h"
#include "binderd/logger.h"

#include <thread>

namespace {
class TestService : public binderd::LocalObject::TransactionHandler {
 public:
  binderd::Status OnTransact(const binderd::ClientPtr &client,
                             std::unique_ptr<binderd::TransactionData> request,
                             std::unique_ptr<binderd::TransactionData> *reply) override {
    return binderd::Status::FdsNotAllowed;
  }
};
}

binderd::cmds::Client::Client()
    : CommandWithFlagsAndAction{
          cli::Name{"client"}, cli::Usage{"client"},
          cli::Description{"Run a binder client"}} {

  flag(cli::make_flag(cli::Name{"service"},
                      cli::Description{"Start the service part"},
                      service_));

  action([this](const cli::Command::Context&) {
    auto client = binderd::Client::Create();
    binderd::ServiceManagerProxy sm;

    if (service_) {
      sm.AddService(client, "Foo", std::make_shared<binderd::LocalObject>(std::make_unique<TestService>()), 0);
      client->StartThreadPool();
      client->JoinThreadPool();
      return EXIT_SUCCESS;
    }

    std::shared_ptr<Object> object;
    if (sm.GetService(client, "Foo", &object) != Status::OK) {
      std::cerr << "Failed to access object 'Foo'" << std::endl;
      return EXIT_FAILURE;
    }

    ParcelTransactionDataWriter writer;
    writer.SetCode(3);

    std::unique_ptr<TransactionData> reply;
    auto status = object->Transact(client, std::move(writer.Finalize()), &reply);
    std::cout << "Receieved reply from service: " << status << std::endl;

    return EXIT_SUCCESS;
  });
}
