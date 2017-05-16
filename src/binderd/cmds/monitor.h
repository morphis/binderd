/*
 * Copyright (C) 2017 Simon Fels <morphis@gravedo.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef BINDER_CMDS_MONITOR_H_
#define BINDER_CMDS_MONITOR_H_

#include "binderd/cli.h"

namespace binderd {
namespace cmds {
class Monitor : public cli::CommandWithFlagsAndAction {
 public:
  Monitor();
};
}  // namespace cmds
}  // namespace binderd

#endif
