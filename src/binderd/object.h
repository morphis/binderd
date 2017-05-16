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

#ifndef BINDERD_OBJECT_H_
#define BINDERD_OBJECT_H_

#include "binderd/transaction_data.h"

namespace binderd {
class Client;
using ClientPtr = std::shared_ptr<Client>;
class Object {
 public:
  enum class Type {
    Local,
    Remote,
  };

  virtual ~Object() {}

  virtual Type GetType() const = 0;
  virtual Status Transact(const ClientPtr &client,
                          std::unique_ptr<TransactionData> request,
                          std::unique_ptr<TransactionData> *reply) = 0;

  class DeathRecipient {
   public:
    virtual ~DeathRecipient() {}
    virtual void OnObjectDied(const Object *who) = 0;
  };

  virtual Status LinkToDeath(const ClientPtr &client, const std::shared_ptr<DeathRecipient> &recipient) = 0;
  virtual Status UnlinkToDeath(const ClientPtr &client, const std::shared_ptr<DeathRecipient> &recipient) = 0;

  virtual void SendObituary() {}

  virtual void OnReferenceAcquired() {}
  virtual void OnReferenceReleased() {}
};
using ObjectPtr = std::shared_ptr<Object>;
} // namespace binderd

#endif
