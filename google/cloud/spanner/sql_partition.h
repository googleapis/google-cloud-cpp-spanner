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

#ifndef GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_SQL_PARTITION_H_
#define GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_SQL_PARTITION_H_

#include "google/cloud/spanner/sql_statement.h"
#include "google/cloud/spanner/transaction.h"
#include <google/cloud/status_or.h>
#include <gtest/gtest_prod.h>
#include <memory>
#include <string>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {
class Client;
class SqlPartition {
 public:
  SqlPartition() = default;
  SqlPartition(SqlPartition const&) = default;
  SqlPartition(SqlPartition&&) = default;
  SqlPartition& operator=(SqlPartition const&) = default;
  SqlPartition& operator=(SqlPartition&&) = default;

  SqlStatement const& sql_statement() const;

 private:
  friend std::string SerializeSqlPartition(SqlPartition const& sql_partition);
  friend google::cloud::StatusOr<SqlPartition> DeserializeSqlPartition(
      std::string const& serialized_sql_partition);
  friend Client;

  explicit SqlPartition(std::string transaction_id, std::string session_id,
      std::string partition_token, SqlStatement sql_statement);
  std::string const& partition_token() const;
  std::string const& session_id() const;
  std::string const& transaction_id() const;

  std::string transaction_id_;
  std::string session_id_;
  std::string partition_token_;
  SqlStatement sql_statement_;

  FRIEND_TEST(SqlPartitionTest, Constructor);
  FRIEND_TEST(SqlPartitionTest, SerializeDeserialize);
};

std::string SerializeSqlPartition(SqlPartition const& sql_partition);
google::cloud::StatusOr<SqlPartition> DeserializeSqlPartition(
    std::string const& serialized_sql_partition);

}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_SQL_PARTITION_H_
