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

#ifndef BINDERD_OBJECT_TRANSLATOR_H_
#define BINDERD_OBJECT_TRANSLATOR_H_

#include "binderd/macros.h"
#include "binderd/transaction_data.h"
#include "binderd/object.h"
#include "binderd/binder_api.h"

#include <unordered_map>
#include <atomic>

namespace binderd {
class ObjectTranslator {
 public:
  enum class Direction {
    In,
    Out
  };

  static const uint64_t Invalid{0};

  ObjectTranslator();
  ~ObjectTranslator();

  Status ProcessTransaction(TransactionData *request, const Direction &direction);
  uint64_t ProcessObject(const std::shared_ptr<Object> &object);

  uintptr_t TranslateCookie(uint64_t cookie);

  uint64_t TranslateObject(uintptr_t object);
  uint64_t TranslateOrAddObject(uintptr_t object);

 private:
  Status ProcessBinderObject(flat_binder_object *obj, const Direction &direction);

  std::unordered_map<uint64_t,uintptr_t> objects_;
  std::atomic<uint64_t> next_object_id_{1};

  DISALLOW_COPY_AND_ASSIGN(ObjectTranslator);
};
using ObjectTranslatorPtr = std::shared_ptr<ObjectTranslator>;
} // namespace binderd

#endif
