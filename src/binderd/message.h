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

#ifndef BINDER_MESSAGE_H_
#define BINDER_MESSAGE_H_

#include "binderd/buffer.h"
#include "binderd/common/binary_writer.h"
#include "binderd/common/binary_reader.h"

#include <memory>
#include <ostream>
#include <cstdint>

namespace binderd {
class Message {
 public:
  enum class Type : std::uint32_t {
    Unknown = 0,

    // These three types are specific for transactions. A transaction
    // is always answered once it arrives at the target on the initator
    // side with a Status message. If the transaction expects a reply
    // a message of type Reply will be send by the target once done with it.
    Transaction,
    TransactionReply,

    // Acquire tells the receiver that a reference for the supplied
    // object needs to be taken.
    Acquire,

    // Release tells the receiver that a reference for the supplied
    // object can be freed.
    Release,

    RequestDeathNotification,
    ClearDeathNotification,
    DeadBinder,

    SetContextMgr,
    SetMonitor,
    Version,

    // Message sent to the monitor for each logged message.
    LogEntry,

    // Status is sent as reply for all non transaction messages like
    // SetContextMgr and carries a int32_t status field.
    Status,
  };

  static std::shared_ptr<Message> Create(const Type &GetType = Type::Unknown);
  static std::shared_ptr<Message> CreateFromData(const BufferPtr &data,
                                                 const std::size_t &offset = 0);

  ~Message();

  BinaryWriter GetWriter();
  BinaryReader GetReader() const;

  Type GetType() const;
  std::uint64_t GetCookie() const;
  std::uint64_t GetDestination() const;

  void SetCookie(std::uint64_t cookie);
  void SetDestination(std::uint64_t destination);

  BufferPtr Pack() const;
  std::size_t Unpack(const BufferPtr &data, const std::size_t &offset = 0);

 private:
  Message(const Type &GetType);

  struct Implementation;
  std::unique_ptr<Implementation> impl;
};
typedef std::shared_ptr<Message> MessagePtr;
std::ostream& operator<<(std::ostream &out, const Message::Type &type);
std::ostream& operator<<(std::ostream &out, const Message &msg);
} // namespace binderd

#endif
