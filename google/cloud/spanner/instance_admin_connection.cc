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
#include "google/cloud/spanner/internal/instance_admin_retry.h"

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {
namespace gcsa = ::google::spanner::admin::instance::v1;

namespace {
class InstanceAdminConnectionImpl : public InstanceAdminConnection {
 public:
  explicit InstanceAdminConnectionImpl(
      std::shared_ptr<internal::InstanceAdminStub> stub)
      : stub_(std::move(stub)) {}
  ~InstanceAdminConnectionImpl() override = default;

  StatusOr<google::spanner::admin::instance::v1::Instance> GetInstance(
      GetInstanceParams gip) override {
    gcsa::GetInstanceRequest request;
    request.set_name("projects/" + gip.project_id + "/instances/" +
                     gip.instance_id);
    grpc::ClientContext context;
    return stub_->GetInstance(context, request);
  }

 private:
  std::shared_ptr<internal::InstanceAdminStub> stub_;
};
}  // namespace

std::shared_ptr<InstanceAdminConnection> MakeInstanceAdminConnection(
    ConnectionOptions const& options) {
  auto stub = internal::CreateDefaultInstanceAdminStub(options);
  stub = std::make_shared<internal::InstanceAdminRetry>(std::move(stub));
  return std::make_shared<InstanceAdminConnectionImpl>(std::move(stub));
}

}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google
