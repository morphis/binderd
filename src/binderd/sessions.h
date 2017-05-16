/*
 * Copyright © 2012-2014 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Alan Griffiths <alan@octopull.co.uk>
 */

#ifndef BINDER_SESSIONS_H_
#define BINDER_SESSIONS_H_

#include <unordered_map>
#include <memory>
#include <mutex>

#include "binderd/logger.h"

namespace binderd {
template <class Session>
class Sessions {
 public:
  Sessions() {}
  ~Sessions() { Clear(); }

  void Add(std::shared_ptr<Session> const& session) {
    std::unique_lock<std::mutex> lock(mutex);
    sessions.insert({session->GetId(), session});
  }

  void Remove(unsigned int id) {
    std::unique_lock<std::mutex> lock(mutex);
    sessions.erase(id);
  }

  void Clear() {
    std::unique_lock<std::mutex> lock(mutex);
    sessions.clear();
  }

  size_t GetSize() { return sessions.size(); }

 private:
  Sessions(Sessions const&) = delete;
  Sessions& operator=(Sessions const&) = delete;

  std::mutex mutex;
  std::unordered_map<unsigned int, std::shared_ptr<Session>> sessions;
};
}  // namespace binderd

#endif
