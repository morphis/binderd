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

#include <gtest/gtest.h>

#include "binderd/message.h"
#include "binderd/common/utils.h"
#include "binderd/logger.h"

TEST(Message, CanBeConstructed) {
  auto msg = binderd::Message::Create();
  ASSERT_NE(msg, nullptr);
}

TEST(Message, ReadWrite) {
  auto msg = binderd::Message::Create();

  auto writer = msg->GetWriter();
  writer.WriteUint16(10);
  writer.WriteUint32(0x22334455);
  writer.WriteUint64(0x2233445588991122);
  writer.WriteString("uuupps");
  writer.WriteInt16(4455);
  writer.WriteInt32(-1122);
  writer.WriteInt64(123213);

  auto reader = msg->GetReader();
  ASSERT_EQ(reader.ReadUint16(), 10);
  ASSERT_EQ(reader.ReadUint32(), 0x22334455);
  ASSERT_EQ(reader.ReadUint64(), 0x2233445588991122);
  ASSERT_EQ(reader.ReadString(), "uuupps");
  ASSERT_EQ(reader.ReadInt16(), 4455);
  ASSERT_EQ(reader.ReadInt32(), -1122);
  ASSERT_EQ(reader.ReadInt64(), 123213);
}

TEST(Message, ReadOnEmptyBufferThrowsException) {
  auto msg = binderd::Message::Create();
  auto reader = msg->GetReader();
  ASSERT_THROW(reader.ReadUint16(), std::out_of_range);
  ASSERT_THROW(reader.ReadUint32(), std::out_of_range);
  ASSERT_THROW(reader.ReadUint64(), std::out_of_range);
  ASSERT_THROW(reader.ReadString(), std::out_of_range);
}

TEST(Message, ReadThrowsOnOverread) {
  auto msg = binderd::Message::Create();

  auto writer = msg->GetWriter();
  writer.WriteUint16(10);

  auto reader = msg->GetReader();
  ASSERT_EQ(reader.ReadUint16(), 10);
  ASSERT_THROW(reader.ReadUint16(), std::out_of_range);
}

TEST(Message, PackAndUnpack) {
  auto msg = binderd::Message::Create(binderd::Message::Type::Transaction);

  auto writer = msg->GetWriter();
  writer.WriteUint32(12345);
  writer.WriteString("foobar");

  auto buffer = msg->Pack();

  msg = binderd::Message::Create();
  msg->Unpack(buffer);

  ASSERT_EQ(msg->GetType(), binderd::Message::Type::Transaction);

  auto reader = msg->GetReader();
  ASSERT_EQ(reader.ReadUint32(), 12345);
  ASSERT_EQ(reader.ReadString(), "foobar");
}
