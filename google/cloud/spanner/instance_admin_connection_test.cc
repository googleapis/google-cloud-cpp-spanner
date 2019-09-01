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

#include "google/cloud/spanner/instance_admin_connection.h"
#include "google/cloud/spanner/testing/mock_instance_admin_stub.h"
#include "google/cloud/testing_util/assert_ok.h"
#include <gmock/gmock.h>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {
namespace {

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;

namespace gcsa = ::google::spanner::admin::instance::v1;

TEST(InstanceAdminConnectionTest, GetInstance_Success) {
  std::string const expected_name =
      "projects/test-project/instances/test-instance";

  auto mock = std::make_shared<spanner_testing::MockInstanceAdminStub>();
  EXPECT_CALL(*mock, GetInstance(_, _))
      .WillOnce(
          Invoke([&expected_name](grpc::ClientContext&,
                                  gcsa::GetInstanceRequest const& request) {
            EXPECT_EQ(expected_name, request.name());
            return Status(StatusCode::kUnavailable, "try-again");
          }))
      .WillOnce(Invoke(
          [](grpc::ClientContext&, gcsa::GetInstanceRequest const& request) {
            gcsa::Instance instance;
            instance.set_name(request.name());
            instance.set_config("test-config");
            instance.set_display_name("test display name");
            instance.set_node_count(42);
            instance.set_state(gcsa::Instance::CREATING);
            return instance;
          }));

  auto conn = internal::MakeInstanceAdminConnection(mock, ConnectionOptions());
  auto actual = conn->GetInstance({expected_name});
  EXPECT_STATUS_OK(actual);
  EXPECT_EQ(expected_name, actual->name());
  EXPECT_EQ("test-config", actual->config());
  EXPECT_EQ("test display name", actual->display_name());
  EXPECT_EQ(42, actual->node_count());
  EXPECT_EQ(gcsa::Instance::CREATING, actual->state());
}

}  // namespace
}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google
