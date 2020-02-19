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
#include "google/cloud/spanner/backoff_policy.h"
#include "google/cloud/spanner/instance.h"
#include "google/cloud/spanner/instance_admin_connection.h"
#include "google/cloud/spanner/polling_policy.h"
#include "google/cloud/spanner/retry_policy.h"
#include "google/cloud/spanner/testing/mock_instance_admin_stub.h"
#include "google/cloud/internal/setenv.h"
#include "google/cloud/log.h"
#include "google/cloud/testing_util/capture_log_lines_backend.h"
#include "google/cloud/testing_util/environment_variable_restore.h"
#include <gmock/gmock.h>
#include <memory>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {
namespace internal {
namespace {

using ::testing::_;
using ::testing::Contains;
using ::testing::HasSubstr;
using ::testing::UnorderedElementsAre;

// Picking this up from google/cloud/spanner/internal/instance_admin_stub.h
namespace gcsa = ::google::spanner::admin::instance::v1;

// Create a `Connection` suitable for use in tests that continues retrying
// until the retry policy is exhausted. Attempting that with the default
// policies would take too long (30 minutes).
std::shared_ptr<InstanceAdminConnection> MakeLimitedRetryConnection(
    std::shared_ptr<spanner_testing::MockInstanceAdminStub> mock) {
  LimitedErrorCountRetryPolicy retry(/*maximum_failures=*/2);
  ExponentialBackoffPolicy backoff(
      /*initial_delay=*/std::chrono::microseconds(1),
      /*maximum_delay=*/std::chrono::microseconds(1),
      /*scaling=*/2.0);
  GenericPollingPolicy<LimitedErrorCountRetryPolicy> polling(retry, backoff);
  return internal::MakeInstanceAdminConnection(
      std::move(mock), retry.clone(), backoff.clone(), polling.clone());
}

TEST(InstanceEndpoint, SimpleOverride) {
  Instance instance("my_project_id", "my_instance_id");

  auto mock = std::make_shared<spanner_testing::MockInstanceAdminStub>();
  EXPECT_CALL(*mock, GetInstance(_, _))
      .WillOnce([&instance](grpc::ClientContext&,
                            gcsa::GetInstanceRequest const& request) {
        gcsa::Instance response;
        EXPECT_EQ(request.name(), instance.FullName());
        response.set_name(request.name());
        EXPECT_THAT(request.field_mask().paths(),
                    UnorderedElementsAre("endpoint_uris"));
        response.add_endpoint_uris("my.spanner.googleapis.com");
        return response;
      });
  auto conn = MakeLimitedRetryConnection(mock);

  ClearInstanceEndpointCache();
  auto endpoint = InstanceEndpoint(instance, "spanner.googleapis.com", *conn);
  EXPECT_EQ(endpoint, "my.spanner.googleapis.com");
}

TEST(InstanceEndpoint, SimpleOverrideWithRetry) {
  Instance instance("my_project_id", "my_instance_id");

  auto mock = std::make_shared<spanner_testing::MockInstanceAdminStub>();
  EXPECT_CALL(*mock, GetInstance(_, _))
      .WillOnce([&instance](grpc::ClientContext&,
                            gcsa::GetInstanceRequest const& request) {
        EXPECT_EQ(request.name(), instance.FullName());
        EXPECT_THAT(request.field_mask().paths(),
                    UnorderedElementsAre("endpoint_uris"));
        return Status(StatusCode::kUnavailable, "try again");
      })
      .WillOnce([&instance](grpc::ClientContext&,
                            gcsa::GetInstanceRequest const& request) {
        gcsa::Instance response;
        EXPECT_EQ(request.name(), instance.FullName());
        response.set_name(request.name());
        EXPECT_THAT(request.field_mask().paths(),
                    UnorderedElementsAre("endpoint_uris"));
        response.add_endpoint_uris("my.spanner.googleapis.com");
        return response;
      });
  auto conn = MakeLimitedRetryConnection(mock);

  ClearInstanceEndpointCache();
  auto endpoint = InstanceEndpoint(instance, "spanner.googleapis.com", *conn);
  EXPECT_EQ(endpoint, "my.spanner.googleapis.com");
}

TEST(InstanceEndpoint, SimpleNonOverride) {
  Instance instance("my_project_id", "my_instance_id");

  auto mock = std::make_shared<spanner_testing::MockInstanceAdminStub>();
  EXPECT_CALL(*mock, GetInstance(_, _))
      .WillOnce([&instance](grpc::ClientContext&,
                            gcsa::GetInstanceRequest const& request) {
        gcsa::Instance response;
        EXPECT_EQ(request.name(), instance.FullName());
        response.set_name(request.name());
        EXPECT_THAT(request.field_mask().paths(),
                    UnorderedElementsAre("endpoint_uris"));
        // No endpoint_uris added to the GetInstance() response.
        return response;
      });
  auto conn = MakeLimitedRetryConnection(mock);

  ClearInstanceEndpointCache();
  auto endpoint = InstanceEndpoint(instance, "spanner.googleapis.com", *conn);
  EXPECT_EQ(endpoint, "spanner.googleapis.com");
}

TEST(InstanceEndpoint, GetInstanceUnavailable) {
  Instance instance("my_project_id", "my_instance_id");

  auto mock = std::make_shared<spanner_testing::MockInstanceAdminStub>();
  EXPECT_CALL(*mock, GetInstance(_, _))
      .WillRepeatedly([&instance](grpc::ClientContext&,
                                  gcsa::GetInstanceRequest const& request) {
        EXPECT_EQ(request.name(), instance.FullName());
        EXPECT_THAT(request.field_mask().paths(),
                    UnorderedElementsAre("endpoint_uris"));
        return Status(StatusCode::kUnavailable, "try again");
      });
  auto conn = MakeLimitedRetryConnection(mock);

  ClearInstanceEndpointCache();
  auto endpoint = InstanceEndpoint(instance, "spanner.googleapis.com", *conn);
  EXPECT_EQ(endpoint, "spanner.googleapis.com");
}

TEST(InstanceEndpoint, GetInstancePermissionDenied) {
  Instance instance("my_project_id", "my_instance_id");

  auto mock = std::make_shared<spanner_testing::MockInstanceAdminStub>();
  EXPECT_CALL(*mock, GetInstance(_, _))
      .WillRepeatedly([&instance](grpc::ClientContext&,
                                  gcsa::GetInstanceRequest const& request) {
        EXPECT_EQ(request.name(), instance.FullName());
        EXPECT_THAT(request.field_mask().paths(),
                    UnorderedElementsAre("endpoint_uris"));
        return Status(StatusCode::kPermissionDenied, "no way");
      });
  auto conn = MakeLimitedRetryConnection(mock);

  auto log_capture =
      std::make_shared<google::cloud::testing_util::CaptureLogLinesBackend>();
  auto log_capture_id =
      google::cloud::LogSink::Instance().AddBackend(log_capture);

  ClearInstanceEndpointCache();
  auto endpoint = InstanceEndpoint(instance, "spanner.googleapis.com", *conn);
  EXPECT_EQ(endpoint, "spanner.googleapis.com");

  google::cloud::LogSink::Instance().RemoveBackend(log_capture_id);
  EXPECT_THAT(
      log_capture->log_lines,
      Contains(HasSubstr("attempted to connect to an endpoint closer to your "
                         "Cloud Spanner data but was unable to do so")));
}

TEST(InstanceEndpoint, CachedResponse) {
  Instance instance("my_project_id", "my_instance_id");

  auto mock = std::make_shared<spanner_testing::MockInstanceAdminStub>();
  EXPECT_CALL(*mock, GetInstance(_, _))
      .WillOnce([&instance](grpc::ClientContext&,
                            gcsa::GetInstanceRequest const& request) {
        gcsa::Instance response;
        EXPECT_EQ(request.name(), instance.FullName());
        response.set_name(request.name());
        EXPECT_THAT(request.field_mask().paths(),
                    UnorderedElementsAre("endpoint_uris"));
        response.add_endpoint_uris("my.spanner.googleapis.com");
        return response;
      });
  auto conn = MakeLimitedRetryConnection(mock);

  ClearInstanceEndpointCache();
  // No matter how many times we request the endpoint, GetInstance() will only
  // be called once as we cache the result of the first call.
  for (int i = 0; i != 10; ++i) {
    auto endpoint = InstanceEndpoint(instance, "spanner.googleapis.com", *conn);
    EXPECT_EQ(endpoint, "my.spanner.googleapis.com");
  }
}

TEST(InstanceEndpoint, Disabled) {
  Instance instance("my_project_id", "my_instance_id");

  auto mock = std::make_shared<spanner_testing::MockInstanceAdminStub>();
  // No expectations as the InstanceAdmin interface should never be called.
  auto conn = MakeLimitedRetryConnection(mock);

  ClearInstanceEndpointCache();
  auto k_env_var = "GOOGLE_CLOUD_SPANNER_ENABLE_RESOURCE_BASED_ROUTING";
  google::cloud::testing_util::EnvironmentVariableRestore env(k_env_var);
  google::cloud::internal::SetEnv(k_env_var, "false");
  auto endpoint = InstanceEndpoint(instance, "spanner.googleapis.com", *conn);
  EXPECT_EQ(endpoint, "spanner.googleapis.com");
}

}  // namespace
}  // namespace internal
}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google
