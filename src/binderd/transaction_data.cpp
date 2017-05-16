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

#include "binderd/transaction_data.h"
#include "binderd/common/binary_writer.h"
#include "binderd/common/utils.h"

#include <asm/types.h>
#include <linux/binder.h>

namespace binderd {
BufferPtr TransactionData::Pack() const {
  BinaryWriter writer;
  writer.SetPadding(false);

  writer.WriteUint64(GetBinder());
  writer.WriteUint64(GetCookie());
  writer.WriteUint32(GetCode());

  uint32_t flags = static_cast<uint32_t>(Flags::AcceptFds);
  if (IsOneWay())
    flags |= static_cast<uint32_t>(Flags::OneWay);
  if (HasStatus())
    flags |= static_cast<uint32_t>(Flags::HasStatus);
  writer.WriteUint32(flags);

  writer.WriteSizedData(GetData(),
                        GetDataSize());
  writer.WriteSizedData(GetObjectOffsets(),
                        GetNumObjectOffsets() * sizeof(binder_size_t));

  return writer.Finalize();
}

std::ostream& operator<<(std::ostream &out, const TransactionData &data) {
  out << utils::string_format("Binder=%d Cookie=%d Code=%d Flags={ IsOneWay=%d HasStatus=%d }",
                              data.GetBinder(), data.GetCookie(), data.GetCode(),
                              data.IsOneWay(), data.HasStatus()) << std::endl;
  out << "DATA:" << std::endl << utils::hex_dump(data.GetData(), data.GetDataSize()) << std::endl;
  out << "OBJECTS:" << std::endl
      << utils::hex_dump(data.GetObjectOffsets(), data.GetNumObjectOffsets() * sizeof(binder_size_t))
      << std::endl;

  return out;
}
} // namespace binderd
