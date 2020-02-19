// Copyright 2020 Google LLC
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

#ifndef GOOGLE_CLOUD_CPP_SPANNER_GOOGLE_CLOUD_SPANNER_INTERNAL_INSTANCE_ENDPOINT_H
#define GOOGLE_CLOUD_CPP_SPANNER_GOOGLE_CLOUD_SPANNER_INTERNAL_INSTANCE_ENDPOINT_H

#include "google/cloud/spanner/instance.h"
#include "google/cloud/spanner/instance_admin_connection.h"
#include <string>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {
namespace internal {

/**
 * Use the `InstanceAdminConnection::GetInstance()` interface to discover any
 * instance-specific endpoint. Such an endpoint is intended to optimize the
 * network routing between the client and the instance's serving resources.
 * For example, instances located in a specific cloud region (or multi region)
 * could have a region-specific endpoint.
 */
std::string InstanceEndpoint(Instance const& instance,
                             std::string original_endpoint,
                             InstanceAdminConnection& conn);

/**
 * Clear the cache used by `InstanceEndpoint()`. For testing only.
 */
void ClearInstanceEndpointCache();

}  // namespace internal
}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_SPANNER_GOOGLE_CLOUD_SPANNER_INTERNAL_INSTANCE_ENDPOINT_H
