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

#ifndef GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_CLIENT_H_
#define GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_CLIENT_H_

#include "google/cloud/spanner/key.h"
#include "google/cloud/spanner/mutation.h"
#include "google/cloud/spanner/partition.h"
#include "google/cloud/spanner/result_set.h"
#include "google/cloud/spanner/result_stats.h"
#include "google/cloud/spanner/row.h"
#include "google/cloud/spanner/sql_statement.h"
#include "google/cloud/spanner/transaction.h"
#include "google/cloud/spanner/value.h"
#include "google/cloud/spanner/version.h"
#include "google/cloud/status.h"
#include "google/cloud/status_or.h"
#include <chrono>
#include <functional>
#include <string>
#include <vector>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {
/**
 * This is the primary class through which customers will interact with Spanner.
 * It represents a connection to a Spanner database. It manages a pool of
 * Spanner sessions, hiding all the details of the session from users. Client
 * instances may be constructed with a map of key-value pairs, which correspond
 * with the labels associated with the sessions. All sessions managed by a
 * Client instance will have the same labels.
 *
 * The spanner::Client will create the first session (with the appropriate
 * labels) when it's created. When the caller invokes a method requiring an RPC,
 * that session will be used. If too much time has passed and the session is
 * expired, the RPC will fail, our library will detect this, it will create a
 * new session, and retry the RPC on behalf of the user. The spanner::Client
 * will not keep sessions alive on its own. Callers may do this by periodically
 * invoking RPCs (i.e., sending a "select 1;").
 */
class Client {
 public:
  /**
   * Options used when creating a client.
   */
  struct ClientOptions {
    std::map<std::string, std::string> labels;
  };

  /**
   * Factory method to create a Client.
   */
  static StatusOr<Client> MakeClient(ClientOptions opts = {}) {
    return Status(StatusCode::kUnimplemented, "unimplemented");
  }

  Client(Client&&) = default;
  Client& operator=(Client&&) = default;
  Client(Client const&) = default;
  Client& operator=(Client const&) = default;

  friend bool operator==(Client a, Client b);
  friend bool operator!=(Client a, Client b);

  //@{
  /**
   * Read methods
   */
  ResultSet Read(std::string table, KeySet keys,
                 std::vector<std::string> columns) {
    return {};
  }
  ResultSet Read(Transaction tx, std::string table, KeySet keys,
                 std::vector<std::string> columns) {
    return {};
  }
  ResultSet Read(ReadPartition partition) { return {}; }
  StatusOr<std::vector<ReadPartition>> PartitionRead(
      Transaction tx, std::string table, KeySet keys,
      std::vector<std::string> columns, PartitionOptions opts = {}) {
    return {};
  }
  //@}

  //@{
  /**
   * ExecuteSql methods
   */
  ResultSet ExecuteSql(SqlStatement statement) { return {}; }
  ResultSet ExecuteSql(Transaction tx, SqlStatement statement) { return {}; }
  ResultSet ExecuteSql(SqlPartition sql) { return {}; }
  StatusOr<std::vector<SqlPartition>> PartitionQuery(
      Transaction tx, SqlStatement statement, PartitionOptions opts = {}) {
    return Status(StatusCode::kUnimplemented, "unimplemented");
  }
  //@}

  //@{
  /**
   * DML methods
   */
  std::vector<StatusOr<ResultStats>> ExecuteBatchDml(
      Transaction tx, std::vector<SqlStatement> statements) {
    return {Status(StatusCode::kUnimplemented, "unimplemented")};
  }
  StatusOr<int64_t> ExecutePartitionedDml(SqlStatement statement) {
    return Status(StatusCode::kUnimplemented, "unimplemented");
  }
  //@}

  //@{
  /**
   * Runs the caller-provided function `f` with a read-only Transaction object.
   * The return type T is whatever `f` returns.
   */
  template <typename T>
  T RunTransaction(Transaction::ReadOnlyOptions opts,
                   std::function<T(Client& c, Transaction txn)> f) {
    Transaction tx("transaction_id");
    return f(*this, tx);
  }

  struct TransactionStatus {
    enum { COMMIT, ROLLBACK } action;
    std::vector<spanner::Mutation> mutations;
  };
  struct CommitResult {
    Status status;
  };
  /**
   * Runs the caller-provided function `f` with a read-write Transaction object.
   */
  StatusOr<CommitResult> RunTransaction(
      Transaction::ReadWriteOptions opts,
      std::function<TransactionStatus(Client& c, Transaction txn)> f) {
    Transaction tx("transaction_id");
    TransactionStatus tx_status = f(*this, tx);
    return {};
  }
  //@}

 private:
  Client() = default;

  //@{
  /**
   * PRIVATE: Transactional methods
   * The transaction runner class will be a friend so it can call these, but
   * users should not call them directly.
   */
  StatusOr<std::chrono::system_clock::time_point> Commit(
      Transaction tx, std::vector<Mutation> v) {
    return Status(StatusCode::kUnimplemented, "unimplemented");
  }
  Status Rollback(Transaction tx) {
    return Status(StatusCode::kUnimplemented, "unimplemented");
  }
  //@}
};
}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_CLIENT_H_
