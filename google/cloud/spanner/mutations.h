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

#ifndef GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_MUTATIONS_H_
#define GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_MUTATIONS_H_

#include "google/cloud/spanner/internal/tuple_utils.h"
#include "google/cloud/spanner/value.h"
#include "google/cloud/spanner/version.h"
#include <google/spanner/v1/mutation.pb.h>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {

class Mutation {
 public:
  Mutation() = default;

  Mutation(Mutation&&) noexcept = default;
  Mutation& operator=(Mutation&&) noexcept = default;
  Mutation(Mutation const&) = default;
  Mutation& operator=(Mutation const&) = default;

  friend bool operator==(Mutation const& lhs, Mutation const& rhs);
  friend bool operator!=(Mutation const& lhs, Mutation const& rhs) {
    return !(lhs == rhs);
  }

  google::spanner::v1::Mutation as_proto() && { return std::move(m_); }

  template <typename... Ts>
  friend Mutation MakeInsertMutation(Ts&&... row);

 private:
  google::spanner::v1::Mutation m_;
};

// This namespace contains implementation details. It is not part of the public
// API, and subject to change without notice.
namespace internal {
inline void PopulateListValue(google::protobuf::ListValue&) {}

template <typename T, typename... Ts>
void PopulateListValue(google::protobuf::ListValue& lv, T&& head, Ts&&... tail) {
  google::spanner::v1::Type type;
  google::protobuf::Value value;
  std::tie(type, value) = internal::ToProto(Value(std::forward<T>(head)));
  *lv.add_values() = std::move(value);
  PopulateListValue(lv, std::forward<Ts>(tail)...);
}
}  // namespace internal

/// Creates an insert mutation for the values in @p row.
template <typename... Ts>
Mutation MakeInsertMutation(Ts&&... values) {
  Mutation m;
  google::protobuf::ListValue lv;
  internal::PopulateListValue(lv, std::forward<Ts>(values)...);
  *m.m_.mutable_insert()->add_values() = std::move(lv);
  return m;
}

}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_MUTATIONS_H_
