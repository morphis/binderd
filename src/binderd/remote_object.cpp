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

#include "binderd/remote_object.h"
#include "binderd/client.h"
#include "binderd/logger.h"

namespace binderd {
std::shared_ptr<RemoteObject> RemoteObject::Create(const ClientPtr &client, uint64_t handle) {
  return std::shared_ptr<RemoteObject>(new RemoteObject(client, handle));
}

RemoteObject::RemoteObject(const ClientPtr &client, uint64_t handle) : handle_{handle} {
  if (!client)
    return;

  client->AddReference(handle_);
  release_ref_ = client->GetReleaseReferenceClosure(handle);
}

RemoteObject::~RemoteObject() {
  if (release_ref_)
    release_ref_();
}

Object::Type RemoteObject::GetType() const {
  return Object::Type::Remote;
}

Status RemoteObject::Transact(const ClientPtr &client,
                              std::unique_ptr<TransactionData> request,
                              std::unique_ptr<TransactionData> *reply) {
  return client->Transact(handle_, std::move(request), reply);
}

Status RemoteObject::LinkToDeath(const ClientPtr &client, const std::shared_ptr<DeathRecipient> &recipient) {
  std::unique_lock<std::mutex> l(lock_);

  if (obituaries_sent_)
    return Status::DeadObject;

  auto status = client->RequestDeathNotification(handle_, shared_from_this());
  if (status != Status::OK)
    return status;

  obituaries_.push_back(recipient);

  return Status::OK;
}

Status RemoteObject::UnlinkToDeath(const ClientPtr &client, const std::shared_ptr<DeathRecipient> &recipient) {
  std::unique_lock<std::mutex> l(lock_);

  if (obituaries_sent_)
    return Status::DeadObject;

  auto iter = std::find(obituaries_.begin(), obituaries_.end(), recipient);
  if (iter == obituaries_.end())
    return Status::NameNotFound;

  auto status = client->ClearDeathNotification(handle_, shared_from_this());
  if (status != Status::OK)
    return status;

  obituaries_.erase(iter);

  return Status::OK;
}

void RemoteObject::SendObituary() {
  if (obituaries_sent_)
    return;

  for (auto recipient : obituaries_)
    recipient->OnObjectDied(this);

  obituaries_sent_ = true;
}
} // namespace binderd
