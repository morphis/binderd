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

#include "binderd/cmds/server.h"
#include "binderd/server.h"

binderd::cmds::Server::Server()
    : CommandWithFlagsAndAction{
          cli::Name{"server"}, cli::Usage{"server"},
          cli::Description{"Run the binder server component"}} {
  action([](const cli::Command::Context&) {

    auto server = binderd::Server::Create();
    server->Run();
    return EXIT_SUCCESS;
  });
}
