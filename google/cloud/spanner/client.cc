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

#include "google/cloud/spanner/client.h"
#include "google/cloud/spanner/backoff_policy.h"
#include "google/cloud/spanner/internal/connection_impl.h"
#include "google/cloud/spanner/internal/retry_loop.h"
#include "google/cloud/spanner/internal/spanner_stub.h"
#include "google/cloud/spanner/retry_policy.h"
#include "google/cloud/log.h"
#include <grpcpp/grpcpp.h>
#include <thread>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {

RowStream Client::Read(std::string table, KeySet keys,
                       std::vector<std::string> columns,
                       ReadOptions read_options) {
  return conn_->Read(
      {internal::MakeSingleUseTransaction(Transaction::ReadOnlyOptions()),
       std::move(table), std::move(keys), std::move(columns),
       std::move(read_options)});
}

RowStream Client::Read(Transaction::SingleUseOptions transaction_options,
                       std::string table, KeySet keys,
                       std::vector<std::string> columns,
                       ReadOptions read_options) {
  return conn_->Read(
      {internal::MakeSingleUseTransaction(std::move(transaction_options)),
       std::move(table), std::move(keys), std::move(columns),
       std::move(read_options)});
}

RowStream Client::Read(Transaction transaction, std::string table, KeySet keys,
                       std::vector<std::string> columns,
                       ReadOptions read_options) {
  return conn_->Read({std::move(transaction), std::move(table), std::move(keys),
                      std::move(columns), std::move(read_options)});
}

RowStream Client::Read(ReadPartition const& read_partition) {
  return conn_->Read(internal::MakeReadParams(read_partition));
}

StatusOr<std::vector<ReadPartition>> Client::PartitionRead(
    Transaction transaction, std::string table, KeySet keys,
    std::vector<std::string> columns, ReadOptions read_options,
    PartitionOptions const& partition_options) {
  return conn_->PartitionRead(
      {{std::move(transaction), std::move(table), std::move(keys),
        std::move(columns), std::move(read_options)},
       partition_options});
}

RowStream Client::ExecuteQuery(SqlStatement statement) {
  return conn_->ExecuteQuery(
      {internal::MakeSingleUseTransaction(Transaction::ReadOnlyOptions()),
       std::move(statement)});
}

RowStream Client::ExecuteQuery(
    Transaction::SingleUseOptions transaction_options, SqlStatement statement) {
  return conn_->ExecuteQuery(
      {internal::MakeSingleUseTransaction(std::move(transaction_options)),
       std::move(statement)});
}

RowStream Client::ExecuteQuery(Transaction transaction,
                               SqlStatement statement) {
  return conn_->ExecuteQuery({std::move(transaction), std::move(statement)});
}

RowStream Client::ExecuteQuery(QueryPartition const& partition) {
  return conn_->ExecuteQuery(internal::MakeSqlParams(partition));
}

ProfileQueryResult Client::ProfileQuery(SqlStatement statement) {
  return conn_->ProfileQuery(
      {internal::MakeSingleUseTransaction(Transaction::ReadOnlyOptions()),
       std::move(statement)});
}

ProfileQueryResult Client::ProfileQuery(
    Transaction::SingleUseOptions transaction_options, SqlStatement statement) {
  return conn_->ProfileQuery(
      {internal::MakeSingleUseTransaction(std::move(transaction_options)),
       std::move(statement)});
}

ProfileQueryResult Client::ProfileQuery(Transaction transaction,
                                        SqlStatement statement) {
  return conn_->ProfileQuery({std::move(transaction), std::move(statement)});
}

StatusOr<std::vector<QueryPartition>> Client::PartitionQuery(
    Transaction transaction, SqlStatement statement,
    PartitionOptions const& partition_options) {
  return conn_->PartitionQuery(
      {{std::move(transaction), std::move(statement)}, partition_options});
}

StatusOr<DmlResult> Client::ExecuteDml(Transaction transaction,
                                       SqlStatement statement) {
  return conn_->ExecuteDml({std::move(transaction), std::move(statement)});
}

StatusOr<ProfileDmlResult> Client::ProfileDml(Transaction transaction,
                                              SqlStatement statement) {
  return conn_->ProfileDml({std::move(transaction), std::move(statement)});
}

StatusOr<ExecutionPlan> Client::AnalyzeSql(Transaction transaction,
                                           SqlStatement statement) {
  return conn_->AnalyzeSql({std::move(transaction), std::move(statement)});
}

StatusOr<BatchDmlResult> Client::ExecuteBatchDml(
    Transaction transaction, std::vector<SqlStatement> statements) {
  return conn_->ExecuteBatchDml(
      {std::move(transaction), std::move(statements)});
}

