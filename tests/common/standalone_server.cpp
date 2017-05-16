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

#include "tests/common/standalone_server.h"

namespace fs = boost::filesystem;

namespace binderd {
namespace testing {
StandaloneServer::StandaloneServer() {
  test_dir_ = fs::temp_directory_path() / fs::unique_path();
  fs::create_directories(test_dir_);
  socket_path_ = (test_dir_ / fs::path("binder.sock")).string();
  server_ = binderd::Server::Create(socket_path_);
}

StandaloneServer::~StandaloneServer() {
  fs::remove_all(test_dir_);
}

void StandaloneServer::RunAsync() {
  server_->RunAsync(2);
}
} // namespace testing
} // namespace binderd
