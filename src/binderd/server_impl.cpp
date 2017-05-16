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

#include "binderd/server_impl.h"
#include "binderd/server_session.h"
#include "binderd/logger.h"
#include "binderd/registry.h"

#include <fstream>
#include <thread>
#include <unordered_map>



#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/epoll.h>
#include <fcntl.h>

namespace ba = asio;

namespace {
bool socket_file_exists(std::string const& filename) {
  struct stat statbuf;
  bool exists = (0 == stat(filename.c_str(), &statbuf));
  /* Avoid removing non-socket files */
  bool is_socket_type = (statbuf.st_mode & S_IFMT) == S_IFSOCK;
  return exists && is_socket_type;
}

bool socket_exists(std::string const& socket_name) {
  try {
    std::string socket_path{socket_name};

    /* In case an abstract socket name exists with the same name*/
    socket_path.insert(std::begin(socket_path), ' ');

    /* If the name is contained in this table, it signifies
     * a process is truly using that socket connection
     */
    std::ifstream socket_names_file("/proc/net/unix");
    std::string line;
    while (std::getline(socket_names_file, line)) {
      auto index = line.find(socket_path);
      /* check for complete match */
      if (index != std::string::npos &&
          (index + socket_path.length()) == line.length()) {
        return true;
      }
    }
  } catch (...) {
    /* Assume the socket exists */
    return true;
  }
  return false;
}

std::string remove_socket_if_stale(std::string const& socket_name) {
  if (socket_file_exists(socket_name) && !socket_exists(socket_name)) {
    if (std::remove(socket_name.c_str()) != 0) {
      throw std::runtime_error("Failed removing stale socket file");
    }
  }
  return socket_name;
}
}

namespace binderd {
ServerImpl::ServerImpl(const std::string &socket_path) :
  io_{new ba::io_service},
  acceptor_(*io_, remove_socket_if_stale(socket_path)),
  socket_(*io_),
  sessions_(std::make_shared<ServerSessions>()),
  registry_(Registry::Create()) {
  StartAccept();
  ::chmod(socket_path.c_str(), 0777);
}

ServerImpl::~ServerImpl() {
  Stop();
}

void ServerImpl::CloseAllSessions() {
  sessions_->Clear();
}

void ServerImpl::StartAccept() {
  acceptor_.async_accept(socket_, [this](const asio::error_code &err) {
    if (!err) {
      auto messenger = std::make_shared<SocketMessenger>(io_, std::move(socket_));
      auto session = ServerSession::Create(sessions_, io_, messenger, registry_);
      sessions_->Add(session);
      session->Start();
    } else {
      ERROR("Error while accepting connection: %s", err.message());
    }
    StartAccept();
  });
}

void ServerImpl::Run() {
  while (!exit_) {
    try {
      io_->run();
    } catch (std::exception &err) {
      ERROR("%s", err.what());
    }
  }
}

void ServerImpl::RunAsync(std::size_t num_workers) {
  for (unsigned int i = 0; i < num_workers; i++)
    workers_.push_back(std::thread{[this]() { Run(); }});
}

void ServerImpl::Stop() {
  exit_ = true;
  io_->stop();
  for (auto &worker : workers_) {
    if (worker.joinable())
      worker.join();
  }
}
} // namespace binderd
