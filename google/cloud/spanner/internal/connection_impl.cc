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

#include "google/cloud/spanner/internal/connection_impl.h"
#include "google/cloud/spanner/internal/logging_result_set_reader.h"
#include "google/cloud/spanner/internal/partial_result_set_resume.h"
#include "google/cloud/spanner/internal/partial_result_set_source.h"
#include "google/cloud/spanner/internal/retry_loop.h"
#include "google/cloud/spanner/internal/status_utils.h"
#include "google/cloud/spanner/query_partition.h"
#include "google/cloud/spanner/read_partition.h"
#include "google/cloud/grpc_error_delegate.h"
#include "google/cloud/internal/make_unique.h"
#include <limits>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {
namespace internal {

class DefaultPartialResultSetReader : public PartialResultSetReader {
 public:
  DefaultPartialResultSetReader(
      std::unique_ptr<grpc::ClientContext> context,
      std::unique_ptr<
          grpc::ClientReaderInterface<google::spanner::v1::PartialResultSet>>
          reader)
      : context_(std::move(context)), reader_(std::move(reader)) {}

  ~DefaultPartialResultSetReader() override = default;

  void TryCancel() override { context_->TryCancel(); }

  optional<google::spanner::v1::PartialResultSet> Read() override {
    google::spanner::v1::PartialResultSet result;
    bool success = reader_->Read(&result);
    if (!success) return {};
    return result;
  }

  Status Finish() override {
    return google::cloud::MakeStatusFromRpcError(reader_->Finish());
  }

