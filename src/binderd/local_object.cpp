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

#include "binderd/local_object.h"
#include "binderd/constants.h"
#include "binderd/logger.h"

namespace binderd {
LocalObject::LocalObject(std::unique_ptr<TransactionHandler> handler) :
  handler_{std::move(handler)} {}

LocalObject::~LocalObject() {}

Object::Type LocalObject::GetType() const {
  return Type::Local;
}

Status LocalObject::Transact(const ClientPtr &client,
                             std::unique_ptr<TransactionData> request,
                             std::unique_ptr<TransactionData> *reply) {
  switch (request->GetCode()) {
  case kInterfaceTransactionCode:
  case kDumpTransactionCode:
  case kShellCommandTransactionCode:
  case kSysPropsTransactionCode:
    WARNING("Base binder object transactions are not yet implemented");
    return Status::UnknownTransaction;
  default:
    break;
  }

  return handler_->OnTransact(client, std::move(request), reply);
}

Status LocalObject::LinkToDeath(const ClientPtr &client, const std::shared_ptr<DeathRecipient> &recipient) {
  (void) client;
  (void) recipient;
  return Status::InvalidOperation;
}

Status LocalObject::UnlinkToDeath(const ClientPtr &client, const std::shared_ptr<DeathRecipient> &recipient) {
  (void) client;
  (void) recipient;
  return Status::InvalidOperation;
}
} // namespace binderd
