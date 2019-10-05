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

#ifndef GOOGLE_CLOUD_CPP_SPANNER_GOOGLE_CLOUD_SPANNER_CREATE_INSTANCE_REQUEST_BUILDER_H_
#define GOOGLE_CLOUD_CPP_SPANNER_GOOGLE_CLOUD_SPANNER_CREATE_INSTANCE_REQUEST_BUILDER_H_

#include "google/cloud/spanner/instance.h"
#include "google/cloud/spanner/version.h"
#include <google/spanner/admin/instance/v1/spanner_instance_admin.pb.h>
#include <map>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {

/**
 * CreateInstanceRequestBuilder is a builder class for
 * `google::spanner::admin::instance::v1::CreateInstanceRequest`
 *
 * This is useful when calling InstanceAdminClient::CreateInstance()
 * function. If you see an error message like "error: no member named 'Build'",
 * this means you don't set a mandatory field.
 *
 * @par Example
 * @snippet samples.cc create-instance
 */
class CreateInstanceRequestBuilder {
 public:
  // Copy and move.
  CreateInstanceRequestBuilder(CreateInstanceRequestBuilder const&) = default;
  CreateInstanceRequestBuilder(CreateInstanceRequestBuilder&&) = default;
  CreateInstanceRequestBuilder& operator=(CreateInstanceRequestBuilder const&) =
      default;
  CreateInstanceRequestBuilder& operator=(CreateInstanceRequestBuilder&&) =
      default;

  /**
   * Only constructor that accepts Instance.
   */
  explicit CreateInstanceRequestBuilder(Instance const& in) {
    request_.set_parent("projects/" + in.project_id());
    request_.set_instance_id(in.instance_id());
    request_.mutable_instance()->set_name(in.FullName());
  }

  class NodeCountSetter;
  class ConfigSetter;
  class Builder;

  NodeCountSetter& SetDisplayName(std::string const& display_name) & {
    request_.mutable_instance()->set_display_name(std::move(display_name));
    return reinterpret_cast<NodeCountSetter&>(*this);
  }

  NodeCountSetter&& SetDisplayName(std::string const& display_name) && {
    request_.mutable_instance()->set_display_name(std::move(display_name));
    return std::move(reinterpret_cast<NodeCountSetter&>(*this));
  }

 protected:
  google::spanner::admin::instance::v1::CreateInstanceRequest request_;
};

class CreateInstanceRequestBuilder::NodeCountSetter
    : public CreateInstanceRequestBuilder {
 public:
  ConfigSetter& SetNodeCount(int node_count) & {
    request_.mutable_instance()->set_node_count(node_count);
    return reinterpret_cast<ConfigSetter&>(*this);
  }
  ConfigSetter&& SetNodeCount(int node_count) && {
    request_.mutable_instance()->set_node_count(node_count);
    return std::move(reinterpret_cast<ConfigSetter&>(*this));
  }
};

class CreateInstanceRequestBuilder::ConfigSetter
    : public CreateInstanceRequestBuilder {
 public:
  Builder& SetConfig(std::string config) & {
    request_.mutable_instance()->set_config(std::move(config));
    return reinterpret_cast<Builder&>(*this);
  }
  Builder&& SetConfig(std::string config) && {
    request_.mutable_instance()->set_config(std::move(config));
    return std::move(reinterpret_cast<Builder&>(*this));
  }
};

class CreateInstanceRequestBuilder::Builder
    : public CreateInstanceRequestBuilder {
 public:
  Builder& SetLabels(std::map<std::string, std::string> const& labels) & {
    for (auto const& pair : labels) {
      request_.mutable_instance()->mutable_labels()->insert(
          {pair.first, pair.second});
    }
    return *this;
  }
  Builder&& SetLabels(std::map<std::string, std::string> const& labels) && {
    for (auto const& pair : labels) {
      request_.mutable_instance()->mutable_labels()->insert(
          {pair.first, pair.second});
    }
    return std::move(*this);
  }
  google::spanner::admin::instance::v1::CreateInstanceRequest& Build() & {
    return request_;
  }
  google::spanner::admin::instance::v1::CreateInstanceRequest&& Build() && {
    return std::move(request_);
  }
};

}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_SPANNER_GOOGLE_CLOUD_SPANNER_CREATE_INSTANCE_REQUEST_BUILDER_H_
