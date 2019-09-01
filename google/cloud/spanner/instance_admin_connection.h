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

#ifndef GOOGLE_CLOUD_CPP_SPANNER_GOOGLE_CLOUD_SPANNER_INSTANCE_ADMIN_CONNECTION_H_
#define GOOGLE_CLOUD_CPP_SPANNER_GOOGLE_CLOUD_SPANNER_INSTANCE_ADMIN_CONNECTION_H_

#include "google/cloud/spanner/connection_options.h"
#include "google/cloud/status_or.h"
#include <google/spanner/admin/instance/v1/spanner_instance_admin.grpc.pb.h>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {
/**
 * A connection to the Cloud Spanner instance administration service.
 *
 * This interface defines pure-virtual methods for each of the user-facing
 * overload sets in `InstanceAdminClient`.  This allows users to inject custom
 * behavior (e.g., with a Google Mock object) in a `InstanceAdminClient` object
 * for use in their own tests.
 *
 * To create a concrete instance that connects you to a real Cloud Spanner
 * instance administration service, see `MakeInstanceAdminConnection()`.
 */

class InstanceAdminConnection {
 public:
  virtual ~InstanceAdminConnection() = 0;

  struct GetInstanceParams {
    std::string project_id;
    std::string instance_id;
  };
  virtual StatusOr<google::spanner::admin::instance::v1::Instance> GetInstance(
      GetInstanceParams) = 0;
};

/**
 * Returns an InstanceAdminConnection object that can be used for interacting
 * with Cloud Spanner's admin APIs.
 *
 * The returned connection object should not be used directly, rather it should
 * be given to a `InstanceAdminClient` instance.
 *
 * @see `InstanceAdminConnection`
 *
 * @param options (optional) configure the `InstanceAdminConnection` created by
 *     this function.
 */
std::shared_ptr<InstanceAdminConnection> MakeInstanceAdminConnection(
    ConnectionOptions const& options = ConnectionOptions());

}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_SPANNER_GOOGLE_CLOUD_SPANNER_INSTANCE_ADMIN_CONNECTION_H_
