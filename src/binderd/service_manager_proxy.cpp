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

#include "binderd/service_manager_proxy.h"
#include "binderd/parcel_transaction_data_writer.h"
#include "binderd/parcel_transaction_data_reader.h"
#include "binderd/common/utils.h"
#include "binderd/logger.h"

namespace binderd {
ServiceManagerProxy::ServiceManagerProxy() {}

ServiceManagerProxy::~ServiceManagerProxy() {}

Status ServiceManagerProxy::AddService(const ClientPtr &client, const std::string &name,
                                       const std::shared_ptr<Object> &object, int options) {
  if (!client)
    return Status::BadValue;

  ParcelTransactionDataWriter writer;
  writer.SetCode(AddServiceTransaction);
  const int kStrictModePolicy = 0;
  writer.WriteInterfaceToken(kInterfaceName, kStrictModePolicy);
  writer.WriteString16(utils::to_string16(name));
  writer.WriteObject(client, object);
  writer.WriteInt32((options & AllowIsolated) ?  1 : 0);

  std::unique_ptr<TransactionData> reply;
  const auto status = client->Transact(kContextManagerHandle, writer.Finalize(), &reply);
  if (status != Status::OK)
    return status;

  return Status::OK;
}

Status ServiceManagerProxy::GetService(const ClientPtr &client, const std::string &name,
                                       std::shared_ptr<Object> *object) {
  if (!client)
    return Status::BadValue;

  ParcelTransactionDataWriter writer;
  writer.SetCode(GetServiceTransaction);
  const int kStrictModePolicy = 0;
  writer.WriteInterfaceToken(kInterfaceName, kStrictModePolicy);
  writer.WriteString16(utils::to_string16(name));

  std::unique_ptr<TransactionData> reply;
  const auto status = client->Transact(kContextManagerHandle, writer.Finalize(), &reply);
  if (status != Status::OK)
    return status;

  if (!reply)
    return Status::BadValue;

  ParcelTransactionDataReader reader(reply);
  *object = reader.ReadObject(client);

  return Status::OK;
}

Status ServiceManagerProxy::ListServices(const ClientPtr &client, std::vector<std::string> &services) {
  if (!client)
    return Status::BadValue;

  services.clear();

  auto status = Status::OK;
  int32_t n = 0;
  do {
    ParcelTransactionDataWriter request;
    request.SetCode(ListServicesTransaction);
    const int kStrictModePolicy = 0;
    request.WriteInterfaceToken(kInterfaceName, kStrictModePolicy);
    request.WriteInt32(n++);

    std::unique_ptr<TransactionData> reply;
    status = client->Transact(kContextManagerHandle, request.Finalize(), &reply);
    if (reply) {
      ParcelTransactionDataReader reader(reply);
      services.push_back(utils::to_string8(reader.ReadString16()));
    }
  } while (status == Status::OK);

  // Last status we get back will be an error for a bad index
  // we tried to access, but that is ok as this is how the
  // service is designed to work.
  if (status != Status::BadValue)
    return status;

  return Status::OK;
}
} // namespace binderd
