/*
 * Copyright (C) 2017 Simon Fels <morphis@gravedo.de>
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

#include <gtest/gtest.h>

#include "binderd/object_translator.h"
#include "binderd/writable_transaction_data.h"
#include "binderd/common/binary_writer.h"

using namespace binderd;

TEST(ObjectTranslator, TransactionWithNoObjectsIsOk) {
  ObjectTranslator translator;

  WritableTransactionData data;
  ASSERT_EQ(0, data.GetNumObjectOffsets());

  ASSERT_EQ(Status::OK, translator.ProcessTransaction(&data, ObjectTranslator::Direction::In));
  ASSERT_EQ(Status::OK, translator.ProcessTransaction(&data, ObjectTranslator::Direction::Out));
}

TEST(ObjectTranslator, FailsToMapUnknownIncomingBinderObject) {
  ObjectTranslator translator;

  BinaryWriter data;
  BinaryWriter objects;

  flat_binder_object obj;
  obj.type = BINDER_TYPE_BINDER;
  obj.cookie = static_cast<uintptr_t>(0x1337);
  obj.binder = 0;
  obj.handle = 0;

  data.WriteData(reinterpret_cast<uint8_t*>(&obj), sizeof(flat_binder_object));
  objects.WriteUint64(0);

  WritableTransactionData request;
  request.SetData(data.Finalize());
  request.SetObjectOffsets(objects.Finalize());

  ASSERT_EQ(Status::BadValue, translator.ProcessTransaction(&request, ObjectTranslator::Direction::In));
}

TEST(ObjectTranslator, MapsBinderObjectInAndOut) {
  ObjectTranslator translator;

  BinaryWriter data;
  BinaryWriter objects;

  flat_binder_object obj;
  obj.type = BINDER_TYPE_BINDER;
  obj.cookie = static_cast<uintptr_t>(0x1337);
  obj.binder = 0;
  obj.handle = 0;

  data.WriteData(reinterpret_cast<uint8_t*>(&obj), sizeof(flat_binder_object));
  objects.WriteUint64(0);

  WritableTransactionData request;
  request.SetData(data.Finalize());
  request.SetObjectOffsets(objects.Finalize());

  ASSERT_EQ(Status::OK, translator.ProcessTransaction(&request, ObjectTranslator::Direction::Out));

  flat_binder_object *translated_obj = reinterpret_cast<flat_binder_object*>(request.GetMutableData());
  ASSERT_EQ(1, translated_obj->cookie);

  ASSERT_EQ(Status::OK, translator.ProcessTransaction(&request, ObjectTranslator::Direction::In));
  ASSERT_EQ(0x1337, translated_obj->cookie);
}
