#ifndef GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_PARTITION_H_
#define GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_PARTITION_H_

#include "google/cloud/spanner/key.h"
#include "google/cloud/spanner/sql_statement.h"
#include "google/cloud/spanner/transaction.h"
#include "google/cloud/spanner/value.h"
#include "google/cloud/spanner/version.h"
#include "google/cloud/status_or.h"
#include <cstdint>
#include <string>
#include <vector>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {

// The options passed to PartitionRead() and PartitionQuery().
struct PartitionOptions {
  std::int64_t partition_size_bytes;  // currently ignored
  std::int64_t max_partitions;        // currently ignored
};

class ReadPartition {
 public:
  // Value type.
 private:
  friend std::string SerializeReadPartition(ReadPartition p);
  friend google::cloud::StatusOr<ReadPartition> DeserializeReadPartition(
      std::string s);
  std::string token_;
  Transaction tx_;
  std::string table_;
  KeySet keys_;
  std::vector<std::string> columns_;
};

std::string SerializeReadPartition(ReadPartition p) { return p.token_; }
google::cloud::StatusOr<ReadPartition> DeserializeReadPartition(std::string s) {
  return {};
}

class SqlPartition {
 public:
  // Value type.
 private:
  friend std::string SerializeSqlPartition(SqlPartition p);
  friend google::cloud::StatusOr<SqlPartition> DeserializeSqlPartition(
      std::string s);
  std::string token_;
  Transaction tx_;
  SqlStatement statement_;
};

std::string SerializeSqlPartition(SqlPartition p) { return p.token_; }
google::cloud::StatusOr<SqlPartition> DeserializeSqlPartition(std::string s) {
  return {};
}

}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_PARTITION_H_
