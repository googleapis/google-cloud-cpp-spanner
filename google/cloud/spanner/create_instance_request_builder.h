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
 * CreateInstanceRequestBuilderTemplate turns on the field bit once it is set
 * and carries the bit as the template argument.
 */
template <unsigned CurrentMask>
class CreateInstanceRequestBuilderTemplate {
  struct FieldBits {
    enum {
      DisplayName = (1 << 0),
      NodeCount = (1 << 1),
      Config = (1 << 2),
      Labels = (1 << 3)
    };
  };

 public:
  // Copy and move.
  CreateInstanceRequestBuilderTemplate(
      CreateInstanceRequestBuilderTemplate const&) = default;
  CreateInstanceRequestBuilderTemplate(CreateInstanceRequestBuilderTemplate&&) =
      default;
  CreateInstanceRequestBuilderTemplate& operator=(
      CreateInstanceRequestBuilderTemplate const&) = default;
  CreateInstanceRequestBuilderTemplate& operator=(
      CreateInstanceRequestBuilderTemplate&&) = default;

  /**
   * Constructor that accepts Instance.
   */
  explicit CreateInstanceRequestBuilderTemplate(Instance const& in) {
    request_.set_parent("projects/" + in.project_id());
    request_.set_instance_id(in.instance_id());
    request_.mutable_instance()->set_name(in.FullName());
  }

  CreateInstanceRequestBuilderTemplate<CurrentMask | FieldBits::DisplayName>&
  SetDisplayName(std::string display_name) & {
    request_.mutable_instance()->set_display_name(std::move(display_name));
    return reinterpret_cast<CreateInstanceRequestBuilderTemplate<
        CurrentMask | FieldBits::DisplayName>&>(*this);
  }

  CreateInstanceRequestBuilderTemplate<CurrentMask | FieldBits::DisplayName>&&
  SetDisplayName(std::string display_name) && {
    request_.mutable_instance()->set_display_name(std::move(display_name));
    return std::move(reinterpret_cast<CreateInstanceRequestBuilderTemplate<
                         CurrentMask | FieldBits::DisplayName>&>(*this));
  }

  CreateInstanceRequestBuilderTemplate<CurrentMask | FieldBits::NodeCount>&
  SetNodeCount(int node_count) & {
    request_.mutable_instance()->set_node_count(node_count);
    return reinterpret_cast<CreateInstanceRequestBuilderTemplate<
        CurrentMask | FieldBits::NodeCount>&>(*this);
  }

  CreateInstanceRequestBuilderTemplate<CurrentMask | FieldBits::NodeCount>&&
  SetNodeCount(int node_count) && {
    request_.mutable_instance()->set_node_count(node_count);
    return std::move(reinterpret_cast<CreateInstanceRequestBuilderTemplate<
                         CurrentMask | FieldBits::NodeCount>&>(*this));
  }

  CreateInstanceRequestBuilderTemplate<CurrentMask | FieldBits::Config>&
  SetConfig(std::string config) & {
    request_.mutable_instance()->set_config(std::move(config));
    return reinterpret_cast<
        CreateInstanceRequestBuilderTemplate<CurrentMask | FieldBits::Config>&>(
        *this);
  }

  CreateInstanceRequestBuilderTemplate<CurrentMask | FieldBits::Config>&&
  SetConfig(std::string config) && {
    request_.mutable_instance()->set_config(std::move(config));
    return std::move(reinterpret_cast<CreateInstanceRequestBuilderTemplate<
                         CurrentMask | FieldBits::Config>&>(*this));
  }

  CreateInstanceRequestBuilderTemplate<CurrentMask | FieldBits::Labels>&
  SetLabels(std::map<std::string, std::string> const& labels) & {
    for (auto const& pair : labels) {
      request_.mutable_instance()->mutable_labels()->insert(
          {pair.first, pair.second});
    }
    return reinterpret_cast<
        CreateInstanceRequestBuilderTemplate<CurrentMask | FieldBits::Labels>&>(
        *this);
  }

  CreateInstanceRequestBuilderTemplate<CurrentMask | FieldBits::Labels>&&
  SetLabels(std::map<std::string, std::string> const& labels) && {
    CreateInstanceRequestBuilderTemplate next = *this;
    for (auto const& pair : labels) {
      request_.mutable_instance()->mutable_labels()->insert(
          {pair.first, pair.second});
    }
    return std::move(reinterpret_cast<CreateInstanceRequestBuilderTemplate<
                         CurrentMask | FieldBits::Labels>&>(*this));
  }

  google::spanner::admin::instance::v1::CreateInstanceRequest& Build() & {
    static_assert(
        (CurrentMask &
         (FieldBits::DisplayName | FieldBits::NodeCount | FieldBits::Config)) ==
            (FieldBits::DisplayName | FieldBits::NodeCount | FieldBits::Config),
        "Call SetDisplayName(), SetNodeCount(), and SetConfig() before calling "
        "Build().");
    return request_;
  }
  google::spanner::admin::instance::v1::CreateInstanceRequest&& Build() && {
    static_assert(
        (CurrentMask &
         (FieldBits::DisplayName | FieldBits::NodeCount | FieldBits::Config)) ==
            (FieldBits::DisplayName | FieldBits::NodeCount | FieldBits::Config),
        "Call SetDisplayName(), SetNodeCount(), and SetConfig() before calling "
        "Build().");
    return std::move(request_);
  }

 private:
  google::spanner::admin::instance::v1::CreateInstanceRequest request_;
};

/**
 * CreateInstanceRequestBuilder is a builder class for
 * `google::spanner::admin::instance::v1::CreateInstanceRequest`
 *
 * This is useful when calling InstanceAdminClient::CreateInstance()
 * function. Call SetDiplayName(), SetNodeCount(), and SetConfig() before
 * calling Build().
 *
 * @par Example
 * @snippet samples.cc create-instance
 */
typedef CreateInstanceRequestBuilderTemplate<0> CreateInstanceRequestBuilder;

}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_SPANNER_GOOGLE_CLOUD_SPANNER_CREATE_INSTANCE_REQUEST_BUILDER_H_
