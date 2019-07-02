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

#include "google/cloud/spanner/sql_partition.h"
#include <gmock/gmock.h>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {

TEST(SqlPartitionTest, Constructor) {
  std::string stmt("select * from foo where name = @name");
  SqlStatement::ParamType params = {{"name", Value("Bob")}};
  std::string partition_token("token");
  std::string session_id("session");
  std::string transaction_id("foo");

  SqlPartition actual_partition(transaction_id, session_id,
                                partition_token, SqlStatement(stmt, params));
  EXPECT_EQ(stmt, actual_partition.sql_statement().sql());
  EXPECT_EQ(params, actual_partition.sql_statement().params());
  EXPECT_EQ(partition_token, actual_partition.partition_token());
  EXPECT_EQ(transaction_id, actual_partition.transaction_id());
  EXPECT_EQ(session_id, actual_partition.session_id());
}

TEST(SqlPartitionTest, SerializeDeserialize) {
  SqlPartition expected_partition(
      "foo", "session", "token",
      SqlStatement("select * from foo where name = @name",
                   {{"name", Value("Bob")}}));
  StatusOr<SqlPartition> actual_partition =
      DeserializeSqlPartition(SerializeSqlPartition(expected_partition));
  ASSERT_TRUE(actual_partition.ok());
  EXPECT_EQ(expected_partition.partition_token(),
            actual_partition->partition_token());
  EXPECT_EQ(expected_partition.transaction_id(),
      actual_partition->transaction_id());
  EXPECT_EQ(expected_partition.session_id(), actual_partition->session_id());
  EXPECT_EQ(expected_partition.sql_statement().sql(),
            actual_partition->sql_statement().sql());
  EXPECT_EQ(expected_partition.sql_statement().params(),
            actual_partition->sql_statement().params());
}

}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google
