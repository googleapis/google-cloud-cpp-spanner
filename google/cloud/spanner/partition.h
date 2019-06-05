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

#ifndef GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_PARTITION_H_
#define GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_PARTITION_H_

#include "google/cloud/spanner/key.h"
#include "google/cloud/spanner/sql_statement.h"
#include "google/cloud/spanner/transaction.h"
#include "google/cloud/spanner/version.h"
#include "google/cloud/status_or.h"
#include <cstdint>
#include <string>
#include <vector>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {
/**
 * The options passed to Client::PartitionRead() and Client::PartitionQuery().
 */
struct PartitionOptions {
  int64_t partition_size_bytes;  // currently ignored by spanner
  int64_t max_partitions;        // currently ignored by spanner
};

/**
 * Spanner can partition read requests so that several (or many) different
 * callers can each read a part of the response. This is done by first asking
 * Spanner to partition the read request. Spanner will return a list of
 * partition tokens, which can be sent along with the original read request to
 * the StreamingRead RPC. It's important to note that the partition token must
 * be sent along with the exact original read request, and the original
 * transaction. Otherwise, the read will fail.
 *
 * The ReadPartition class encapsulates a partition token along with all the
 * required state that must go along with it (transaction, read request, etc.).
 * This class can be serialized to an opaque string, sent to another machine,
 * deserialized, then passed to spanner::Client::Read, which will perform the
 * partitioned read appropriately.
 */
class ReadPartition {
 public:
  // Value type. op==, op!=

 private:
  friend std::string SerializeReadPartition(ReadPartition p);
  friend StatusOr<ReadPartition> DeserializeReadPartition(std::string s);

  std::string token_;
  Transaction tx_;
  std::string table_;
  KeySet keys_;
  std::vector<std::string> columns_;
};

std::string SerializeReadPartition(ReadPartition p) { return {}; }
StatusOr<ReadPartition> DeserializeReadPartition(std::string s) {
  return Status(StatusCode::kUnimplemented, "unimplemented");
}

/*
 * (See the spanner::ReadPartition section above for background on partitioning
 * requests.)
 *
 * The spanner::SqlPartition class encapsulates a partition token, a
 * transaction, and a spanner::SqlStatment. This class is serializable to an
 * opaque string, which can be deserialized (even on another machine) and
 * executed using spanner::Client::ExecuteSql.
 */

class SqlPartition {
 public:
  // Value type. op==, op!=

 private:
  friend std::string SerializeSqlPartition(SqlPartition p);
  friend StatusOr<SqlPartition> DeserializeSqlPartition(std::string s);

  std::string token_;
  Transaction tx_;
  SqlStatement statement_;
};

std::string SerializeSqlPartition(SqlPartition p) { return {}; }
StatusOr<SqlPartition> DeserializeSqlPartition(std::string s) {
  return Status(StatusCode::kUnimplemented, "unimplemented");
}
}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_PARTITION_H_
