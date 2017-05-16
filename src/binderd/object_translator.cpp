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

#include "binderd/object_translator.h"
#include "binderd/binder_api.h"
#include "binderd/logger.h"

namespace binderd {
ObjectTranslator::ObjectTranslator() {}

ObjectTranslator::~ObjectTranslator() {}

uintptr_t ObjectTranslator::TranslateCookie(uint64_t cookie) {
  auto iter = objects_.find(cookie);
  if (iter == objects_.end())
    return 0;
  return iter->second;
}

Status ObjectTranslator::ProcessTransaction(TransactionData *request, const Direction &direction) {
  if (request->GetNumObjectOffsets() == 0)
    return Status::OK;

  // FIXME: We really don't want the const_cast<> here.
  Buffer objects(const_cast<uint8_t*>(request->GetObjectOffsets()),
                 request->GetNumObjectOffsets() * sizeof(binder_size_t));

  auto current = objects.GetBegin();
  while (current != objects.GetEnd()) {
    auto offset = static_cast<binder_size_t>(*current);
    auto obj = reinterpret_cast<flat_binder_object*>(request->GetMutableData() + offset);

    switch (obj->type) {
    case BINDER_TYPE_BINDER: {
      auto status = ProcessBinderObject(obj, direction);
      if (status != Status::OK)
        return status;
      break;
    }
    case BINDER_TYPE_HANDLE: {
      // If we pass a handle out or receive one we don't have to do
      // anything as if we receive one we will use it construct a
      // proxy for it and in the other case we just forward it.
      break;
    }
    case BINDER_TYPE_WEAK_BINDER:
    case BINDER_TYPE_WEAK_HANDLE:
      WARNING("Got a weak binder/handle which we can't handle yet");
      break;
    case BINDER_TYPE_FD:
      // FIXME Need to support file descriptors
      break;
    default:
      WARNING("Invalid object type %d", obj->type);
      return Status::InvalidOperation;
    }

    current += sizeof(binder_size_t);
  }

  return Status::OK;
}

Status ObjectTranslator::ProcessBinderObject(flat_binder_object *obj, const Direction &direction) {
  switch (direction) {
  case Direction::In: {
    auto iter = objects_.find(obj->cookie);
    if (iter == objects_.end())
      return Status::BadValue;

    // Give the object back its real memory address so that it can
    // be called based on the transaction data.
    obj->cookie = iter->second;
    break;
  }
  case Direction::Out: {
    // We convert the real cookie into a fake one here so that the
    // server never stores memory addresses of our objects. If it
    // would do this and gets compromised it could execute arbitrary
    // code at a memory location of its choice as we don't check the
    // memory addresses and rely on them being the right one for
    // the object being called.
    //
    // In a world with a binder driver inside the kernel we have a
    // different trust situation. A userland process should be
    // considered unsafe here.
    //
    // To avoid passing real memory addresses around we convert them
    // into a fake one which is a key to a LUT we then store the
    // objects real address in. Once we receive a binder object
    // with an incoming transaction again we lookup the right address
    // from the table and pass it to the local object.
    obj->cookie = TranslateOrAddObject(obj->cookie);
    break;
  }
  default:
    break;
  }
  return Status::OK;
}

uint64_t ObjectTranslator::TranslateObject(uintptr_t object) {
  for (auto iter : objects_) {
    if (iter.second == object)
      return iter.first;
  }
  return Invalid;
}

uint64_t ObjectTranslator::TranslateOrAddObject(uintptr_t object) {
  auto id = TranslateObject(object);
  if (id != Invalid)
    return id;

  id = next_object_id_++;
  objects_.insert({id, object});
  return id;
}
} // namespace binderd
