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

#ifndef BINDER_CONSTANTS_H_
#define BINDER_CONSTANTS_H_

#include <stdint.h>

#define BINDER_PACK_CHARS(c1, c2, c3, c4) \
  (((c1) << 24) | ((c2) << 16) | ((c3) << 8) | (c4))

namespace binderd {
// Default path we're using for the server socket
constexpr const char* kDefaultSocketPath{"/run/binder.sock"};

// The context manager always as 0 as its handle
const uint32_t kContextManagerHandle{0};

// Necessary transaction code constants
constexpr const uint32_t kFirstTransactionCode{0x00000001};
constexpr const uint32_t kLastTransactionCode{0x00ffffff};
constexpr const uint32_t kPingTransactionCode{BINDER_PACK_CHARS('_', 'P', 'N', 'G')};
constexpr const uint32_t kDumpTransactionCode{BINDER_PACK_CHARS('_', 'D', 'M', 'P')};
constexpr const uint32_t kShellCommandTransactionCode{BINDER_PACK_CHARS('_', 'C', 'M', 'D')};
constexpr const uint32_t kInterfaceTransactionCode{BINDER_PACK_CHARS('_', 'N', 'T', 'F')};
constexpr const uint32_t kSysPropsTransactionCode{BINDER_PACK_CHARS('_', 'S', 'P', 'R')};

constexpr const int32_t kStrictModePenaltyGather{0x40 << 16};
} // namespace binderd

#endif
