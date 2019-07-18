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

#ifndef GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_READ_PARTITION_H_
#define GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_READ_PARTITION_H_

#include "google/cloud/spanner/keys.h"
#include "google/cloud/spanner/version.h"
#include "google/cloud/status_or.h"
#include <string>
#include <vector>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {

class ReadPartition;

/**
 * Serializes an instance of `ReadPartition` for transmission to another
 * process.
 *
 * @param read_partition - instance to be serialized.
 *
 * @par Example
 *
 * @code
 * spanner::SqlStatement stmt("select * from Albums");
 * std::vector<spanner::SqlPartition> partitions =
 *   spanner_client.PartitionSql(stmt);
 * for (auto const& partition : partitions) {
 *   auto serialized_partition = spanner::SerializeSqlPartition(partition);
 *   if (serialized_partition.ok()) {
 *     SendToRemoteMachine(*serialized_partition);
 *   }
 * }
 * @endcode
 */
StatusOr<std::string> SerializeReadPartition(
    ReadPartition const& read_partition);

/**
 * Deserializes the provided string into a `ReadPartition`, if able.
 *
 * Returned `Status` should be checked to determine if deserialization was
 * successful.
 *
 * @param serialized_read_partition
 *
 * @par Example
 *
 * @code
 * std::string serialized_partition = ReceiveFromRemoteMachine();
 * spanner::SqlPartition partition =
 *   spanner::DeserializeSqlPartition(serialized_partition);
 * auto rows = spanner_client.ExecuteSql(partition);
 * @endcode
 */
StatusOr<SqlPartition> DeserializeReadPartition(
    std::string const& serialized_read_partition);

// Internal implementation details that callers should not use.
namespace internal {

ReadPartition MakeReadPartition(std::string const& transaction_id,
                                std::string const& session_id,
                                std::string const& partition_token);
}  // namespace internal

/**
 * The `ReadPartition` class is a semi-regular type that represents a single
 * slice of a parallel Read operation.
 *
 * Instances of `ReadPartition` are created by `Client::PartitionRead`. Once
 * created, `ReadPartition` objects can be serialized, transmitted to separate
 * process, and used to read data in parallel using `Client::Read`.
 */
class ReadPartition {
 public:
  /**
   * Constructs an instance of `ReadPartition` that does not specify any table
   * or columns to be read.
   */
  ReadPartition() = default;

  // Copy and move.
  ReadPartition(ReadPartition const&) = default;
  ReadPartition(ReadPartition&&) = default;
  ReadPartition& operator=(ReadPartition const&) = default;
  ReadPartition& operator=(ReadPartition&&) = default;

 private:
  friend class ReadPartitionTester;
  friend ReadPartition internal::MakeReadPartition(
      std::string const& transaction_id, std::string const& session_id,
      std::string const& partition_token);
  friend StatusOr<std::string> SerializeReadPartition(
      ReadPartition const& read_partition);
  friend StatusOr<ReadPartition> DeserializeReadPartition(
      std::string const& serialized_read_partition);

  explicit ReadPartition(std::string transaction_id, std::string session_id,
                         std::string partition_token, std::string table,
                         std::vector<std::string> columns_read, KeySet key_set);

  // Accessor methods for use by friends.
  std::string const& partition_token() const { return partition_token_; }
  std::string const& session_id() const { return session_id_; }
  std::string const& transaction_id() const { return transaction_id_; }

  std::string transaction_id_;
  std::string session_id_;
  std::string partition_token_;
};

}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_READ_PARTITION_H_
