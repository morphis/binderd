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

#ifndef BINDERD_CLIENT_IMPL_H_
#define BINDERD_CLIENT_IMPL_H_

#include "binderd/client.h"
#include "binderd/socket_messenger.h"
#include "binderd/object_translator.h"

#include <unordered_map>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <deque>
#include <future>

#include <asio.hpp>

namespace binderd {
class ClientImpl : public Client,
                   public std::enable_shared_from_this<ClientImpl> {
 public:
  using CallPromisePtr = std::shared_ptr<std::promise<MessagePtr>>;


  ClientImpl(const std::shared_ptr<asio::io_service> &io,
             const std::shared_ptr<Messenger> &messenger,
             const std::string &socket_path,
             bool connect = true,
             std::size_t num_workers = 2);

  ~ClientImpl() override;

  State GetState() const override;

  void QueueMessage(const MessagePtr &msg) override;
  MessagePtr DequeueMessage() override;

  MessagePtr Transact(const MessagePtr &msg, bool one_way = false) override;
  Status Transact(int32_t handle,
                  std::unique_ptr<TransactionData> request,
                  std::unique_ptr<TransactionData> *reply) override;

  uint64_t NewCookie() override;

  Status AddReference(uint64_t handle) override;
  Status ReleaseReference(uint64_t handle) override;
  ReleaseReferenceFunc GetReleaseReferenceClosure(uint64_t handle) override;

  Status BecomeContextManager() override;
  void SetContextObject(const std::shared_ptr<Object> &object) override;

  Status RequestDeathNotification(uint64_t handle, const std::shared_ptr<Object> &proxy) override;
  Status ClearDeathNotification(uint64_t handle, const std::shared_ptr<Object> &proxy) override;

  Status BecomeMonitor() override;
  void SetLogMessageHandler(const std::shared_ptr<LogMessageHandler> &handler) override;

  void StartThreadPool() override;
  void JoinThreadPool() override;

  bool ProcessAndExecuteCommand() override;

  ObjectPoolPtr GetObjectPool() override;

 private:
  CallPromisePtr StartCall(const MessagePtr &msg, bool one_way);
  void FinishCall(std::uint64_t cookie);
  void SendStatus(const MessagePtr &msg, const Status &status);
  void HandleContextObjectMessage(const MessagePtr &msg);
  void OnNewMessage(const asio::error_code &err, const BufferPtr &buffer);
  void ReadNextMessage();
  void Terminate();
  void Connect(const std::string &socket_path);

  struct PendingCall {
    CallPromisePtr promise;
    Message::Type msg_type;
    bool one_way = false;
  };

  std::atomic<Client::State> state_;
  std::shared_ptr<asio::io_service> io_;
  asio::strand strand_;
  std::thread io_thread_;
  std::vector<std::thread> io_workers_;
  std::shared_ptr<Messenger> messenger_;
  std::mutex read_lock_;
  std::condition_variable can_read_;
  std::deque<MessagePtr> in_messages_;
  std::atomic_bool running_{false};
  std::atomic_bool should_exit_{false};
  std::unordered_map<std::uint64_t,PendingCall> pending_calls_;
  std::atomic<std::uint64_t> next_cookie_;
  std::shared_ptr<Object> context_obj_;
  std::shared_ptr<Client::LogMessageHandler> log_message_handler_;
  ObjectPoolPtr object_pool_;
  ObjectTranslatorPtr object_translator_;
  std::thread processor_thread_;
  std::vector<std::thread> workers_;

  struct DeadRecipient {
    int id;
    std::shared_ptr<Object> proxy;
    std::vector<uint64_t> handles;
  };

  std::vector<DeadRecipient> dead_recipients_;
};
using ClientPtr = std::shared_ptr<Client>;
} // namespace binderd

#endif
