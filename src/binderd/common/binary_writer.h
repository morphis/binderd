/*
 * Copyright (C) 2016 Thomas Voss <thomas.voss.bochum@gmail.com>
 *                    Simon Fels <morphis@gravedo.de>
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

#ifndef BINDER_COMMON_BINARY_WRITER_H_
#define BINDER_COMMON_BINARY_WRITER_H_

#include "binderd/buffer.h"

#include <cstdint>
#include <vector>
#include <string>

namespace binderd {
class BinaryWriter {
 public:
  explicit BinaryWriter(const BufferPtr &buffer);
  BinaryWriter();

  ~BinaryWriter();

  BinaryWriter(const BinaryWriter&) = delete;
  BinaryWriter& operator=(const BinaryWriter&) = delete;

  BinaryWriter(BinaryWriter&&);
  BinaryWriter& operator=(BinaryWriter&&);

  template<typename T>
  void WriteValue(T value);
  void WriteUint16(std::uint16_t value);
  void WriteUint32(std::uint32_t value);
  void WriteUint64(std::uint64_t value);
  void WriteInt16(std::int16_t value);
  void WriteInt32(std::int32_t value);
  void WriteInt64(std::int64_t value);
  void WriteData(const std::uint8_t *data, std::size_t size);
  void WriteData(const BufferPtr &buffer);
  void WriteSizedData(const std::uint8_t *data, std::size_t size);
  void WriteSizedData(const BufferPtr &buffer);
  void WriteString(const std::string &str);
  void WriteString(const std::u16string &str);
  void WriteString(const char *s, std::size_t size);

  std::size_t GetBytesWritten() const;
  BufferPtr Finalize() const;

  void SetPadding(bool enable);

 private:
  struct Implementation;
  std::unique_ptr<Implementation> impl;
};
} // namespace binderd

#endif
