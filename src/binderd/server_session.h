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

#ifndef BINDER_SERVER_SESSION_H_
#define BINDER_SERVER_SESSION_H_

#include "binderd/socket_messenger.h"
#include "binderd/buffer.h"
#include "binderd/message.h"
#include "binderd/sessions.h"
#include "binderd/transaction_data_from_message.h"

#include <memory>
#include <unordered_map>

#include <asio.hpp>

namespace binderd {
class ServerSession;
using ServerSessions = Sessions<ServerSession>;

class Registry;
class Server;
class ServerSession : public std::enable_shared_from_this<ServerSession> {
 public:
  static std::shared_ptr<ServerSession> Create(
    const std::shared_ptr<ServerSessions> &sessions,
    const std::shared_ptr<asio::io_service> &io,
    const std::shared_ptr<Messenger> &messenger,
    const std::shared_ptr<Registry> &registry);
  ~ServerSession();

  void Start();

  void SendDeathNotification(std::uint64_t handle);
  void SendMessage(const MessagePtr &msg);

  unsigned int GetId() const;

 private:
  ServerSession(
      const std::shared_ptr<ServerSessions> &sessions,
      const std::shared_ptr<asio::io_service> &io,
      const std::shared_ptr<Messenger> &messenger,
      const std::shared_ptr<Registry> &registry);

  // A transaction record describes a currently pending transaction
  // which was delivered to the callee but waits for an answer which
  // can be send back to the caller.
  struct TransactionRecord {
    std::shared_ptr<ServerSession> caller;
    std::uint64_t reply_cookie;
    MessagePtr message;
  };

  // Deliever a transaction to the session. This will return
  // Status::OK only when the transaction was successfully added
  // to the write queue. In all other cases a proper error status
  // is returned and the transaction discarded.
  Status PostTransaction(const TransactionRecord &tr);

  void ReadNextMessage();
  void ProcessMessage(const MessagePtr &msg);
  void Terminate();

  void OnSetContextManager(const MessagePtr &msg);
  void OnSetMonitor(const MessagePtr &msg);
  void OnTransaction(const MessagePtr &msg);
  void OnReply(const MessagePtr &msg);
  void OnReference(const MessagePtr &msg);
  void OnDeathNotification(const MessagePtr &msg);

  void ForwardToMonitor(const MessagePtr &msg);

  Status TranslateObjects(TransactionData &tr);
  void SendStatus(const MessagePtr &msg, const Status &status);

  void WriteMessage(const MessagePtr &msg);

  struct BinderObject {
    uintptr_t binder;
    uintptr_t cookie;
  };

  unsigned int id_;
  std::shared_ptr<ServerSessions> sessions_;
  std::shared_ptr<Messenger> messenger_;
  asio::strand read_strand_;
  std::shared_ptr<Registry> registry_;
  std::deque<MessagePtr> unread_messages_;
  std::atomic<std::uint64_t> next_transaction_cookie_;
  std::unordered_map<std::uint64_t,TransactionRecord> pending_transactions_;
  std::atomic<std::uint64_t> next_node_handle_;
  std::unordered_map<std::uint64_t,BinderObject> objects_;
  std::unordered_map<std::uint64_t,std::uint64_t> death_notifications_;
};
using ServerSessionPtr = std::shared_ptr<ServerSession>;
} // namespace binderd

#endif
