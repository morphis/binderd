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

#ifndef BINDERD_CLIENT_H_
#define BINDERD_CLIENT_H_

#include "binderd/buffer.h"
#include "binderd/message.h"
#include "binderd/constants.h"
#include "binderd/transaction_data.h"
#include "binderd/local_object.h"
#include "binderd/object_pool.h"

#include <memory>
#include <chrono>
#include <functional>

namespace binderd {
class Client {
 public:
  enum class State {
    Disconnected,
    Connected,
  };

  class LogMessageHandler {
   public:
    virtual ~LogMessageHandler() {}
    virtual void OnLogMessage(const MessagePtr &msg) = 0;
  };

  // Needed for unit tests where we use ProcessState/IPCThreadState
  // to talk with the server and have a client instance we can't
  // influence otherwise. It will set a global path variable which
  // will take priority over one supplied with the c'tor of the
  // Client class.
  static void SetSocketPathOverride(const std::string &socket_path);

  static std::shared_ptr<Client> Create(const std::string &socket_path = kDefaultSocketPath,
                                        bool connect = true);

  static Client* CreateNotTracked(const std::string &socket_path = kDefaultSocketPath);

  virtual ~Client() {}

  virtual State GetState() const = 0;

  virtual void QueueMessage(const MessagePtr &msg) = 0;
  virtual MessagePtr DequeueMessage() = 0;

  virtual MessagePtr Transact(const MessagePtr &msg, bool one_way = false) = 0;
  virtual Status Transact(int32_t handle,
                          std::unique_ptr<TransactionData> request,
                          std::unique_ptr<TransactionData> *reply) = 0;

  virtual uint64_t NewCookie() = 0;

  virtual Status AddReference(uint64_t handle) = 0;
  virtual Status ReleaseReference(uint64_t handle) = 0;
  using ReleaseReferenceFunc = std::function<Status()>;
  virtual ReleaseReferenceFunc GetReleaseReferenceClosure(uint64_t handle) = 0;

  virtual Status BecomeContextManager() = 0;
  virtual void SetContextObject(const std::shared_ptr<Object> &object) = 0;

  virtual Status RequestDeathNotification(uint64_t handle, const std::shared_ptr<Object> &proxy) = 0;
  virtual Status ClearDeathNotification(uint64_t handle, const std::shared_ptr<Object> &proxy) = 0;

  virtual Status BecomeMonitor() = 0;
  virtual void SetLogMessageHandler(const std::shared_ptr<LogMessageHandler> &handler) = 0;

  virtual void StartThreadPool() = 0;
  virtual void JoinThreadPool() = 0;
  virtual bool ProcessAndExecuteCommand() = 0;
  virtual ObjectPoolPtr GetObjectPool() = 0;
};
using ClientPtr = std::shared_ptr<Client>;
} // namespace binderd

#endif