 private:
  std::unique_ptr<grpc::ClientContext> context_;
  std::unique_ptr<
      grpc::ClientReaderInterface<google::spanner::v1::PartialResultSet>>
      reader_;
};

namespace spanner_proto = ::google::spanner::v1;

std::unique_ptr<RetryPolicy> DefaultConnectionRetryPolicy() {
  return google::cloud::spanner::LimitedTimeRetryPolicy(
             std::chrono::minutes(10))
      .clone();
}

std::unique_ptr<BackoffPolicy> DefaultConnectionBackoffPolicy() {
  auto constexpr kBackoffScaling = 2.0;
  return google::cloud::spanner::ExponentialBackoffPolicy(
             std::chrono::milliseconds(100), std::chrono::minutes(1),
             kBackoffScaling)
      .clone();
}

std::shared_ptr<ConnectionImpl> MakeConnection(
    Database db, std::vector<std::shared_ptr<SpannerStub>> stubs,
    ConnectionOptions const& options, SessionPoolOptions session_pool_options,
    std::unique_ptr<RetryPolicy> retry_policy,
    std::unique_ptr<BackoffPolicy> backoff_policy) {
  return std::shared_ptr<ConnectionImpl>(new ConnectionImpl(
      std::move(db), std::move(stubs), options, std::move(session_pool_options),
      std::move(retry_policy), std::move(backoff_policy)));
}

ConnectionImpl::ConnectionImpl(Database db,
                               std::vector<std::shared_ptr<SpannerStub>> stubs,
                               ConnectionOptions const& options,
                               SessionPoolOptions session_pool_options,
                               std::unique_ptr<RetryPolicy> retry_policy,
                               std::unique_ptr<BackoffPolicy> backoff_policy)
    : db_(std::move(db)),
      retry_policy_prototype_(std::move(retry_policy)),
      backoff_policy_prototype_(std::move(backoff_policy)),
      background_threads_(options.background_threads_factory()()),
      session_pool_(MakeSessionPool(
          db_, std::move(stubs), std::move(session_pool_options),
          background_threads_->cq(), retry_policy_prototype_->clone(),
          backoff_policy_prototype_->clone())),
      rpc_stream_tracing_enabled_(options.tracing_enabled("rpc-streams")),
      tracing_options_(options.tracing_options()) {}

RowStream ConnectionImpl::Read(ReadParams params) {
  return internal::Visit(
      std::move(params.transaction),
      [this, &params](SessionHolder& session,
                      spanner_proto::TransactionSelector& s, std::int64_t) {
        return ReadImpl(session, s, std::move(params));
      });
}

StatusOr<std::vector<ReadPartition>> ConnectionImpl::PartitionRead(
    PartitionReadParams params) {
  return internal::Visit(
      std::move(params.read_params.transaction),
      [this, &params](SessionHolder& session,
                      spanner_proto::TransactionSelector& s, std::int64_t) {
        return PartitionReadImpl(session, s, params.read_params,
                                 params.partition_options);
      });
}

RowStream ConnectionImpl::ExecuteQuery(SqlParams params) {
  return internal::Visit(std::move(params.transaction),
                         [this, &params](SessionHolder& session,
                                         spanner_proto::TransactionSelector& s,
                                         std::int64_t seqno) {
                           return ExecuteQueryImpl(session, s, seqno,
                                                   std::move(params));
                         });
}

StatusOr<DmlResult> ConnectionImpl::ExecuteDml(SqlParams params) {
  return internal::Visit(std::move(params.transaction),
                         [this, &params](SessionHolder& session,
                                         spanner_proto::TransactionSelector& s,
                                         std::int64_t seqno) {
                           return ExecuteDmlImpl(session, s, seqno,
                                                 std::move(params));
                         });
}

ProfileQueryResult ConnectionImpl::ProfileQuery(SqlParams params) {
  return internal::Visit(std::move(params.transaction),
                         [this, &params](SessionHolder& session,
                                         spanner_proto::TransactionSelector& s,
                                         std::int64_t seqno) {
                           return ProfileQueryImpl(session, s, seqno,
                                                   std::move(params));
                         });
}

StatusOr<ProfileDmlResult> ConnectionImpl::ProfileDml(SqlParams params) {
  return internal::Visit(std::move(params.transaction),
                         [this, &params](SessionHolder& session,
                                         spanner_proto::TransactionSelector& s,
                                         std::int64_t seqno) {
                           return ProfileDmlImpl(session, s, seqno,
                                                 std::move(params));
                         });
}

StatusOr<ExecutionPlan> ConnectionImpl::AnalyzeSql(SqlParams params) {
  return internal::Visit(std::move(params.transaction),
                         [this, &params](SessionHolder& session,
                                         spanner_proto::TransactionSelector& s,
                                         std::int64_t seqno) {
                           return AnalyzeSqlImpl(session, s, seqno,
                                                 std::move(params));
                         });
}

StatusOr<PartitionedDmlResult> ConnectionImpl::ExecutePartitionedDml(
    ExecutePartitionedDmlParams params) {
  auto txn = MakeReadOnlyTransaction();
  return internal::Visit(
      txn, [this, &params](SessionHolder& session,
                           spanner_proto::TransactionSelector& s,
                           std::int64_t seqno) {
        return ExecutePartitionedDmlImpl(session, s, seqno, std::move(params));
      });
}

StatusOr<std::vector<QueryPartition>> ConnectionImpl::PartitionQuery(
    PartitionQueryParams params) {
  return internal::Visit(
      std::move(params.transaction),
      [this, &params](SessionHolder& session,
                      spanner_proto::TransactionSelector& s, std::int64_t) {
        return PartitionQueryImpl(session, s, params);
      });
}

StatusOr<BatchDmlResult> ConnectionImpl::ExecuteBatchDml(
    ExecuteBatchDmlParams params) {
  return internal::Visit(std::move(params.transaction),
                         [this, &params](SessionHolder& session,
                                         spanner_proto::TransactionSelector& s,
                                         std::int64_t seqno) {
                           return ExecuteBatchDmlImpl(session, s, seqno,
                                                      std::move(params));
                         });
}

StatusOr<CommitResult> ConnectionImpl::Commit(CommitParams params) {
  return internal::Visit(
      std::move(params.transaction),
      [this, &params](SessionHolder& session,
                      spanner_proto::TransactionSelector& s, std::int64_t) {
        return this->CommitImpl(session, s, std::move(params));
      });
}

Status ConnectionImpl::Rollback(RollbackParams params) {
  return internal::Visit(
      std::move(params.transaction),
      [this](SessionHolder& session, spanner_proto::TransactionSelector& s,
             std::int64_t) { return this->RollbackImpl(session, s); });
}

class StatusOnlyResultSetSource : public internal::ResultSourceInterface {
 public:
  explicit StatusOnlyResultSetSource(google::cloud::Status status)
      : status_(std::move(status)) {}
  ~StatusOnlyResultSetSource() override = default;

