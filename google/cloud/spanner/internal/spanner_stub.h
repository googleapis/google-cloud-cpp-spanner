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

#ifndef GOOGLE_CLOUD_CPP_SPANNER_GOOGLE_CLOUD_SPANNER_INTERNAL_SPANNER_STUB_H_
#define GOOGLE_CLOUD_CPP_SPANNER_GOOGLE_CLOUD_SPANNER_INTERNAL_SPANNER_STUB_H

#include "google/cloud/spanner/version.h"
#include "google/cloud/status.h"
#include "google/cloud/status_or.h"
#include <google/spanner/v1/spanner.grpc.pb.h>
#include <google/spanner/v1/spanner.pb.h>
#include <grpcpp/grpcpp.h>
#include <memory>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {
namespace internal {

/**
 * SpannerStub is a thin stub layer over the Cloud Spanner API to avoid
 * exposing the underlying transport stub (gRPC, etc.) directly.
 *
 * The API is defined in:
 * https://github.com/googleapis/googleapis/blob/master/google/spanner/v1/spanner.proto
 */
class SpannerStub {
 public:
  virtual ~SpannerStub() = default;

  /// Not copyable or moveable.
  SpannerStub(SpannerStub const&) = delete;
  SpannerStub(SpannerStub&&) = delete;
  SpannerStub& operator=(SpannerStub const&) = delete;
  SpannerStub& operator=(SpannerStub&&) = delete;

  virtual StatusOr<google::spanner::v1::Session> CreateSession(
      grpc::ClientContext& client_context,
      google::spanner::v1::CreateSessionRequest const& request) = 0;
  virtual StatusOr<google::spanner::v1::Session> GetSession(
      grpc::ClientContext& client_context,
      google::spanner::v1::GetSessionRequest const& request) = 0;
  virtual StatusOr<google::spanner::v1::ListSessionsResponse> ListSessions(
      grpc::ClientContext& client_context,
      google::spanner::v1::ListSessionsRequest const& request) = 0;
  virtual Status DeleteSession(
      grpc::ClientContext& client_context,
      google::spanner::v1::DeleteSessionRequest const& request) = 0;
  virtual StatusOr<google::spanner::v1::ResultSet> ExecuteSql(
      grpc::ClientContext& client_context,
      google::spanner::v1::ExecuteSqlRequest const& request) = 0;
  virtual std::unique_ptr<
      grpc::ClientReaderInterface<google::spanner::v1::PartialResultSet>>
  ExecuteStreamingSql(
      grpc::ClientContext& client_context,
      google::spanner::v1::ExecuteSqlRequest const& request) = 0;
  virtual StatusOr<google::spanner::v1::ExecuteBatchDmlResponse>
  ExecuteBatchDml(
      grpc::ClientContext& client_context,
      google::spanner::v1::ExecuteBatchDmlRequest const& request) = 0;
  virtual StatusOr<google::spanner::v1::ResultSet> Read(
      grpc::ClientContext& client_context,
      google::spanner::v1::ReadRequest const& request) = 0;
  virtual std::unique_ptr<
      grpc::ClientReaderInterface<google::spanner::v1::PartialResultSet>>
  StreamingRead(grpc::ClientContext& client_context,
                google::spanner::v1::ReadRequest const& request) = 0;
  virtual StatusOr<google::spanner::v1::Transaction> BeginTransaction(
      grpc::ClientContext& client_context,
      google::spanner::v1::BeginTransactionRequest const& request) = 0;
  virtual StatusOr<google::spanner::v1::CommitResponse> Commit(
      grpc::ClientContext& client_context,
      google::spanner::v1::CommitRequest const& request) = 0;
  virtual Status Rollback(
      grpc::ClientContext& client_context,
      google::spanner::v1::RollbackRequest const& request) = 0;
  virtual StatusOr<google::spanner::v1::PartitionResponse> PartitionQuery(
      grpc::ClientContext& client_context,
      google::spanner::v1::PartitionQueryRequest const& request) = 0;
  virtual StatusOr<google::spanner::v1::PartitionResponse> PartitionRead(
      grpc::ClientContext& client_context,
      google::spanner::v1::PartitionReadRequest const& request) = 0;

 protected:
  SpannerStub() = default;
};

/**
 * Factory method to create a default stub.
 */
std::shared_ptr<SpannerStub> CreateDefaultSpannerStub(
    std::shared_ptr<grpc::ChannelCredentials> const& creds,
    std::string const& endpoint);

}  // namespace internal
}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_SPANNER_GOOGLE_CLOUD_SPANNER_INTERNAL_SPANNER_STUB_H_
