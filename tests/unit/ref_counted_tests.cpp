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

#include "binderd/common/ref_counted.h"

namespace {
class TestObject : public binderd::RefCounted<TestObject> {
 public:
  TestObject() = default;

 protected:
  friend class binderd::RefCounted<TestObject>;
  virtual ~TestObject() {}

  DISALLOW_COPY_AND_ASSIGN(TestObject);
};
}

TEST(RefCounted, CountStartsWithZero) {
  auto obj = new TestObject;
  ASSERT_EQ(obj->GetCount(), 0);
  obj->Release();
}