  StatusOr<Row> NextRow() override { return status_; }
  optional<google::spanner::v1::ResultSetMetadata> Metadata() override {
    return {};
  }
  optional<google::spanner::v1::ResultSetStats> Stats() const override {
    return {};
  }

 private:
  google::cloud::Status status_;
};

// Helper function to build and wrap a `StatusOnlyResultSetSource`.
template <typename ResultType>
ResultType MakeStatusOnlyResult(Status status) {
  return ResultType(
      google::cloud::internal::make_unique<StatusOnlyResultSetSource>(
          std::move(status)));
}

class DmlResultSetSource : public internal::ResultSourceInterface {
 public:
  static StatusOr<std::unique_ptr<ResultSourceInterface>> Create(
      spanner_proto::ResultSet result_set) {
    return std::unique_ptr<ResultSourceInterface>(
        new DmlResultSetSource(std::move(result_set)));
  }

  explicit DmlResultSetSource(spanner_proto::ResultSet result_set)
      : result_set_(std::move(result_set)) {}
  ~DmlResultSetSource() override = default;

  StatusOr<Row> NextRow() override { return {}; }

  optional<google::spanner::v1::ResultSetMetadata> Metadata() override {
    if (result_set_.has_metadata()) {
      return result_set_.metadata();
    }
    return {};
  }

  optional<google::spanner::v1::ResultSetStats> Stats() const override {
    if (result_set_.has_stats()) {
      return result_set_.stats();
    }
    return {};
  }

