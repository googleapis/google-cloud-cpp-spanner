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

#include "google/cloud/spanner/internal/keys.h"
#include "google/cloud/spanner/value.h"
#include <google/spanner/v1/keys.pb.h>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {
namespace internal {
::google::spanner::v1::KeySet ToProto(KeySet const& keyset) {
  ::google::spanner::v1::KeySet proto;
  proto.set_all(keyset.IsAll());

  for (KeySet::ValueRow const& key : keyset.key_values_) {
     google::protobuf::ListValue lv;
     auto mlv = lv.mutable_values();
     for (Value const& v : key.column_values()) {
       std::pair<google::spanner::v1::Type, google::protobuf::Value> p = ToProto(v);
       mlv->Add(std::move(p.second));
     }
     proto.mutable_keys()->Add(std::move(lv));
  }
  return proto;
}

}  // namespace internal
}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google
