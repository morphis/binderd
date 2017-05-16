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

#ifndef BINDERD_OBJECT_POOL_H_
#define BINDERD_OBJECT_POOL_H_

#include "binderd/macros.h"
#include "binderd/object.h"

#include <unordered_map>
#include <type_traits>

namespace binderd {
class ObjectPool {
 public:
  ObjectPool();
  ~ObjectPool();

  template<typename T, typename... Ts>
  inline std::shared_ptr<T> New(Ts&&... args) {
    static_assert(std::is_base_of<Object, T>::value,
                  "Type needs to be derived from binderd::Object");
    auto obj = std::shared_ptr<T>(new T(std::forward<Ts>(args)...));
    Reference ref{0, obj};
    references_.insert(std::make_pair(reinterpret_cast<uintptr_t>(obj.get()), ref));
    return obj;
  }

  void AddRef(const uintptr_t &cookie);
  void Release(const uintptr_t &cookie);

  std::shared_ptr<Object> GetObject(const uintptr_t &cookie);

  void AdoptObject(const std::shared_ptr<Object> &obj);

  size_t GetReferencesCount() const;

  bool Contains(const uintptr_t &cookie);

 private:
  struct Reference {
    uint32_t count;
    ObjectPtr obj;
  };

  std::unordered_map<uintptr_t,Reference> references_;
  DISALLOW_COPY_AND_ASSIGN(ObjectPool);
};
using ObjectPoolPtr = std::shared_ptr<ObjectPool>;
} // namespace binderd

#endif