 private:
  spanner_proto::ResultSet result_set_;
};

/**
 * Helper function that ensures `session` holds a valid `Session`, or returns
 * an error if `session` is empty and no `Session` can be allocated.
 */
Status ConnectionImpl::PrepareSession(SessionHolder& session,
                                      bool dissociate_from_pool) {
  if (!session) {
    auto session_or = session_pool_->Allocate(dissociate_from_pool);
    if (!session_or) {
      return std::move(session_or).status();
    }
    session = std::move(*session_or);
  }
  return Status();
}

RowStream ConnectionImpl::ReadImpl(SessionHolder& session,
                                   spanner_proto::TransactionSelector& s,
                                   ReadParams params) {
  auto prepare_status = PrepareSession(session);
  if (!prepare_status.ok()) {
    return MakeStatusOnlyResult<RowStream>(std::move(prepare_status));
  }

  spanner_proto::ReadRequest request;
  request.set_session(session->session_name());
  *request.mutable_transaction() = s;
  request.set_table(std::move(params.table));
  request.set_index(std::move(params.read_options.index_name));
  for (auto&& column : params.columns) {
    request.add_columns(std::move(column));
  }
  *request.mutable_key_set() = internal::ToProto(std::move(params.keys));
  request.set_limit(params.read_options.limit);
  if (params.partition_token) {
    request.set_partition_token(*std::move(params.partition_token));
  }

  // Capture a copy of `stub` to ensure the `shared_ptr<>` remains valid through
  // the lifetime of the lambda.
  auto stub = session_pool_->GetStub(*session);
  auto const tracing_enabled = rpc_stream_tracing_enabled_;
  auto const tracing_options = tracing_options_;
  auto factory = [stub, request, tracing_enabled,
                  tracing_options](std::string const& resume_token) mutable {
    request.set_resume_token(resume_token);
    auto context = google::cloud::internal::make_unique<grpc::ClientContext>();
    std::unique_ptr<PartialResultSetReader> reader =
        google::cloud::internal::make_unique<DefaultPartialResultSetReader>(
            std::move(context), stub->StreamingRead(*context, request));
    if (tracing_enabled) {
      reader = google::cloud::internal::make_unique<LoggingResultSetReader>(
          std::move(reader), tracing_options);
    }
    return reader;
  };
  auto rpc = google::cloud::internal::make_unique<PartialResultSetResume>(
      std::move(factory), Idempotency::kIdempotent,
      retry_policy_prototype_->clone(), backoff_policy_prototype_->clone());
  auto reader = PartialResultSetSource::Create(std::move(rpc));
  if (!reader.ok()) {
    auto status = std::move(reader).status();
    if (internal::IsSessionNotFound(status)) session->set_bad();
    return MakeStatusOnlyResult<RowStream>(std::move(status));
  }
  if (s.has_begin()) {
    auto metadata = (*reader)->Metadata();
    if (!metadata || metadata->transaction().id().empty()) {
      return MakeStatusOnlyResult<RowStream>(
          Status(StatusCode::kInternal,
                 "Begin transaction requested but no transaction returned (in "
                 "Read)."));
    }
    s.set_id(metadata->transaction().id());
  }
  return RowStream(*std::move(reader));
}

StatusOr<std::vector<ReadPartition>> ConnectionImpl::PartitionReadImpl(
    SessionHolder& session, spanner_proto::TransactionSelector& s,
    ReadParams const& params, PartitionOptions const& partition_options) {
  // Since the session may be sent to other machines, it should not be returned
  // to the pool when the Transaction is destroyed.
  auto prepare_status = PrepareSession(session, /*dissociate_from_pool=*/true);
  if (!prepare_status.ok()) {
    return prepare_status;
  }

  spanner_proto::PartitionReadRequest request;
  request.set_session(session->session_name());
  *request.mutable_transaction() = s;
  request.set_table(params.table);
  request.set_index(params.read_options.index_name);
  for (auto const& column : params.columns) {
    *request.add_columns() = column;
  }
  *request.mutable_key_set() = internal::ToProto(params.keys);
  *request.mutable_partition_options() = internal::ToProto(partition_options);

  auto stub = session_pool_->GetStub(*session);
  auto response = internal::RetryLoop(
      retry_policy_prototype_->clone(), backoff_policy_prototype_->clone(),
      true,
      [&stub](grpc::ClientContext& context,
              spanner_proto::PartitionReadRequest const& request) {
        return stub->PartitionRead(context, request);
      },
      request, __func__);
  if (!response.ok()) {
    auto status = std::move(response).status();
    if (internal::IsSessionNotFound(status)) session->set_bad();
    return status;
  }

  if (s.has_begin()) {
    s.set_id(response->transaction().id());
  }

  std::vector<ReadPartition> read_partitions;
  for (auto& partition : response->partitions()) {
    read_partitions.push_back(internal::MakeReadPartition(
        response->transaction().id(), session->session_name(),
        partition.partition_token(), params.table, params.keys, params.columns,
        params.read_options));
  }

  return read_partitions;
}

template <typename ResultType>
StatusOr<ResultType> ConnectionImpl::ExecuteSqlImpl(
    SessionHolder& session, google::spanner::v1::TransactionSelector& s,
    std::int64_t seqno, SqlParams params,
    google::spanner::v1::ExecuteSqlRequest::QueryMode query_mode,
    std::function<StatusOr<std::unique_ptr<ResultSourceInterface>>(
        google::spanner::v1 ::ExecuteSqlRequest& request)> const&
        retry_resume_fn) {
  spanner_proto::ExecuteSqlRequest request;
  request.set_session(session->session_name());
  *request.mutable_transaction() = s;
  auto sql_statement = internal::ToProto(std::move(params.statement));
  request.set_sql(std::move(*sql_statement.mutable_sql()));
  *request.mutable_params() = std::move(*sql_statement.mutable_params());
  *request.mutable_param_types() =
      std::move(*sql_statement.mutable_param_types());
  request.set_seqno(seqno);
  request.set_query_mode(query_mode);
  if (params.partition_token) {
    request.set_partition_token(*std::move(params.partition_token));
  }
  if (params.query_options.optimizer_version()) {
    request.mutable_query_options()->set_optimizer_version(
        *params.query_options.optimizer_version());
  }
  auto reader = retry_resume_fn(request);
  if (!reader.ok()) {
    return std::move(reader).status();
  }
  if (s.has_begin()) {
    auto metadata = (*reader)->Metadata();
    if (!metadata || metadata->transaction().id().empty()) {
      return Status(StatusCode::kInternal,
                    "Begin transaction requested but no transaction returned.");
    }
    s.set_id(metadata->transaction().id());
  }
  return ResultType(std::move(*reader));
}

template <typename ResultType>
ResultType ConnectionImpl::CommonQueryImpl(
    SessionHolder& session, spanner_proto::TransactionSelector& s,
    std::int64_t seqno, SqlParams params,
    google::spanner::v1::ExecuteSqlRequest::QueryMode query_mode) {
  auto prepare_status = PrepareSession(session);
  if (!prepare_status.ok()) {
    return MakeStatusOnlyResult<ResultType>(std::move(prepare_status));
  }
  // Capture a copy of of these to ensure the `shared_ptr<>` remains valid
  // through the lifetime of the lambda. Note that the local variables are a
  // reference to avoid increasing refcounts twice, but the capture is by value.
  auto stub = session_pool_->GetStub(*session);
  auto const& retry_policy = retry_policy_prototype_;
  auto const& backoff_policy = backoff_policy_prototype_;
  auto const tracing_enabled = rpc_stream_tracing_enabled_;
  auto const tracing_options = tracing_options_;
  auto retry_resume_fn =
      [stub, retry_policy, backoff_policy, tracing_enabled,
       tracing_options](spanner_proto::ExecuteSqlRequest& request) mutable
      -> StatusOr<std::unique_ptr<ResultSourceInterface>> {
    auto factory = [stub, request, tracing_enabled,
                    tracing_options](std::string const& resume_token) mutable {
      request.set_resume_token(resume_token);
      auto context =
          google::cloud::internal::make_unique<grpc::ClientContext>();
      std::unique_ptr<PartialResultSetReader> reader =
          google::cloud::internal::make_unique<DefaultPartialResultSetReader>(
              std::move(context), stub->ExecuteStreamingSql(*context, request));
      if (tracing_enabled) {
        reader = google::cloud::internal::make_unique<LoggingResultSetReader>(
            std::move(reader), tracing_options);
      }
      return reader;
    };
    auto rpc = google::cloud::internal::make_unique<PartialResultSetResume>(
        std::move(factory), Idempotency::kIdempotent, retry_policy->clone(),
        backoff_policy->clone());

    return PartialResultSetSource::Create(std::move(rpc));
  };

  StatusOr<ResultType> response =
      ExecuteSqlImpl<ResultType>(session, s, seqno, std::move(params),
                                 query_mode, std::move(retry_resume_fn));
  if (!response) {
    auto status = std::move(response).status();
    if (internal::IsSessionNotFound(status)) session->set_bad();
    return MakeStatusOnlyResult<ResultType>(std::move(status));
  }
  return std::move(*response);
}

RowStream ConnectionImpl::ExecuteQueryImpl(
    SessionHolder& session, spanner_proto::TransactionSelector& s,
    std::int64_t seqno, SqlParams params) {
  return CommonQueryImpl<RowStream>(session, s, seqno, std::move(params),
                                    spanner_proto::ExecuteSqlRequest::NORMAL);
}

ProfileQueryResult ConnectionImpl::ProfileQueryImpl(
    SessionHolder& session, google::spanner::v1::TransactionSelector& s,
    std::int64_t seqno, SqlParams params) {
  return CommonQueryImpl<ProfileQueryResult>(
      session, s, seqno, std::move(params),
      spanner_proto::ExecuteSqlRequest::PROFILE);
}

template <typename ResultType>
StatusOr<ResultType> ConnectionImpl::CommonDmlImpl(
    SessionHolder& session, spanner_proto::TransactionSelector& s,
    std::int64_t seqno, SqlParams params,
    google::spanner::v1::ExecuteSqlRequest::QueryMode query_mode) {
  auto function_name = __func__;
  auto prepare_status = PrepareSession(session);
  if (!prepare_status.ok()) {
    return prepare_status;
  }
  // Capture a copy of of these to ensure the `shared_ptr<>` remains valid
  // through the lifetime of the lambda. Note that the local variables are a
  // reference to avoid increasing refcounts twice, but the capture is by value.
  auto stub = session_pool_->GetStub(*session);
  auto const& retry_policy = retry_policy_prototype_;
  auto const& backoff_policy = backoff_policy_prototype_;

  auto retry_resume_fn =
      [function_name, stub, retry_policy, backoff_policy,
       session](spanner_proto::ExecuteSqlRequest& request) mutable
      -> StatusOr<std::unique_ptr<ResultSourceInterface>> {
    StatusOr<spanner_proto::ResultSet> response = internal::RetryLoop(
        retry_policy->clone(), backoff_policy->clone(), true,
        [stub](grpc::ClientContext& context,
               spanner_proto::ExecuteSqlRequest const& request) {
          return stub->ExecuteSql(context, request);
        },
        request, function_name);
    if (!response) {
      auto status = std::move(response).status();
      if (internal::IsSessionNotFound(status)) session->set_bad();
      return status;
    }
    return DmlResultSetSource::Create(std::move(*response));
  };
  return ExecuteSqlImpl<ResultType>(session, s, seqno, std::move(params),
                                    query_mode, std::move(retry_resume_fn));
}

StatusOr<DmlResult> ConnectionImpl::ExecuteDmlImpl(
    SessionHolder& session, spanner_proto::TransactionSelector& s,
    std::int64_t seqno, SqlParams params) {
  return CommonDmlImpl<DmlResult>(session, s, seqno, std::move(params),
                                  spanner_proto::ExecuteSqlRequest::NORMAL);
}

StatusOr<ProfileDmlResult> ConnectionImpl::ProfileDmlImpl(
    SessionHolder& session, google::spanner::v1::TransactionSelector& s,
    std::int64_t seqno, SqlParams params) {
  return CommonDmlImpl<ProfileDmlResult>(
      session, s, seqno, std::move(params),
      spanner_proto::ExecuteSqlRequest::PROFILE);
}

StatusOr<ExecutionPlan> ConnectionImpl::AnalyzeSqlImpl(
    SessionHolder& session, google::spanner::v1::TransactionSelector& s,
    std::int64_t seqno, SqlParams params) {
  auto result =
      CommonDmlImpl<ProfileDmlResult>(session, s, seqno, std::move(params),
                                      spanner_proto::ExecuteSqlRequest::PLAN);
  if (result.status().ok()) {
    return *result->ExecutionPlan();
  }
  return result.status();
}

StatusOr<std::vector<QueryPartition>> ConnectionImpl::PartitionQueryImpl(
    SessionHolder& session, spanner_proto::TransactionSelector& s,
    PartitionQueryParams const& params) {
  // Since the session may be sent to other machines, it should not be returned
  // to the pool when the Transaction is destroyed.
  auto prepare_status = PrepareSession(session, /*dissociate_from_pool=*/true);
  if (!prepare_status.ok()) {
    return prepare_status;
  }

  spanner_proto::PartitionQueryRequest request;
  request.set_session(session->session_name());
  *request.mutable_transaction() = s;
  auto sql_statement = internal::ToProto(params.statement);
  request.set_sql(std::move(*sql_statement.mutable_sql()));
  *request.mutable_params() = std::move(*sql_statement.mutable_params());
  *request.mutable_param_types() =
      std::move(*sql_statement.mutable_param_types());
  *request.mutable_partition_options() =
      internal::ToProto(std::move(params.partition_options));

  auto stub = session_pool_->GetStub(*session);
  auto response = internal::RetryLoop(
      retry_policy_prototype_->clone(), backoff_policy_prototype_->clone(),
      true,
      [&stub](grpc::ClientContext& context,
              spanner_proto::PartitionQueryRequest const& request) {
        return stub->PartitionQuery(context, request);
      },
      request, __func__);
  if (!response.ok()) {
    auto status = std::move(response).status();
    if (internal::IsSessionNotFound(status)) session->set_bad();
    return status;
  }

  if (s.has_begin()) {
    s.set_id(response->transaction().id());
  }

  std::vector<QueryPartition> query_partitions;
  for (auto& partition : response->partitions()) {
    query_partitions.push_back(internal::MakeQueryPartition(
        response->transaction().id(), session->session_name(),
        partition.partition_token(), params.statement));
  }

  return query_partitions;
}

StatusOr<BatchDmlResult> ConnectionImpl::ExecuteBatchDmlImpl(
    SessionHolder& session, spanner_proto::TransactionSelector& s,
    std::int64_t seqno, ExecuteBatchDmlParams params) {
  auto prepare_status = PrepareSession(session);
  if (!prepare_status.ok()) {
    return prepare_status;
  }

  spanner_proto::ExecuteBatchDmlRequest request;
  request.set_session(session->session_name());
  request.set_seqno(seqno);
  *request.mutable_transaction() = s;
  for (auto& sql : params.statements) {
    *request.add_statements() = internal::ToProto(std::move(sql));
  }

  auto stub = session_pool_->GetStub(*session);
  auto response = internal::RetryLoop(
      retry_policy_prototype_->clone(), backoff_policy_prototype_->clone(),
      true,
      [&stub](grpc::ClientContext& context,
              spanner_proto::ExecuteBatchDmlRequest const& request) {
        return stub->ExecuteBatchDml(context, request);
      },
      request, __func__);
  if (!response) {
    auto status = std::move(response).status();
    if (internal::IsSessionNotFound(status)) session->set_bad();
    return status;
  }

  if (response->result_sets_size() > 0 && s.has_begin()) {
    s.set_id(response->result_sets(0).metadata().transaction().id());
  }

  BatchDmlResult result;
  result.status = google::cloud::MakeStatusFromRpcError(response->status());
  for (auto const& result_set : response->result_sets()) {
    result.stats.push_back({result_set.stats().row_count_exact()});
  }

  return result;
}

StatusOr<PartitionedDmlResult> ConnectionImpl::ExecutePartitionedDmlImpl(
    SessionHolder& session, spanner_proto::TransactionSelector& s,
    std::int64_t seqno, ExecutePartitionedDmlParams params) {
  auto prepare_status = PrepareSession(session);
  if (!prepare_status.ok()) {
    return prepare_status;
  }

  spanner_proto::BeginTransactionRequest begin_request;
  begin_request.set_session(session->session_name());
  *begin_request.mutable_options()->mutable_partitioned_dml() =
      spanner_proto::TransactionOptions_PartitionedDml();
  auto stub = session_pool_->GetStub(*session);
  auto begin_response = internal::RetryLoop(
      retry_policy_prototype_->clone(), backoff_policy_prototype_->clone(),
      true,
      [&stub](grpc::ClientContext& context,
              spanner_proto::BeginTransactionRequest const& request) {
        return stub->BeginTransaction(context, request);
      },
      begin_request, __func__);
  if (!begin_response) {
    auto status = std::move(begin_response).status();
    if (internal::IsSessionNotFound(status)) session->set_bad();
    return status;
  }
  s.set_id(begin_response->id());

  spanner_proto::ExecuteSqlRequest request;
  request.set_session(session->session_name());
  *request.mutable_transaction() = s;
  auto sql_statement = internal::ToProto(std::move(params.statement));
  request.set_sql(std::move(*sql_statement.mutable_sql()));
  *request.mutable_params() = std::move(*sql_statement.mutable_params());
  *request.mutable_param_types() =
      std::move(*sql_statement.mutable_param_types());
  request.set_seqno(seqno);
  auto response = internal::RetryLoop(
      retry_policy_prototype_->clone(), backoff_policy_prototype_->clone(),
      true,
      [&stub](grpc::ClientContext& context,
              spanner_proto::ExecuteSqlRequest const& request) {
        return stub->ExecuteSql(context, request);
      },
      request, __func__);
  if (!response) {
    auto status = std::move(response).status();
    if (internal::IsSessionNotFound(status)) session->set_bad();
    return status;
  }
  PartitionedDmlResult result{0};
  if (response->has_stats()) {
    result.row_count_lower_bound = response->stats().row_count_lower_bound();
  }

  return result;
}

StatusOr<CommitResult> ConnectionImpl::CommitImpl(
    SessionHolder& session, spanner_proto::TransactionSelector& s,
    CommitParams params) {
  auto prepare_status = PrepareSession(session);
  if (!prepare_status.ok()) {
    return prepare_status;
  }

  spanner_proto::CommitRequest request;
  request.set_session(session->session_name());
  for (auto&& m : params.mutations) {
    *request.add_mutations() = std::move(m).as_proto();
  }

  if (s.selector_case() != spanner_proto::TransactionSelector::kId) {
    spanner_proto::BeginTransactionRequest begin;
    begin.set_session(session->session_name());
    *begin.mutable_options() = s.has_begin() ? s.begin() : s.single_use();
    auto stub = session_pool_->GetStub(*session);
    auto response = internal::RetryLoop(
        retry_policy_prototype_->clone(), backoff_policy_prototype_->clone(),
        true,
        [&stub](grpc::ClientContext& context,
                spanner_proto::BeginTransactionRequest const& request) {
          return stub->BeginTransaction(context, request);
        },
        begin, __func__);
    if (!response) {
      auto status = std::move(response).status();
      if (internal::IsSessionNotFound(status)) session->set_bad();
      return status;
    }
    s.set_id(response->id());
  }
  request.set_transaction_id(s.id());

  auto stub = session_pool_->GetStub(*session);
  auto response = internal::RetryLoop(
      retry_policy_prototype_->clone(), backoff_policy_prototype_->clone(),
      true,
      [&stub](grpc::ClientContext& context,
              spanner_proto::CommitRequest const& request) {
        return stub->Commit(context, request);
      },
      request, __func__);
  if (!response) {
    auto status = std::move(response).status();
    if (internal::IsSessionNotFound(status)) session->set_bad();
    return status;
  }
  CommitResult r;
  r.commit_timestamp =
      internal::TimestampFromProto(response->commit_timestamp());
  return r;
}

Status ConnectionImpl::RollbackImpl(SessionHolder& session,
                                    spanner_proto::TransactionSelector& s) {
  auto prepare_status = PrepareSession(session);
  if (!prepare_status.ok()) {
    return prepare_status;
  }

  if (s.has_single_use()) {
    return Status(StatusCode::kInvalidArgument,
                  "Cannot rollback a single-use transaction");
  }
  if (s.has_begin()) {
    // There is nothing to rollback if a transaction id has not yet been
    // assigned, so we just succeed without making an RPC.
    return Status();
  }
  spanner_proto::RollbackRequest request;
  request.set_session(session->session_name());
  request.set_transaction_id(s.id());
  auto stub = session_pool_->GetStub(*session);
  auto status = internal::RetryLoop(
      retry_policy_prototype_->clone(), backoff_policy_prototype_->clone(),
      true,
      [&stub](grpc::ClientContext& context,
              spanner_proto::RollbackRequest const& request) {
        return stub->Rollback(context, request);
      },
      request, __func__);
  if (internal::IsSessionNotFound(status)) session->set_bad();
  return status;
}

}  // namespace internal
}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google
