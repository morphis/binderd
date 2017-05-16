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

#include "binderd/cmds/service.h"
#include "binderd/client.h"
#include "binderd/service_manager_proxy.h"
#include "binderd/common/utils.h"

binderd::cmds::Service::Service() :
  CommandWithFlagsAndAction{
    cli::Name{"service"}, cli::Usage{"service"},
    cli::Description{"Simple command to interact with binder services"}} {

  flag(cli::make_flag(cli::Name{"list"},
                      cli::Description{"List available binder services"},
                      list_));

  action([this](const binderd::cli::Command::Context&) {
    if (!list_) {
      std::cerr << "No valid action specified" << std::endl;
      return EXIT_FAILURE;
    }

    auto client = Client::Create();
    if (client->GetState() != Client::State::Connected) {
      std::cerr << "Failed to connect to server";
      return EXIT_FAILURE;
    }

    auto sm = std::make_shared<ServiceManagerProxy>();

    std::vector<std::string> services;
    auto status = sm->ListServices(client, services);
    if (status != Status::OK) {
      std::cerr << utils::string_format("Failed to retrieve available services: %s", status) << std::endl;
      return EXIT_FAILURE;
    }
    std::cout << "Found " << services.size() << " services:" << std::endl;
    auto n = 0;
    for (const auto &service : services) {
      std::cout << n++ << "\t" << service << std::endl;
    }
    return EXIT_SUCCESS;
  });
}

