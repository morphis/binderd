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

#ifndef BINDER_BUFFER_H_
#define BINDER_BUFFER_H_

#include <cstdint>
#include <vector>
#include <memory>

namespace binderd {
class Buffer {
 public:
  using BaseType = std::uint8_t;

  static std::shared_ptr<Buffer> Create(std::size_t size = 0);
  static std::shared_ptr<Buffer> Create(std::uint8_t *data, std::size_t size);

  Buffer(std::size_t size);
  Buffer(std::uint8_t *data, std::size_t size);

  std::uint8_t* GetBegin() const;
  std::uint8_t* GetEnd() const;

  std::size_t GetSize() const;
  std::uint8_t* GetData() const;

  void Resize(std::size_t GetSize);

 private:
  struct Implementation;
  std::shared_ptr<Implementation> impl;
};

using BufferPtr = std::shared_ptr<Buffer>;
BufferPtr CreateBufferWithSize(std::size_t size);
} // namespace binderd

#endif
