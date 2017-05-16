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

#include "binderd/service_manager_service.h"
#include "binderd/parcel_transaction_data_reader.h"
#include "binderd/parcel_transaction_data_writer.h"
#include "binderd/logger.h"

#include <functional>

using namespace std::placeholders;

namespace binderd {
// As ServiceManagerService is constructed as a std::unique_ptr we can't
// pass it to an object via Object::LinkToDeath. Therefor we need a simple
// wrapper object which gets a function which translates all incoming calls.
struct ServiceManagerService::ObjectDeathHandlerWrapper : public Object::DeathRecipient {
  ObjectDeathHandlerWrapper(const std::function<void(const Object*)> &handler) : handler_{handler} {}

  void OnObjectDied(const Object *who) override {
    if (!handler_)
      return;

    handler_(who);
  }

  std::function<void(const Object*)> handler_;
};

ServiceManagerService::ServiceManagerService() :
  death_handler_{new ObjectDeathHandlerWrapper(std::bind(&ServiceManagerService::OnObjectDied, this, _1))} {}

ServiceManagerService::~ServiceManagerService() {}

Status ServiceManagerService::AddService(const ClientPtr &client, const std::string &name,
                                         const std::shared_ptr<Object> &object, int options) {
  (void) client;
  (void) options;

  if (!object || services_.find(name) != services_.end())
    return Status::BadValue;

  services_.insert({name, object});

  object->LinkToDeath(client, death_handler_);

  return Status::OK;
}

Status ServiceManagerService::GetService(const ClientPtr &client, const std::string &name,
                                         std::shared_ptr<Object> *object) {
  (void) client;

  auto iter = services_.find(name);
  if (iter == services_.end())
    return Status::BadValue;

  *object = iter->second;

  return Status::OK;
}

Status ServiceManagerService::ListServices(const ClientPtr &client, std::vector<std::string> &services) {
  (void) client;

  for (auto iter = services_.begin(); iter != services_.end(); iter++)
    services.push_back(iter->first);

  return Status::OK;
}

Status ServiceManagerService::OnTransact(
    const ClientPtr &client,
    std::unique_ptr<TransactionData> request,
    std::unique_ptr<TransactionData> *reply) {

  switch (request->GetCode()) {
  case ServiceManagerBase::AddServiceTransaction: {
    ParcelTransactionDataReader reader(request);
    reader.EnforceInterface(kInterfaceName);
    const auto name = utils::to_string8(reader.ReadString16());
    auto obj = reader.ReadObject(client);
    const auto allow_isolated = reader.ReadInt32();

    const auto status = AddService(client, name, obj, 0);
    if (status != Status::OK)
      return status;

    ParcelTransactionDataWriter writer;
    writer.WriteInt32(static_cast<int32_t>(Status::OK));
    *reply = std::move(writer.Finalize());

    return Status::OK;
  }
  case ServiceManagerBase::CheckServiceTransaction:
  case ServiceManagerBase::GetServiceTransaction: {
    ParcelTransactionDataReader reader(request);
    reader.EnforceInterface(kInterfaceName);
    const auto name = utils::to_string8(reader.ReadString16());
    std::shared_ptr<Object> obj;
    const auto status = GetService(client, name, &obj);
    if (status != Status::OK)
      return status;

    if (!obj)
      return Status::DeadObject;

    ParcelTransactionDataWriter writer;
    writer.WriteObject(client, obj);

    *reply = std::move(writer.Finalize());

    return Status::OK;
  }
  case ServiceManagerBase::ListServicesTransaction: {
    std::vector<std::string> services;
    const auto status = ListServices(client, services);
    if (status != Status::OK)
      return status;

    ParcelTransactionDataReader reader(request);
    reader.EnforceInterface(kInterfaceName);

    const auto n = reader.ReadInt32();
    if (n < 0 || n >= services.size()) {
      // Status::BadIndex would be a better fit here but the Android code in
      // Parcel.cpp uses Status::BadValue so we do the same here
      return Status::BadValue;
    }

    ParcelTransactionDataWriter writer;
    writer.WriteString16(services[n]);

    *reply = std::move(writer.Finalize());
    return Status::OK;
  }
  default:
    break;
  }

  return Status::UnknownTransaction;
}

void ServiceManagerService::OnObjectDied(const Object *who) {
  for (auto iter = services_.begin(); iter != services_.end(); iter++) {
    if (iter->second.get() == who) {
      services_.erase(iter);
      break;
    }
  }
}
} // namespace binderd
