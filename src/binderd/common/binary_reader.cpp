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

#include "binderd/common/binary_reader.h"
#include "binderd/logger.h"

#include <cstring>
#include <stdexcept>

namespace {
// This macro should never be used at runtime, as a too large value
// of s could cause an integer overflow. Instead, you should always
// use the wrapper function pad_size()
#define PAD_SIZE_UNSAFE(s) (((s)+3)&~3)

#define SIZE_T_MAX UINT32_MAX

static size_t pad_size(std::size_t s) {
    if (s > (SIZE_T_MAX - 3))
      throw std::runtime_error("");
    return PAD_SIZE_UNSAFE(s);
}
}

namespace binderd {
struct BinaryReader::Implementation {
  Implementation(const BufferPtr &buffer, const std::size_t &offset) :
    buffer{buffer} {
    if (buffer) {
      begin = buffer->GetBegin();
      current = buffer->GetBegin() + offset;
      end = buffer->GetEnd();
    }
  }

  Buffer::BaseType *begin;
  Buffer::BaseType *current;
  Buffer::BaseType *end;
  BufferPtr buffer;
  bool use_padding_ = true;
};

BinaryReader::BinaryReader(const BufferPtr &buffer, const std::size_t &offset) :
  impl{new Implementation{buffer, offset}} {}

BinaryReader::~BinaryReader() {}

BinaryReader::BinaryReader(BinaryReader&& that) : impl(std::move(that.impl)) {}

BinaryReader& BinaryReader::operator=(BinaryReader&& rhs) {
    impl = std::move(rhs.impl);
    return *this;
}

std::size_t BinaryReader::GetBytesRead() const {
  return (impl->current - impl->begin);
}

std::size_t BinaryReader::GetBytesLeft() const {
  return impl->end - impl->current;
}

template<typename T>
T BinaryReader::ReadValue() {
  if (impl->current + sizeof(T) > impl->end)
    throw std::out_of_range{"Read buffer exhausted"};

  T v = static_cast<T>(*impl->current);
  impl->current += sizeof(v);

  return v;
}

std::uint16_t BinaryReader::ReadUint16() {
  return ReadValue<std::uint16_t>();
}

std::uint32_t BinaryReader::ReadUint32() {
  return ReadValue<std::uint32_t>();
}

std::uint64_t BinaryReader::ReadUint64() {
  return ReadValue<std::uint64_t>();
}

std::int16_t BinaryReader::ReadInt16() {
  return ReadValue<std::int16_t>();
}

std::int32_t BinaryReader::ReadInt32() {
  return ReadValue<std::int32_t>();
}

std::int64_t BinaryReader::ReadInt64() {
  return ReadValue<std::int64_t>();
}

BufferPtr BinaryReader::ReadData(std::size_t size) {
  if (size <= 0)
    return Buffer::Create(0);

  const auto padded = impl->use_padding_ ? pad_size(size) : size;

  if (impl->current + padded > impl->end)
    throw std::out_of_range{"Read buffer exhausted"};

  auto data = Buffer::Create(impl->current, size);
  ::memcpy(data->GetData(), impl->current, size);

  impl->current += padded;

  return data;
}

BufferPtr BinaryReader::ReadSizedData() {
  auto size = ReadInt32();
  return ReadData(size);
}

std::string BinaryReader::ReadString() {
  const auto size = ReadInt32();

  const auto data_size = (size + 1);
  const auto padded = impl->use_padding_ ? pad_size(data_size) : data_size;

  if (impl->current + padded > impl->end)
    throw std::out_of_range{"Read buffer exhausted"};

  auto data = CreateBufferWithSize(data_size);
  ::memcpy(data->GetData(), &(*impl->current), data_size);

  impl->current += padded;

  return std::string(reinterpret_cast<const char*>(data->GetData()), size);
}

std::u16string BinaryReader::ReadString16() {
  const auto size = ReadInt32();

  const auto data_size = (size + 1) * sizeof(uint16_t);
  const auto padded = impl->use_padding_ ? pad_size(data_size) : data_size;

  if (impl->current + padded > impl->end)
    throw std::out_of_range{"Read buffer exhausted"};

  auto data = CreateBufferWithSize(data_size);
  ::memcpy(data->GetData(), &(*impl->current), data_size);

  impl->current += padded;

  return std::u16string(reinterpret_cast<char16_t*>(data->GetData()), size);
}

template<typename T>
void BinaryReader::Skip() {
  const auto size = sizeof(T);
  if (impl->current + size > impl->end)
    throw std::out_of_range{"Read buffer exhausted"};
  impl->current += size;
}

void BinaryReader::SetPadding(bool enable) {
  impl->use_padding_ = enable;
}
} // namespace binderd
