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

#ifndef BINDERD_LOCAL_OBJECT_H_
#define BINDERD_LOCAL_OBJECT_H_

#include "binderd/object.h"
#include "binderd/macros.h"

namespace binderd {
class LocalObject : public Object {
 public:
  class TransactionHandler {
   public:
    virtual ~TransactionHandler() {}

    virtual Status OnTransact(
        const ClientPtr &client,
        std::unique_ptr<TransactionData> request,
        std::unique_ptr<TransactionData> *reply) = 0;
  };

  explicit LocalObject(std::unique_ptr<TransactionHandler> handler);
  ~LocalObject();

  Type GetType() const override;
  Status Transact(const ClientPtr &client,
                  std::unique_ptr<TransactionData> request,
                  std::unique_ptr<TransactionData> *reply) override;

  Status LinkToDeath(const ClientPtr &client, const std::shared_ptr<DeathRecipient> &recipient) override;
  Status UnlinkToDeath(const ClientPtr &client, const std::shared_ptr<DeathRecipient> &recipient) override;

 private:
  std::unique_ptr<TransactionHandler> handler_;

  DISALLOW_COPY_AND_ASSIGN(LocalObject);
};
} // namespace binderd

#endif
