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

::google::spanner::v1::KeySet ToProto(KeySet keyset) {
  ::google::spanner::v1::KeySet proto;
  proto.set_all(keyset.IsAll());

  auto make_key = [](KeySet::ValueRow const& key) {
    google::protobuf::ListValue lv;
    for (Value const& v : key.column_values()) {
      std::pair<google::spanner::v1::Type, google::protobuf::Value> p =
          ToProto(v);
      *lv.add_values() = std::move(p.second);
    }
    return lv;
  };

  for (KeySet::ValueRow const& key : keyset.key_values_) {
    *proto.add_keys() = make_key(key);
  }

  for (KeySet::ValueKeyRange const& range : keyset.key_ranges_) {
    google::spanner::v1::KeyRange kr;
    auto const& start = range.start();
    auto const& end = range.end();
    if (start.mode() == KeySet::ValueBound::Mode::MODE_CLOSED) {
      *kr.mutable_start_closed() = make_key(start.key());
    } else {
      *kr.mutable_start_open() = make_key(start.key());
    }

    if (end.mode() == KeySet::ValueBound::Mode::MODE_CLOSED) {
      *kr.mutable_end_closed() = make_key(end.key());
    } else {
      *kr.mutable_end_open() = make_key(end.key());
    }

    *proto.add_ranges() = std::move(kr);
  }

  return proto;
}

}  // namespace internal
}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google
