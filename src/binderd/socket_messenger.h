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

#ifndef BINDERD_SOCKET_MESSENGER_H_
#define BINDERD_SOCKET_MESSENGER_H_

#include "binderd/messenger.h"

#include <deque>
#include <memory>
#include <cstdint>

namespace binderd {
class ClientImpl;
class SocketMessenger : public Messenger {
 public:
  explicit SocketMessenger(const std::shared_ptr<asio::io_service> &io);
  SocketMessenger(const std::shared_ptr<asio::io_service> &io,
                  asio::local::stream_protocol::socket socket);
  ~SocketMessenger();

  bool Connect(const std::string &socket_path) override;
  void Close() override;
  void ReadMessageAsync(const ReadHandler &handler) override;
  void WriteData(const BufferPtr &data) override;
  void Flush() override;

 protected:
  asio::local::stream_protocol::socket socket_;
  asio::strand write_strand_;
  std::atomic_bool exit_{false};

 private:
  asio::error_code ReadBuffer(const asio::mutable_buffers_1 &buffer);
  size_t GetAvailableBytes();
  void OnMessageSent(const asio::error_code &err);
  void WriteNext();

  std::deque<BufferPtr> write_queue_;
  static const size_t header_size = 2;
  std::uint8_t header_[header_size];
  BufferPtr body_;
};
} // namespace binderd

#endif
