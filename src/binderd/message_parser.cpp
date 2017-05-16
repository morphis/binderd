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

#include "binderd/message_parser.h"

namespace binderd {
MessageParser::MessageParser(const BufferPtr &buffer, const std::size_t &size) :
  buffer{buffer}, bytes_available{size}, consumed{0} {}

MessageParser::~MessageParser() {}

MessagePtr MessageParser::Next() {
  if (consumed >= bytes_available)
    return nullptr;

  auto msg = Message::Create();
  const auto used = msg->Unpack(buffer, consumed);
  if (used <= 0)
    return nullptr;

  consumed += used;

  return msg;
}

std::size_t MessageParser::GetBytesConsumed() const { return consumed; }
} // namespace binderd
