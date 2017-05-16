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

#include "binderd/socket_messenger.h"
#include "binderd/logger.h"

#include <functional>

namespace binderd {
SocketMessenger::SocketMessenger(const std::shared_ptr<asio::io_service> &io) :
  socket_(asio::local::stream_protocol::socket(*io)),
  write_strand_(*io),
  body_{CreateBufferWithSize(0)} {}

SocketMessenger::SocketMessenger(const std::shared_ptr<asio::io_service> &io,
                                 asio::local::stream_protocol::socket socket) :
  socket_(std::move(socket)),
  write_strand_(*io),
  body_{CreateBufferWithSize(0)} {}

SocketMessenger::~SocketMessenger() {}

void SocketMessenger::OnMessageSent(const asio::error_code &err) {
  if (err) {
    if (err == asio::error::bad_descriptor)
      return;

    ERROR("Failed to write message: %s (%d)", err.message(), err.value());
    return;
  }

  write_queue_.pop_front();
  if (write_queue_.size() > 0)
    WriteNext();

  if (exit_) {
    socket_.shutdown(asio::local::stream_protocol::socket::shutdown_both);
    socket_.close();
  }
}

bool SocketMessenger::Connect(const std::string &socket_path) {
  asio::error_code err;
  socket_.connect(asio::local::stream_protocol::endpoint(socket_path), err);
  return !err;
}

void SocketMessenger::Close() {
  exit_ = true;
}

void SocketMessenger::WriteNext() {
  if (exit_)
    return;

  auto &item = write_queue_.front();
  auto handler = write_strand_.wrap(std::bind(&SocketMessenger::OnMessageSent, this, std::placeholders::_1));

  auto whole_message = CreateBufferWithSize(header_size + item->GetSize());
  whole_message->GetData()[0] = static_cast<std::uint8_t>((item->GetSize() >> 8) & 0xff);
  whole_message->GetData()[1] = static_cast<std::uint8_t>((item->GetSize() >> 0) & 0xff);
  std::copy(item->GetData(), item->GetData() + item->GetSize(), whole_message->GetData() + header_size);

  asio::async_write(socket_, asio::buffer(whole_message->GetData(), whole_message->GetSize()), handler);
}

void SocketMessenger::Flush() {
}

void SocketMessenger::WriteData(const BufferPtr &data) {
  write_queue_.push_back(data);
  if (write_queue_.size() > 1)
    return;

  WriteNext();
}

void SocketMessenger::ReadMessageAsync(const ReadHandler &handler) {
  auto buffer = asio::buffer(header_, header_size);
  asio::async_read(socket_,
                 buffer,
                 asio::transfer_exactly(asio::buffer_size(buffer)),
                 [this, handler](const asio::error_code &err, size_t) {
    if (err) {
      handler(err, nullptr);
      return;
    }

    const size_t body_size = (header_[0] << 8) + header_[1];
    body_->Resize(body_size);

    if (GetAvailableBytes() >= body_size) {
      handler(ReadBuffer(asio::buffer(body_->GetData(), body_->GetSize())), body_);
    } else {
      auto buffer = asio::buffer(body_->GetData(), body_->GetSize());
      asio::async_read(socket_,
                     buffer,
                     asio::transfer_exactly(asio::buffer_size(buffer)),
                     [this, handler](const asio::error_code &err, size_t) { handler(err, body_); });
    }
  });
}

asio::error_code SocketMessenger::ReadBuffer(const asio::mutable_buffers_1 &buffer) {
  asio::error_code err;
  size_t nread = 0;

  while (nread < asio::buffer_size(buffer)) {
    nread += asio::read(socket_,
                      asio::mutable_buffers_1{buffer+nread},
                      err);

    if (err && err!= asio::error::would_block)
      break;
  }

  return err;
}

size_t SocketMessenger::GetAvailableBytes() {
  asio::socket_base::bytes_readable cmd{true};
  socket_.io_control(cmd);
  return cmd.get();
}
} // namespace binderd
