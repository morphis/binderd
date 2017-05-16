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

#ifndef BINDERD_SERVER_H_
#define BINDERD_SERVER_H_

#include "binderd/constants.h"

#include <memory>

namespace binderd {
class Server : public std::enable_shared_from_this<Server> {
 public:
  static std::shared_ptr<Server> Create(const std::string &socket_path = kDefaultSocketPath);

  virtual ~Server() {}

  virtual void Run() = 0;
  virtual void RunAsync(std::size_t num_workers) = 0;
  virtual void Stop() = 0;

 private:
  friend class ServerSession;
};
} // namespace binderd

#endif
