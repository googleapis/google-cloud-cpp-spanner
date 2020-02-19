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

#include "google/cloud/spanner/internal/instance_endpoint.h"
#include "google/cloud/internal/getenv.h"
#include "google/cloud/log.h"
#include "google/cloud/status.h"
#include <map>
#include <mutex>
#include <utility>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {
namespace internal {

namespace {

constexpr auto kEnvVar = "GOOGLE_CLOUD_SPANNER_ENABLE_RESOURCE_BASED_ROUTING";

// Cached mapping from (instance name, original endpoint) to final endpoint.
struct EndpointCache {
  using map_type = std::map<std::pair<std::string, std::string>, std::string>;
  std::mutex mu;
  map_type endpoints;  // GUARDED_BY(mu_)
};

EndpointCache& endpoint_cache() {
  static auto* const kCache = new EndpointCache;
  return *kCache;
}

}  // namespace

std::string InstanceEndpoint(Instance const& instance,
                             std::string original_endpoint,
                             InstanceAdminConnection& conn) {
  auto env = google::cloud::internal::GetEnv(kEnvVar);
  if (env && !env->empty() && *env != "true") return original_endpoint;

  EndpointCache& cache = endpoint_cache();
  EndpointCache::map_type::value_type value{
      {instance.FullName(), std::move(original_endpoint)}, {}};
  {
    std::unique_lock<std::mutex> lk(cache.mu);
    auto it = cache.endpoints.find(value.first);
    if (it != cache.endpoints.end()) return it->second;
  }

  InstanceAdminConnection::GetInstanceParams params;
  params.instance_name = value.first.first;
  params.field_paths.emplace_back("endpoint_uris");
  auto response = conn.GetInstance(params);
  if (response && response->endpoint_uris_size() != 0) {
    // While the protobuf documentation says, "If multiple endpoints are
    // present, client may establish connections using any of the given URIs,"
    // the client-library specification says we "should use the first returned
    // endpoint."
    value.second = std::move(*response->mutable_endpoint_uris(0));
  } else {
    value.second = value.first.second;  // fallback to original_endpoint
  }

  if (!response && response.status().code() == StatusCode::kPermissionDenied) {
    GCP_LOG(ERROR)
        << "The client library attempted to connect to an endpoint closer "
           "to your Cloud Spanner data but was unable to do so. The client "
           "library will fall back and route requests to the endpoint given "
           "in the connection options ("
        << value.second
        << "), which may result in increased latency. We recommend including "
           "the scope https://www.googleapis.com/auth/spanner.admin so that "
           "the client library can get an instance-specific endpoint and "
           "efficiently route requests.";
  }

  std::unique_lock<std::mutex> lk(cache.mu);
  return cache.endpoints.insert(std::move(value)).first->second;
}

void ClearInstanceEndpointCache() {
  EndpointCache& cache = endpoint_cache();
  std::unique_lock<std::mutex> lk(cache.mu);
  cache.endpoints.clear();
}

}  // namespace internal
}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google
