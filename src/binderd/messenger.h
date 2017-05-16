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

#ifndef BINDERD_MESSENGER_H_
#define BINDERD_MESSENGER_H_

#include "binderd/buffer.h"

#include <functional>

#include <asio.hpp>

namespace binderd {
class Messenger {
 public:
  typedef std::function<void(asio::error_code const&, const BufferPtr&)> ReadHandler;

  virtual ~Messenger() {}

  virtual bool Connect(const std::string &socket_path) = 0;
  virtual void Close() = 0;

  virtual void ReadMessageAsync(const ReadHandler &handler) = 0;
  virtual void WriteData(const BufferPtr &data) = 0;

  virtual void Flush() = 0;
};
} // namespace binderd

#endif
