/*
 * Copyright © 2014 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Alexandros Frantzis <alexandros.frantzis@canonical.com>
 */

#ifndef BINDER_COMMON_VARIABLE_LENGTH_ARRAY_H_
#define BINDER_COMMON_VARIABLE_LENGTH_ARRAY_H_

#include <sys/types.h>
#include <memory>

namespace binderd {
template <size_t BuiltInBufferSize>
class VariableLengthArray {
 public:
  explicit VariableLengthArray(size_t size) : size_{size} {
    /* Don't call resize if the initial values of member variables are valid */
    if (size > BuiltInBufferSize) Resize(size);
  }

  void Resize(size_t size) {
    if (size > BuiltInBufferSize)
      effective_buffer = BufferUPtr{new unsigned char[size], HeapDeleter};
    else
      effective_buffer = BufferUPtr{builtin_buffer, NullDeleter};

    size_ = size;
  }

  std::uint8_t* GetData() const { return effective_buffer.get(); }
  size_t GetSize() const { return size_; }

 private:
  typedef std::unique_ptr<unsigned char, void (*)(unsigned char*)> BufferUPtr;

  static void NullDeleter(unsigned char*) {}
  static void HeapDeleter(unsigned char* b) { delete[] b; }

  std::uint8_t builtin_buffer[BuiltInBufferSize];
  BufferUPtr effective_buffer{builtin_buffer, NullDeleter};
  size_t size_;
};
}  // namespace binderd

#endif
