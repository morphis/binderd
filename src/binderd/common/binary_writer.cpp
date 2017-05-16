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

#include "binderd/common/binary_writer.h"
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

#if BYTE_ORDER == BIG_ENDIAN
static const uint32_t pad_mask[4] = { 0x00000000, 0xffffff00, 0xffff0000, 0xff000000 };
#endif
#if BYTE_ORDER == LITTLE_ENDIAN
static const uint32_t pad_mask[4] = { 0x00000000, 0x00ffffff, 0x0000ffff, 0x000000ff };
#endif
}

namespace binderd {
struct BinaryWriter::Implementation {
  Implementation(const BufferPtr &buffer) :
    buffer{buffer},
    current{buffer->GetBegin()} {}

  void ensure_size(std::size_t size) {
    auto pos = current - buffer->GetBegin();
    if (buffer->GetSize() - pos >= size)
      return;

    buffer->Resize(buffer->GetSize() + size);
    current = buffer->GetBegin() + pos;
  }

  BufferPtr buffer;
  Buffer::BaseType *current;
  bool use_padding_ = true;
};

BinaryWriter::BinaryWriter(const BufferPtr &buffer) :
  impl{new Implementation{buffer}} {}

BinaryWriter::BinaryWriter() :
  impl{new Implementation{CreateBufferWithSize(0)}} {}

BinaryWriter::~BinaryWriter() {}

BinaryWriter::BinaryWriter(BinaryWriter&& that) : impl(std::move(that.impl)) {}

BinaryWriter& BinaryWriter::operator=(BinaryWriter&& rhs) {
  impl = std::move(rhs.impl);
  return *this;
}

std::size_t BinaryWriter::GetBytesWritten() const {
  return impl->current - impl->buffer->GetBegin();
}

BufferPtr BinaryWriter::Finalize() const {
  return std::move(impl->buffer);
}

template<typename T>
void BinaryWriter::WriteValue(T value) {
  const auto size = sizeof(T);
  impl->ensure_size(size);
  *reinterpret_cast<T*>(&(*impl->current)) = value;
  impl->current += size;
}

void BinaryWriter::WriteUint16(std::uint16_t value) {
  WriteValue<std::uint16_t>(value);
}

void BinaryWriter::WriteUint32(std::uint32_t value) {
  WriteValue<std::uint32_t>(value);
}

void BinaryWriter::WriteUint64(std::uint64_t value) {
  WriteValue<std::uint64_t>(value);
}

void BinaryWriter::WriteInt16(std::int16_t value) {
  WriteValue<std::int16_t>(value);
}

void BinaryWriter::WriteInt32(std::int32_t value) {
  WriteValue<std::int32_t>(value);
}

void BinaryWriter::WriteInt64(std::int64_t value) {
  WriteValue<std::int64_t>(value);
}

void BinaryWriter::WriteData(const std::uint8_t *data, std::size_t size) {
  if (size > INT32_MAX)
    throw std::out_of_range("Invalid size");

  const auto padded = impl->use_padding_ ? pad_size(size) : size;
  impl->ensure_size(padded);
  ::memcpy(&(*impl->current), data, size);
  impl->current += size;

  if (padded != size) {
    DEBUG("Adding padding: size %d padded %d", size, padded);
    *reinterpret_cast<uint32_t*>(impl->current + (padded - size - 4)) &= pad_mask[padded - size];
    impl->current += (padded - size);
  }
}

void BinaryWriter::WriteData(const BufferPtr &buffer) {
  WriteData(buffer->GetData(), buffer->GetSize());
}

void BinaryWriter::WriteSizedData(const BufferPtr &buffer) {
  WriteSizedData(buffer->GetData(), buffer->GetSize());
}

void BinaryWriter::WriteSizedData(const std::uint8_t *data, std::size_t size) {
  WriteInt32(size);
  WriteData(data, size);
}

void BinaryWriter::WriteString(const std::string &str) {
  WriteString(str.c_str(), str.size());
}

void BinaryWriter::WriteString(const std::u16string &str) {
  WriteInt32(str.size());
  if (str.size() == 0)
    return;

  WriteData(reinterpret_cast<const uint8_t*>(str.c_str()), (str.size() + 1) * sizeof(uint16_t));
}

void BinaryWriter::WriteString(const char *s, std::size_t size) {
  WriteSizedData(reinterpret_cast<const std::uint8_t*>(s), size + 1);
}

void BinaryWriter::SetPadding(bool enable) {
  impl->use_padding_ = enable;
}
} // namespace binderd
