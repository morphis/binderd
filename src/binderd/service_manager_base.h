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

#ifndef BINDER_SERVICE_MANAGER_BASE_H_
#define BINDER_SERVICE_MANAGER_BASE_H_

#include "binderd/constants.h"
#include "binderd/client.h"
#include "binderd/object.h"

#include <vector>
#include <string>

namespace binderd {
class ServiceManagerBase {
 public:
  static const char kInterfaceName[];

  virtual ~ServiceManagerBase() {}

  enum {
    GetServiceTransaction = kFirstTransactionCode,
    CheckServiceTransaction,
    AddServiceTransaction,
    ListServicesTransaction,
  };

  enum AddServiceOptions {
    // Allow isolated Android processes to access the service
    AllowIsolated = 1,
  };

  // Add a new service with the specified name to the service manager
  virtual Status AddService(const ClientPtr &client, const std::string &name,
                            const std::shared_ptr<Object> &object, int options) = 0;

  // Return service object with the given name
  virtual Status GetService(const ClientPtr &client, const std::string &name,
                            std::shared_ptr<Object> *object) = 0;

  // List all available services registered to the service manager
  virtual Status ListServices(const ClientPtr &client,
                              std::vector<std::string> &services) = 0;
};
} // namespace binderd

#endif
