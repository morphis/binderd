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

#ifndef BINDER_COMMON_BINARY_READER_H_
#define BINDER_COMMON_BINARY_READER_H_

#include "binderd/buffer.h"

#include <cstdint>
#include <vector>
#include <string>

namespace binderd {
class BinaryReader {
 public:
  explicit BinaryReader(const BufferPtr &buffer, const std::size_t &offset = 0);

  ~BinaryReader();

  BinaryReader(const BinaryReader&) = delete;
  BinaryReader& operator=(const BinaryReader&) = delete;

  BinaryReader(BinaryReader&&);
  BinaryReader& operator=(BinaryReader&&);

  template<typename T>
  T ReadValue();
  std::uint16_t ReadUint16();
  std::uint32_t ReadUint32();
  std::uint64_t ReadUint64();
  std::int16_t ReadInt16();
  std::int32_t ReadInt32();
  std::int64_t ReadInt64();
  BufferPtr ReadData(std::size_t size);
  BufferPtr ReadSizedData();
  std::string ReadString();
  std::u16string ReadString16();

  template<typename T>
  void Skip();

  std::size_t GetBytesRead() const;
  std::size_t GetBytesLeft() const;

  void SetPadding(bool enable);

 private:
  struct Implementation;
  std::unique_ptr<Implementation> impl;
};
} // namespace binderd

#endif
