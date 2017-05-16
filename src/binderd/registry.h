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

#ifndef BINDER_REGISTRY_H_
#define BINDER_REGISTRY_H_

#include "binderd/server_session.h"

#include <memory>
#include <atomic>
#include <unordered_map>

namespace binderd {
class Registry {
 public:
  static std::shared_ptr<Registry> Create();

  ~Registry();

  void SetContextManager(const ServerSessionPtr &session);
  ServerSessionPtr GetContextManager() const;

  void SetMonitor(const ServerSessionPtr &session);
  ServerSessionPtr GetMonitor() const;

  ServerSessionPtr FindSessionByHandle(std::uint64_t handle);

  std::uint64_t NewObjectHandle(const ServerSessionPtr &session);

  void RemoveHandle(std::uint64_t handle);

  bool IsHandleKnown(std::uint64_t handle) const;
  bool IsHandleReferenced(std::uint64_t handle) const;

  Status AddReferenceForHandle(const ServerSessionPtr &session, std::uint64_t handle);
  Status RemoveReferenceForHandle(const ServerSessionPtr &session, std::uint64_t handle);

 private:
  Registry();

  ServerSessionPtr context_mgr_;
  ServerSessionPtr monitor_;
  std::atomic<std::uint64_t> next_object_handle_;
  std::unordered_map<std::uint64_t,ServerSessionPtr> handles_;
  std::unordered_map<std::uint64_t,std::vector<ServerSessionPtr>> references_;
};
using RegistryPtr = std::shared_ptr<Registry>;
} // namespace binderd

#endif
