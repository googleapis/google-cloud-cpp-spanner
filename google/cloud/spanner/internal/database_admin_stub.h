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

#ifndef GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_INTERNAL_DATABASE_ADMIN_STUB_H_
#define GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_INTERNAL_DATABASE_ADMIN_STUB_H_

#include "google/cloud/spanner/client_options.h"
#include "google/cloud/status_or.h"
#include <google/spanner/admin/database/v1/spanner_database_admin.grpc.pb.h>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {
namespace internal {
/**
 * Defines the low-level interface for database administration RPCs.
 */
class DatabaseAdminStub {
 public:
  virtual ~DatabaseAdminStub() = 0;

  /// Start the long-running operation to create a new Cloud Spanner database.
  virtual StatusOr<google::longrunning::Operation> CreateDatabase(
      grpc::ClientContext& client_context,
      google::spanner::admin::database::v1::CreateDatabaseRequest const&
          request);

  /// Drop an existing Cloud Spanner database.
  virtual Status DropDatabase(
      grpc::ClientContext& client_context,
      google::spanner::admin::database::v1::DropDatabaseRequest const& request);

  /// Poll a long-running operation.
  virtual StatusOr<google::longrunning::Operation> GetOperation(
      grpc::ClientContext& client_context,
      google::longrunning::GetOperationRequest const& request);
};

/**
 * Constructs a simple `DatabaseAdminStub`,
 *
 * This stub does not create a channel pool, or retry operations.
 */
std::shared_ptr<DatabaseAdminStub> CreateDefaultDatabaseAdminStub(
    ClientOptions const& options);

}  // namespace internal
}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_INTERNAL_DATABASE_ADMIN_STUB_H_