StatusOr<CommitResult> Client::Commit(
    std::function<StatusOr<Mutations>(Transaction)> const& mutator,
    std::unique_ptr<TransactionRerunPolicy> rerun_policy,
    std::unique_ptr<BackoffPolicy> backoff_policy) {
  // The status-code discriminator of TransactionRerunPolicy.
  using RerunnablePolicy = internal::SafeTransactionRerun;

  Transaction txn = MakeReadWriteTransaction();
  for (int rerun = 0;; ++rerun) {
    StatusOr<Mutations> mutations;
#if GOOGLE_CLOUD_CPP_HAVE_EXCEPTIONS
    try {
#endif
      mutations = mutator(txn);
#if GOOGLE_CLOUD_CPP_HAVE_EXCEPTIONS
    } catch (...) {
      auto rb_status = Rollback(txn);
      if (!RerunnablePolicy::IsOk(rb_status)) {
        GCP_LOG(WARNING) << "Rollback() failure in Client::Commit(): "
                         << rb_status.message();
      }
      throw;
    }
#endif
    auto status = mutations.status();
    if (RerunnablePolicy::IsOk(status)) {
      auto result = Commit(txn, *mutations);
      status = result.status();
      if (!RerunnablePolicy::IsTransientFailure(status)) {
        return result;
      }
    } else {
      if (!RerunnablePolicy::IsTransientFailure(status)) {
        auto rb_status = Rollback(txn);
        if (!RerunnablePolicy::IsOk(rb_status)) {
          GCP_LOG(WARNING) << "Rollback() failure in Client::Commit(): "
                           << rb_status.message();
        }
        return status;
      }
    }
    // A transient failure (i.e., kAborted), so consider rerunning.
    if (!rerun_policy->OnFailure(status)) {
      return status;  // reruns exhausted
    }
    std::this_thread::sleep_for(backoff_policy->OnCompletion());
    // Create a new transaction for the next loop, but share lock priority
    // so that we have a slightly better chance of avoiding another abort.
    txn = MakeReadWriteTransaction(std::move(txn));
  }
}

StatusOr<CommitResult> Client::Commit(
    std::function<StatusOr<Mutations>(Transaction)> const& mutator) {
  auto const rerun_maximum_duration = std::chrono::minutes(10);
  auto default_commit_rerun_policy =
      LimitedTimeTransactionRerunPolicy(rerun_maximum_duration).clone();

  auto const backoff_initial_delay = std::chrono::milliseconds(100);
  auto const backoff_maximum_delay = std::chrono::minutes(5);
  auto const backoff_scaling = 2.0;
  auto default_commit_backoff_policy =
      ExponentialBackoffPolicy(backoff_initial_delay, backoff_maximum_delay,
                               backoff_scaling)
          .clone();

  return Commit(mutator, std::move(default_commit_rerun_policy),
                std::move(default_commit_backoff_policy));
}

StatusOr<CommitResult> Client::Commit(Transaction transaction,
                                      Mutations mutations) {
  return conn_->Commit({std::move(transaction), std::move(mutations)});
}

Status Client::Rollback(Transaction transaction) {
  return conn_->Rollback({std::move(transaction)});
}

StatusOr<PartitionedDmlResult> Client::ExecutePartitionedDml(
    SqlStatement statement) {
  return conn_->ExecutePartitionedDml({std::move(statement)});
}

std::shared_ptr<Connection> MakeConnection(
    Database const& db, ConnectionOptions const& connection_options,
    SessionPoolOptions session_pool_options) {
  return MakeConnection(db, connection_options, std::move(session_pool_options),
                        internal::DefaultConnectionRetryPolicy(),
                        internal::DefaultConnectionBackoffPolicy());
}

std::shared_ptr<Connection> MakeConnection(
    Database const& db, ConnectionOptions const& connection_options,
    SessionPoolOptions session_pool_options,
    std::unique_ptr<RetryPolicy> retry_policy,
    std::unique_ptr<BackoffPolicy> backoff_policy) {
  std::vector<std::shared_ptr<internal::SpannerStub>> stubs;
  int num_channels = std::min(connection_options.num_channels(), 1);
  stubs.reserve(num_channels);
  for (int channel_id = 0; channel_id < num_channels; ++channel_id) {
    stubs.push_back(
        internal::CreateDefaultSpannerStub(connection_options, channel_id));
  }
  return internal::MakeConnection(
      db, std::move(stubs), std::move(session_pool_options),
      std::move(retry_policy), std::move(backoff_policy));
}

}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google
