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
#include "google/cloud/spanner/internal/partial_result_set_reader.h"
#include "google/cloud/spanner/internal/time.h"
#include <google/spanner/v1/spanner.pb.h>
#include <atomic>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {
namespace internal {

namespace spanner_proto = ::google::spanner::v1;

StatusOr<ResultSet> ConnectionImpl::Read(ReadParams rp) {
  return internal::Visit(std::move(rp.transaction),
                         [this, &rp](spanner_proto::TransactionSelector& s) {
                           return Read(s, std::move(rp));
                         });
}

StatusOr<ResultSet> ConnectionImpl::ExecuteSql(ExecuteSqlParams esp) {
  return internal::Visit(std::move(esp.transaction),
                         [this, &esp](spanner_proto::TransactionSelector& s) {
                           return ExecuteSql(s, esp);
                         });
}

StatusOr<CommitResult> ConnectionImpl::Commit(Connection::CommitParams cp) {
  return internal::Visit(
      std::move(cp.transaction),
      [this, &cp](spanner_proto::TransactionSelector& s, std::int64_t) {
        return this->Commit(s, std::move(cp));
      });
}

Status ConnectionImpl::Rollback(Connection::RollbackParams rp) {
  return internal::Visit(std::move(rp.transaction),
                         [this](spanner_proto::TransactionSelector& s,
                                std::int64_t) { return this->Rollback(s); });
}

StatusOr<ConnectionImpl::SessionHolder> ConnectionImpl::GetSession() {
  if (!sessions_.empty()) {
    std::string session = sessions_.back();
    sessions_.pop_back();
    return SessionHolder(std::move(session), this);
  }
  grpc::ClientContext context;
  spanner_proto::CreateSessionRequest request;
  request.set_database(database_);
  auto response = stub_->CreateSession(context, request);
  if (!response) {
    return response.status();
  }
  return SessionHolder(std::move(*response->mutable_name()), this);
}

StatusOr<ResultSet> ConnectionImpl::Read(spanner_proto::TransactionSelector& s,
                                         ReadParams rp) {
  auto session = GetSession();
  if (!session) {
    return std::move(session).status();
  }
  spanner_proto::ReadRequest request;
  request.set_session(session->session_name());
  *request.mutable_transaction() = s;
  request.set_table(std::move(rp.table));
  request.set_index(std::move(rp.read_options.index_name));
  for (auto&& column : rp.columns) {
    request.add_columns(std::move(column));
  }
  spanner_proto::KeySet* key_set = request.mutable_key_set();
  // TODO(#202) update this when the final KeySet is implemented.
  if (rp.keys.IsAll()) {
    key_set->set_all(true);
  }
  request.set_limit(rp.read_options.limit);

  grpc::ClientContext context;
  auto reader = internal::PartialResultSetReader::Create(
      stub_->StreamingRead(context, request));
  if (!reader.ok()) {
    return std::move(reader).status();
  }
  if (s.has_begin()) {
    auto metadata = (*reader)->Metadata();
    if (!metadata || metadata->transaction().id().empty()) {
      return Status(StatusCode::kInternal,
                    "begin transaction requested but no transaction returned");
    }
    s.set_id(metadata->transaction().id());
  }
  return ResultSet(std::move(*reader));
}

StatusOr<ResultSet> ConnectionImpl::ExecuteSql(
    spanner_proto::TransactionSelector& s, ExecuteSqlParams const& esp) {
  auto session = GetSession();
  if (!session) {
    return std::move(session).status();
  }
  spanner_proto::ExecuteSqlRequest request;
  request.set_session(session->session_name());
  *request.mutable_transaction() = s;
  request.set_sql(esp.statement.sql());
  for (auto const& param : esp.statement.params()) {
    auto type_and_value = internal::ToProto(param.second);
    request.mutable_params()->mutable_fields()->insert(
        {param.first, type_and_value.second});
    request.mutable_param_types()->insert({param.first, type_and_value.first});
  }
  // TODO(#279) implement proper sequence number management; for now just
  // assign a globally increasing sequence number to all requests.
  static std::atomic<std::int64_t> seqno(1);
  request.set_seqno(seqno++);

  grpc::ClientContext context;
  auto reader = internal::PartialResultSetReader::Create(
      stub_->ExecuteStreamingSql(context, request));
  if (!reader.ok()) {
    return std::move(reader).status();
  }
  if (s.has_begin()) {
    auto metadata = (*reader)->Metadata();
    if (!metadata || metadata->transaction().id().empty()) {
      return Status(StatusCode::kInternal,
                    "begin transaction requested but no transaction returned");
    }
    s.set_id(metadata->transaction().id());
  }
  return ResultSet(std::move(*reader));
}

StatusOr<CommitResult> ConnectionImpl::Commit(
    spanner_proto::TransactionSelector& s, CommitParams cp) {
  auto session = GetSession();
  if (!session) {
    return std::move(session).status();
  }
  spanner_proto::CommitRequest request;
  request.set_session(session->session_name());
  for (auto&& m : cp.mutations) {
    *request.add_mutations() = std::move(m).as_proto();
  }
  if (s.has_single_use()) {
    *request.mutable_single_use_transaction() = s.single_use();
  } else if (s.has_begin()) {
    *request.mutable_single_use_transaction() = s.begin();
  } else {
    request.set_transaction_id(s.id());
  }
  grpc::ClientContext context;
  auto response = stub_->Commit(context, request);
  if (!response) {
    return std::move(response).status();
  }
  CommitResult r;
  r.commit_timestamp = internal::FromProto(response->commit_timestamp());
  return r;
}

Status ConnectionImpl::Rollback(spanner_proto::TransactionSelector& s) {
  auto session = GetSession();
  if (!session) {
    return std::move(session).status();
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
  grpc::ClientContext context;
  return stub_->Rollback(context, request);
}

}  // namespace internal
}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google
