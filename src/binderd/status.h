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

#ifndef BINDERD_STATUS_H_
#define BINDERD_STATUS_H_

#include <ostream>

#include <errno.h>
#include <stdint.h>

namespace binderd {
enum class Status : int32_t {
  OK = 0,

  UnknownError = INT32_MIN,

  NoMemory = -ENOMEM,
  InvalidOperation = -ENOSYS,
  BadValue = -EINVAL,
  BadType = (UnknownError + 1),
  NameNotFound = -ENOENT,
  PermissionDenied = -EPERM,
  NoInit = -ENODEV,
  AlreadyExists = -EEXIST,
  DeadObject = -EPIPE,
  FailedTransaction = (UnknownError + 2),
  BadIndex = -EOVERFLOW,
  NotEnoughData = -ENODATA,
  WouldBlock = -EWOULDBLOCK,
  TimedOut = -ETIMEDOUT,
  UnknownTransaction = -EBADMSG,
  FdsNotAllowed = (UnknownError + 7),
  UnexpectedNull = (UnknownError + 8),
};
inline std::ostream& operator<<(std::ostream &out, const Status &status) {
  switch (status) {
  case Status::OK:
    out << "ok";
    break;
  case Status::NoMemory:
    out << "no-memory";
    break;
  case Status::InvalidOperation:
    out << "invalid-operation";
    break;
  case Status::BadValue:
    out << "bad-value";
    break;
  case Status::BadType:
    out << "bad-type";
    break;
  case Status::NameNotFound:
    out << "name-not-found";
    break;
  case Status::PermissionDenied:
    out << "permission-denied";
    break;
  case Status::NoInit:
    out << "no-init";
    break;
  case Status::AlreadyExists:
    out << "already-exists";
    break;
  case Status::DeadObject:
    out << "dead-object";
    break;
  case Status::FailedTransaction:
    out << "failed-transaction";
    break;
  case Status::BadIndex:
    out << "bad-index";
    break;
  case Status::NotEnoughData:
    out << "not-enough-data";
    break;
  case Status::WouldBlock:
    out << "would-block";
    break;
  case Status::TimedOut:
    out << "timed-out";
    break;
  case Status::UnknownTransaction:
    out << "unknown-transaction";
    break;
  case Status::FdsNotAllowed:
    out << "fds-not-allowed";
    break;
  case Status::UnexpectedNull:
    out << "unexpected-null";
    break;
  default:
    out << "unknown-error";
    break;
  }
  return out;
}
} // namespace binderd

#endif
