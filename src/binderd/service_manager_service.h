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

#ifndef BINDER_SERVICE_MANAGER_SERVICE_H_
#define BINDER_SERVICE_MANAGER_SERVICE_H_

#include "binderd/constants.h"
#include "binderd/client.h"
#include "binderd/object.h"
#include "binderd/service_manager_base.h"

#include <unordered_map>

namespace binderd {
class ServiceManagerService :
    public ServiceManagerBase,
    public LocalObject::TransactionHandler {
 public:
  ServiceManagerService();
  ~ServiceManagerService() override;

  Status AddService(const ClientPtr &client, const std::string &name,
                    const std::shared_ptr<Object> &object, int options) override;

  Status GetService(const ClientPtr &client, const std::string &name,
                    std::shared_ptr<Object> *object) override;

  Status ListServices(const ClientPtr &client, std::vector<std::string> &services) override;

  Status OnTransact(
      const ClientPtr &client,
      std::unique_ptr<TransactionData> request,
      std::unique_ptr<TransactionData> *reply) override;

 private:
  void OnObjectDied(const Object *who);

  std::unordered_map<std::string,std::shared_ptr<Object>> services_;

  struct ObjectDeathHandlerWrapper;
  std::shared_ptr<ObjectDeathHandlerWrapper> death_handler_;
};
} // namespace binderd

#endif
