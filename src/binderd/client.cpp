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

#include "binderd/client.h"
#include "binderd/client_impl.h"

namespace binderd {
static std::string socket_path_override;

void Client::SetSocketPathOverride(const std::string &socket_path) {
  socket_path_override = socket_path;
}

std::shared_ptr<Client> Client::Create(const std::string &socket_path, bool connect) {
  auto path = socket_path;
  if (!socket_path_override.empty())
    path = socket_path_override;

  auto io = std::make_shared<asio::io_service>();
  auto messenger = std::make_shared<SocketMessenger>(io);
  return std::shared_ptr<Client>(new ClientImpl(io, messenger, path, connect));
}

Client* Client::CreateNotTracked(const std::string &socket_path) {
  auto io = std::make_shared<asio::io_service>();
  auto messenger = std::make_shared<SocketMessenger>(io);
  return new ClientImpl(io, messenger, socket_path, connect);
}
} // namespace binderd
