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

#include "binderd/object_pool.h"
#include "binderd/logger.h"

namespace binderd {
ObjectPool::ObjectPool() {}

ObjectPool::~ObjectPool() {}

void ObjectPool::AddRef(const uintptr_t &cookie) {
  auto item = references_.find(cookie);
  if (item == references_.end())
    throw std::runtime_error("Unknown object");
  auto &ref = item->second;
  ref.count++;
  ref.obj->OnReferenceAcquired();
}

void ObjectPool::Release(const uintptr_t &cookie) {
  auto item = references_.find(cookie);
  if (item == references_.end())
    throw std::runtime_error("Unknown object");

  auto &ref = item->second;
  ref.count--;
  if (ref.count == 0)
    references_.erase(item);

  ref.obj->OnReferenceReleased();
}

void ObjectPool::AdoptObject(const std::shared_ptr<Object> &obj) {
  const auto cookie = reinterpret_cast<uintptr_t>(obj.get());
  auto item = references_.find(cookie);
  if (item != references_.end())
    throw std::runtime_error("Object already tracked");

  Reference ref{0, obj};
  references_.insert({cookie, ref});
}

std::shared_ptr<Object> ObjectPool::GetObject(const uintptr_t &cookie) {
  auto item = references_.find(cookie);
  if (item == references_.end())
    return nullptr;

  return item->second.obj;
}

size_t ObjectPool::GetReferencesCount() const {
  return references_.size();
}

bool ObjectPool::Contains(const uintptr_t &cookie) {
  return (references_.find(cookie) != references_.end());
}
} // namespace binderd
