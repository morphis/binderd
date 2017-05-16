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

#ifndef BINDERD_COMMON_REF_COUNTED_H_
#define BINDERD_COMMON_REF_COUNTED_H_

#include "binderd/macros.h"

#include <atomic>
#include <cstdint>

namespace binderd {
template<typename T>
class RefCounted {
 public:
  explicit RefCounted() : ref_count_(0) {}

  void AddRef() const {
    ++ref_count_;
  }

  void Release() const {
    if (ref_count_ > 0)
      --ref_count_;
    if (ref_count_ == 0) {
      delete static_cast<const T*>(this);
    }
  }

  uint32_t GetCount() const { return ref_count_; }

 protected:
  ~RefCounted() = default;

 private:
  mutable std::atomic<uint32_t> ref_count_;

  DISALLOW_COPY_AND_ASSIGN(RefCounted);
};
} // namespace binderd

#endif
