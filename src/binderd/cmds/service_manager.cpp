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

#include "binderd/cmds/service_manager.h"
#include "binderd/client.h"
#include "binderd/logger.h"
#include "binderd/service_manager_service.h"

binderd::cmds::ServiceManager::ServiceManager()
    : CommandWithFlagsAndAction{
          cli::Name{"service-manager"}, cli::Usage{"service-manger"},
          cli::Description{"Run the service-manager service"}} {
  action([](const cli::Command::Context&) {
    auto client = Client::Create();

    if (client->BecomeContextManager() != Status::OK) {
      ERROR("Failed to become context manager");
      return EXIT_FAILURE;
    }

    client->SetContextObject(std::make_shared<LocalObject>(std::make_unique<ServiceManagerService>()));
    client->StartThreadPool();
    client->JoinThreadPool();

    return EXIT_SUCCESS;
  });
}
