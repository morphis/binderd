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

#ifndef BINDER_TESTING_COMMON_STANDALONE_SERVER_H_
#define BINDER_TESTING_COMMON_STANDALONE_SERVER_H_

#include "binderd/server.h"

#include <boost/filesystem.hpp>

namespace binderd {
namespace testing {
class StandaloneServer {
 public:
  explicit StandaloneServer();
  ~StandaloneServer();

  void RunAsync();

  std::string GetSocketPath() const { return socket_path_; }

 private:
  boost::filesystem::path test_dir_;
  std::string socket_path_;
  std::shared_ptr<binderd::Server> server_;
};
} // namespace testing
} // namespace binderd

#endif
