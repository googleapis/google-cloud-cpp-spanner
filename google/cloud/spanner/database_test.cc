// Copyright 2019 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "google/cloud/spanner/database.h"
#include "google/cloud/spanner/testing/matchers.h"
#include <gmock/gmock.h>
#include <sstream>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {
namespace {

TEST(Database, Basics) {
  Database db("p1", "i1", "d1");
  EXPECT_EQ("p1", db.project_id());
  EXPECT_EQ("i1", db.instance_id());
  EXPECT_EQ("d1", db.database_id());
  EXPECT_EQ("projects/p1/instances/i1/databases/d1", db.FullName());

  auto copy = db;
  EXPECT_EQ(copy, db);
  EXPECT_EQ("projects/p1/instances/i1/databases/d1", copy.FullName());

  auto moved = std::move(copy);
  EXPECT_EQ(moved, db);
  EXPECT_EQ("projects/p1/instances/i1/databases/d1", moved.FullName());

  Database db2("p2", "i2", "d2");
  EXPECT_NE(db2, db);
  EXPECT_EQ("projects/p2/instances/i2/databases/d2", db2.FullName());
}

TEST(Database, OutputStream) {
  Database db("p1", "i1", "d1");
  std::ostringstream os;
  os << db;
  EXPECT_EQ("projects/p1/instances/i1/databases/d1", os.str());
}

}  // namespace
}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google
