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

#include "binderd/registry.h"
#include "binderd/constants.h"

namespace binderd {
RegistryPtr Registry::Create() {
  auto self = std::shared_ptr<Registry>(new Registry);
  return self;
}

Registry::Registry() : next_object_handle_{1} {}

Registry::~Registry() {}

void Registry::SetContextManager(const ServerSessionPtr &session) {
  context_mgr_ = session;
}

ServerSessionPtr Registry::GetContextManager() const { return context_mgr_; }

void Registry::SetMonitor(const ServerSessionPtr &session) {
  monitor_ = session;
}

ServerSessionPtr Registry::GetMonitor() const { return monitor_; }

ServerSessionPtr Registry::FindSessionByHandle(std::uint64_t handle) {
  if (handle == kContextManagerHandle)
    return GetContextManager();

  auto iter = handles_.find(handle);
  if (iter == handles_.end())
    return nullptr;

  return iter->second;
}

std::uint64_t Registry::NewObjectHandle(const ServerSessionPtr &session) {
  const auto handle = next_object_handle_++;
  handles_.insert({handle, session});
  return handle;
}

void Registry::RemoveHandle(std::uint64_t handle) {
  auto iter = handles_.find(handle);
  if (iter == handles_.end()) {
    WARNING("Cannot remove unknown handle %d", handle);
    return;
  }

  auto references = references_.find(handle);
  if (references != references_.end()) {
    for (const auto &session : references->second)
      session->SendDeathNotification(handle);
    references_.erase(references);
  }

  handles_.erase(iter);
}

bool Registry::IsHandleKnown(std::uint64_t handle) const {
  return handles_.find(handle) != handles_.end();
}

bool Registry::IsHandleReferenced(std::uint64_t handle) const {
  auto iter = references_.find(handle);
  if (iter == references_.end())
    return false;

  return iter->second.size() > 0;
}

Status Registry::AddReferenceForHandle(const ServerSessionPtr &session, std::uint64_t handle) {
  if (!IsHandleKnown(handle))
    return Status::DeadObject;

  auto reference = references_.find(handle);
  if (reference == references_.end())
    references_.insert({handle, {session}});
  else
    reference->second.push_back(session);

  return Status::OK;
}

Status Registry::RemoveReferenceForHandle(const ServerSessionPtr &session, std::uint64_t handle) {
  if (!IsHandleKnown(handle))
    return Status::DeadObject;

  auto reference = references_.find(handle);
  if (reference == references_.end())
    return Status::BadValue;

  auto iter = std::find(reference->second.begin(), reference->second.end(), session);
  if (iter == reference->second.end())
    return Status::BadValue;

  reference->second.erase(iter);

  return Status::OK;
}
} // namespace binderd
