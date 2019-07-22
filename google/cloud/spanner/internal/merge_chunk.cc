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

#include "google/cloud/spanner/internal/merge_chunk.h"

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {
namespace internal {

Status MergeChunk(google::protobuf::Value& value,
                  google::protobuf::Value&& chunk) {
  switch (value.kind_case()) {
    case google::protobuf::Value::kBoolValue:
    case google::protobuf::Value::kNumberValue:
    case google::protobuf::Value::kNullValue:
      return Status(StatusCode::kInvalidArgument,
                    "Cannot merge values of type bool/number/null");

    case google::protobuf::Value::kStructValue: {
      if (chunk.kind_case() != google::protobuf::Value::kStructValue) {
        return Status(StatusCode::kInvalidArgument,
                      "Cannot merge mismatched types");
      }
      return Status(StatusCode::kInternal,
                    "Spanner should never return struct_value");
    }

    case google::protobuf::Value::kStringValue: {
      if (chunk.kind_case() != google::protobuf::Value::kStringValue) {
        return Status(StatusCode::kInvalidArgument,
                      "Cannot merge mismatched types");
      }
      *value.mutable_string_value() += chunk.string_value();
      return Status();
    }

    case google::protobuf::Value::kListValue: {
      if (chunk.kind_case() != google::protobuf::Value::kListValue) {
        return Status(StatusCode::kInvalidArgument,
                      "Cannot merge mismatched types");
      }

      auto& value_list = *value.mutable_list_value()->mutable_values();
      auto& chunk_list = *chunk.mutable_list_value()->mutable_values();

      // Check if we need to recursively merge the last element in value_list
      // with the first element in chunk_list.
      if (!value_list.empty()) {
        auto& last = value_list[value_list.size() - 1];
        if (last.kind_case() == google::protobuf::Value::kStringValue ||
            last.kind_case() == google::protobuf::Value::kListValue) {
          if (chunk_list.empty()) {
            return Status(StatusCode::kInternal,
                          "Cannot merge with empty chunk");
          }
          auto& first = chunk_list[0];
          auto const status = MergeChunk(last, std::move(first));
          if (!status.ok()) return status;
          chunk_list.erase(chunk_list.begin());
        }
      }
      // Moves all the remaining elements over.
      for (auto& e : chunk_list) {
        *value.mutable_list_value()->add_values() = std::move(e);
      }

      return Status();
    }

    default:
      return Status(StatusCode::kUnknown, "Unknown Value type");
  }
  return Status(StatusCode::kUnknown, "Unknown Value type");
}

}  // namespace internal
}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google
