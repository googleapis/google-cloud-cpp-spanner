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
#include <string>
#include <vector>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {

class Client {
 public:
  // move-only
  Client(Client&&) = default;
  Client& operator=(Client&&) = default;
  Client(Client const&) = delete;
  Client& operator=(Client const&) = delete;

  //
  // Read()
  //

  // Reads the columns for the given keys from the specified table. Returns a
  // stream of Rows.
  // TODO: Add support for "limit" integer.
  ResultSet Read(Transaction tx, std::string table, KeySet keys,
                 std::vector<std::string> columns) {
    // Fills in two rows of dummy data.
    std::vector<Row> v;
    int64_t data = 1;
    v.push_back(Row{});
    for (auto const& c : columns) {
      (void)c;  // unused
      v.back().AddValue(Value(data++));
    }
    v.push_back(Row{});
    for (auto const& c : columns) {
      (void)c;  // unused
      v.back().AddValue(Value(data++));
    }
    return ResultSet(v);
  }

  // Same as Read() above, but implicitly uses a single-use Transaction.
  ResultSet Read(std::string table, KeySet keys,
                 std::vector<std::string> columns) {
    auto single_use = Transaction::MakeSingleUseTransaction();
    return Read(std::move(single_use), std::move(table), std::move(keys),
                std::move(columns));
  }

  ResultSet Read(ReadPartition partition) {
    // TODO: Call Spanner's StreamingRead RPC with the data in `partition`.
    return {};
  }

  // NOTE: Requires a read-only transaction
  google::cloud::StatusOr<std::vector<ReadPartition>> PartitionRead(
      Transaction tx, std::string table, KeySet keys,
      std::vector<std::string> columns, PartitionOptions opts = {}) {
    // TODO: Call Spanner's PartitionRead() RPC.
    return {};
  }

  //
  // SQL methods
  //

  // TODO: Add support for QueryMode
  ResultSet ExecuteSql(Transaction tx, SqlStatement statement) {
    auto columns = {"col1", "col2", "col3"};
    // Fills in two rows of dummy data.
    std::vector<Row> v;
    double data = 1;
    v.push_back(Row{});
    for (auto const& c : columns) {
      (void)c;  // unused
      v.back().AddValue(Value(data));
      data += 1;
    }
    v.push_back(Row{});
    for (auto const& c : columns) {
      (void)c;  // unused
      v.back().AddValue(Value(data));
      data += 1;
    }
    return ResultSet(v);
  }

  ResultSet ExecuteSql(SqlStatement statement) {
    auto single_use = Transaction::MakeSingleUseTransaction();
    return ExecuteSql(std::move(single_use), std::move(statement));
  }

  ResultSet ExecuteSql(SqlPartition partition) {
    // TODO: Call Spanner's StreamingExecuteSql RPC with the data in
    // `partition`.
    return {};
  }

  // NOTE: Requires a read-only transaction
  google::cloud::StatusOr<std::vector<SqlPartition>> PartitionQuery(
      Transaction tx, SqlStatement statement, PartitionOptions opts = {}) {
    // TODO: Call Spanner's PartitionQuery() RPC.
    return {};
  }

  //
  // DML methods
  //

  // Note: Does not support single-use transactions, so no overload for that.
  // Note: statements.size() == result.size()
  std::vector<google::cloud::StatusOr<ResultStats>> ExecuteBatchDml(
      Transaction tx, std::vector<SqlStatement> statements) {
    // TODO: Call spanner's ExecuteBatchDml RPC.
    return {};
  }

  google::cloud::StatusOr<int64_t> ExecutePartitionedDml(
      SqlStatement statement) {
    // TODO: Call ExecuteSql() with a PartitionedDmlTransaction
    Transaction dml = Transaction::MakePartitionedDmlTransaction();
    ResultSet r = ExecuteSql(dml, statement);
    (void)r;  // unused
    // Look at the result set stats and return the "row_count_lower_bound
    return 42;
  }

  // TODO: Commit and Rollback should be private and only callable by a
  // transaction runner rather than allowing the user to call them directly.

  // Transactions

  google::cloud::StatusOr<std::chrono::system_clock::time_point> Commit(
      Transaction tx, std::vector<Mutation> mutations) {
    // TODO: Call Spanner's Commit() rpc.
    return std::chrono::system_clock::now();
  }

  google::cloud::Status Rollback(Transaction tx) {
    // TODO: Call Spanner's Rollback() rpc.
    return {};
  }

 private:
  friend google::cloud::StatusOr<Client> MakeClient(
      std::map<std::string, std::string>);
  friend google::cloud::StatusOr<Client> MakeClient(Transaction tx);

  Client(std::map<std::string, std::string> labels)
      : labels_{std::move(labels)} {}
  Client(std::string session, std::map<std::string, std::string> labels)
      : sessions_{std::move(session)}, labels_{std::move(labels)} {}

  std::vector<std::string> sessions_;
  std::map<std::string, std::string> labels_;
  // grpc stubs.
};

google::cloud::StatusOr<Client> MakeClient(
    std::map<std::string, std::string> labels = {}) {
  // TODO: Make a connection to Spanner, set up stubs, etc.
  return Client(std::move(labels));
}

google::cloud::StatusOr<Client> MakeClient(Transaction tx) {
  // TODO: Call rpc.GetSession() using tx.session_ to get labels
  std::map<std::string, std::string> labels = {};
  std::string session;  // TODO: Get this from tx
  return Client(std::move(session), std::move(labels));
}

}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_CLIENT_H_
