/*
 * Copyright (C) 2016 Simon Fels <morphis@gravedo.de>
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

#include "binderd/daemon.h"
#include "binderd/cmds/server.h"
#include "binderd/cmds/client.h"
#include "binderd/cmds/service_manager.h"
#include "binderd/cmds/service.h"
#include "binderd/cmds/monitor.h"
#include "binderd/logger.h"

#include <signal.h>
#include <sys/prctl.h>

namespace binderd {
Daemon::Daemon()
    : cmd{cli::Name{"binder"}, cli::Usage{"binder"},
          cli::Description{"Binder service"}} {

  cmd.command(std::make_shared<cmds::Server>())
     .command(std::make_shared<cmds::Client>())
     .command(std::make_shared<cmds::ServiceManager>())
     .command(std::make_shared<cmds::Service>())
     .command(std::make_shared<cmds::Monitor>());

  Log().Init(binderd::Logger::Severity::kWarning);

  const auto log_level = utils::get_env_value("BINDER_LOG_LEVEL", "");
  if (!log_level.empty() && !Log().SetSeverityFromString(log_level))
    WARNING("Failed to set logging severity to '%s'", log_level);
}

int Daemon::Run(const std::vector<std::string> &arguments) try {
  return cmd.run({std::cin, std::cout, arguments});
} catch (std::exception &err) {
  ERROR("%s", err.what());
  return EXIT_FAILURE;
}
}  // namespace binderd
