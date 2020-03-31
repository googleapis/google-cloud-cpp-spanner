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

#ifndef GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_QUERY_OPTIONS_H
#define GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_QUERY_OPTIONS_H

#include "google/cloud/spanner/version.h"
#include "google/cloud/optional.h"
#include <string>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {

/**
 * These QueryOptions allow users to configure features about how their SQL
 * queries executes on the server.
 */
class QueryOptions {
 public:
  QueryOptions() = default;
  QueryOptions(QueryOptions const&) = default;
  QueryOptions& operator=(QueryOptions const&) = default;
  QueryOptions(QueryOptions&&) = default;
  QueryOptions& operator=(QueryOptions&&) = default;

  /// Returns the optimizer version
  optional<std::string> const& optimizer_version() const {
    return optimizer_version_;
  }

  /**
   * Sets the optimizerion version to the specified integer string. Setting to
   * the empty string will use the database default. Use the string "latest" to
   * use the latest available optimizer version.
   */
  QueryOptions& set_optimizer_version(optional<std::string> version) {
    optimizer_version_ = std::move(version);
    return *this;
  }

  friend bool operator==(QueryOptions const& a, QueryOptions const& b) {
    return a.optimizer_version_ == b.optimizer_version_;
  }

  friend bool operator!=(QueryOptions const& a, QueryOptions const& b) {
    return !(a == b);
  }

 private:
  optional<std::string> optimizer_version_;
};

}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_QUERY_OPTIONS_H
