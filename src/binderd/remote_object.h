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

#ifndef BINDERD_REMOTE_OBJECT_H_
#define BINDERD_REMOTE_OBJECT_H_

#include "binderd/object.h"
#include "binderd/macros.h"
#include "binderd/client.h"

#include <mutex>

namespace binderd {
class RemoteObject : public Object,
                     public std::enable_shared_from_this<RemoteObject> {
 public:
  static std::shared_ptr<RemoteObject> Create(const ClientPtr &client, uint64_t handle);

  ~RemoteObject();

  uint64_t GetHandle() const { return handle_; }

  Type GetType() const override;
  Status Transact(const ClientPtr &client,
                  std::unique_ptr<TransactionData> request,
                  std::unique_ptr<TransactionData> *reply) override;

  Status LinkToDeath(const ClientPtr &client, const std::shared_ptr<DeathRecipient> &recipient) override;
  Status UnlinkToDeath(const ClientPtr &client, const std::shared_ptr<DeathRecipient> &recipient) override;

  void SendObituary() override;

 private:
  RemoteObject(const ClientPtr &client, uint64_t handle);

  std::mutex lock_;
  uint64_t handle_;
  Client::ReleaseReferenceFunc release_ref_;
  bool obituaries_sent_ = false;
  std::vector<std::shared_ptr<DeathRecipient>> obituaries_;

  DISALLOW_COPY_AND_ASSIGN(RemoteObject);
};
} // namespace binderd

#endif
