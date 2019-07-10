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
class SqlPartitionTester {
 public:
  SqlPartitionTester() = default;
  SqlPartitionTester(SqlPartition partition) :
    partition_(std::move(partition)) {}
  SqlStatement const& sql_statement() const {
    return partition_.sql_statement();
  }
  std::string const& partition_token() const {
    return partition_.partition_token();
  }
  std::string const& session_id() const {
    return partition_.session_id();
  }
  std::string const& transaction_id() const {
    return partition_.transaction_id();
  }
  SqlPartition partition() const {
    return partition_;
  }

 private:
  SqlPartition partition_;
};

class SqlPartitionTest : public ::testing::Test {
 protected:
  SqlPartitionTester actual_partition_;
};

TEST_F(SqlPartitionTest, MakeSqlPartition) {
  std::string stmt("select * from foo where name = @name");
  SqlStatement::ParamType params = {{"name", Value("Bob")}};
  std::string partition_token("token");
  std::string session_id("session");
  std::string transaction_id("foo");

  actual_partition_ = internal::MakeSqlPartition(transaction_id,
      session_id, partition_token, SqlStatement(stmt, params));
  EXPECT_EQ(stmt, actual_partition_.sql_statement().sql());
  EXPECT_EQ(params, actual_partition_.sql_statement().params());
  EXPECT_EQ(partition_token, actual_partition_.partition_token());
  EXPECT_EQ(transaction_id, actual_partition_.transaction_id());
  EXPECT_EQ(session_id, actual_partition_.session_id());
}

TEST_F(SqlPartitionTest, Constructor) {
  std::string stmt("select * from foo where name = @name");
  SqlStatement::ParamType params = {{"name", Value("Bob")}};
  std::string partition_token("token");
  std::string session_id("session");
  std::string transaction_id("foo");

  actual_partition_ = internal::MakeSqlPartition(transaction_id,
      session_id, partition_token, SqlStatement(stmt, params));
  EXPECT_EQ(stmt, actual_partition_.sql_statement().sql());
  EXPECT_EQ(params, actual_partition_.sql_statement().params());
  EXPECT_EQ(partition_token, actual_partition_.partition_token());
  EXPECT_EQ(transaction_id, actual_partition_.transaction_id());
  EXPECT_EQ(session_id, actual_partition_.session_id());
}

TEST_F(SqlPartitionTest, SerializeDeserialize) {
  SqlPartitionTester expected_partition(internal::MakeSqlPartition(
      "foo", "session", "token",
      SqlStatement("select * from foo where name = @name",
                   {{"name", Value("Bob")}})));
  StatusOr<SqlPartition> partition =
      DeserializeSqlPartition(SerializeSqlPartition(
          expected_partition.partition()));

  ASSERT_TRUE(partition.ok());
  actual_partition_ = SqlPartitionTester(*partition);
  EXPECT_EQ(expected_partition.partition_token(),
            actual_partition_.partition_token());
  EXPECT_EQ(expected_partition.transaction_id(),
      actual_partition_.transaction_id());
  EXPECT_EQ(expected_partition.session_id(), actual_partition_.session_id());
  EXPECT_EQ(expected_partition.sql_statement().sql(),
            actual_partition_.sql_statement().sql());
  EXPECT_EQ(expected_partition.sql_statement().params(),
            actual_partition_.sql_statement().params());
}

}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google
