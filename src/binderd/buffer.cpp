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

#include "binderd/buffer.h"

#include <memory.h>

namespace binderd {

struct Buffer::Implementation {
  Implementation(std::size_t size) : size_{size} {
    allocate(size);
  }

  Implementation(std::uint8_t *data, std::size_t size) :
    size_{size},
    data_{data},
    immutable_{true} {}

  ~Implementation() {
    if (!immutable_)
      free(data_);
  }

  void allocate(std::size_t size) {
    if (size == 0)
      return;

    data_ = reinterpret_cast<std::uint8_t*>(malloc(size * sizeof(std::uint8_t)));
    if (!data_)
      throw std::runtime_error("Not enough memory to allocate buffer");

    ::memset(data_, 0, size);
    size_ = size;
  }

  void resize(std::size_t size) {
    if (immutable_)
      throw std::runtime_error("Tried to resize an immutable buffer");

    data_ = reinterpret_cast<std::uint8_t*>(realloc(data_, size));
    if (!data_)
      throw std::runtime_error("Not enough memory to allocate buffer");

    size_ = size;
  }

  std::uint8_t *data_ = nullptr;
  std::size_t size_ = 0;
  bool immutable_ = false;
};

std::shared_ptr<Buffer> Buffer::Create(std::size_t size) {
  return std::shared_ptr<Buffer>(new Buffer(size));
}

std::shared_ptr<Buffer> Buffer::Create(std::uint8_t *data, std::size_t size) {
  return std::shared_ptr<Buffer>(new Buffer(data, size));
}

Buffer::Buffer(std::size_t size) :
  impl{new Implementation(size)} {}

Buffer::Buffer(std::uint8_t *data, std::size_t size) :
  impl{new Implementation(data, size)} {}

std::uint8_t* Buffer::GetBegin() const {
  return impl->data_;
}

std::uint8_t* Buffer::GetEnd() const {
  return impl->data_ + impl->size_;
}

std::size_t Buffer::GetSize() const {
  return impl->size_;
}

std::uint8_t* Buffer::GetData() const {
  return impl->data_;
}

void Buffer::Resize(std::size_t size) {
  impl->resize(size);
}

BufferPtr CreateBufferWithSize(std::size_t size) {
  return std::make_shared<Buffer>(size);
}
} // namespace binderd
