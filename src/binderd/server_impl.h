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

#ifndef BINDERD_SERVER_IMPL_H_
#define BINDERD_SERVER_IMPL_H_

#include "binderd/constants.h"
#include "binderd/server.h"
#include "binderd/registry.h"

#include <memory>
#include <thread>

#include <asio.hpp>

namespace binderd {
class ServerImpl : public Server {
 public:
  explicit ServerImpl(const std::string &socket_path = kDefaultSocketPath);
  ~ServerImpl() override;

  void Run() override;
  void RunAsync(std::size_t num_workers) override;
  void Stop() override;

 private:
  void StartAccept();
  void CloseAllSessions();

  std::shared_ptr<asio::io_service> io_;
  asio::local::stream_protocol::acceptor acceptor_;
  asio::local::stream_protocol::socket socket_;
  std::shared_ptr<ServerSessions> sessions_;
  std::vector<std::thread> workers_;
  RegistryPtr registry_;
  std::atomic_bool exit_{false};
};
} // namespace binderd

#endif
