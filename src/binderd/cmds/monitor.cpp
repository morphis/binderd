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

#include "binderd/cmds/monitor.h"
#include "binderd/client.h"
#include "binderd/logger.h"

namespace {
class MonitorPrinter : public binderd::Client::LogMessageHandler {
 public:
  MonitorPrinter() {}
  ~MonitorPrinter() override {}

  void OnLogMessage(const binderd::MessagePtr &msg) override {
    std::cout << *msg << std::endl;
  }
};
}

binderd::cmds::Monitor::Monitor() :
  CommandWithFlagsAndAction{
    cli::Name{"monitor"}, cli::Usage{"monitor"},
    cli::Description{"Monitor all messages exchanged over the binderd server"}} {

  action([this](const binderd::cli::Command::Context&) {
    auto client = Client::Create();
    if (client->GetState() != Client::State::Connected) {
      std::cerr << "Failed to connect to server";
      return EXIT_FAILURE;
    }

    const auto status = client->BecomeMonitor();
    if (status != Status::OK) {
      std::cerr << "Failed to enable monitor mode" << std::endl;
      return EXIT_FAILURE;
    }

    auto printer = std::make_shared<MonitorPrinter>();
    client->SetLogMessageHandler(printer);
    client->StartThreadPool();
    client->JoinThreadPool();

    return EXIT_SUCCESS;
  });
}

