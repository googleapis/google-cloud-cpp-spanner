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

#ifndef GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_VALUE_H_
#define GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_VALUE_H_

#include "google/cloud/optional.h"
#include "google/cloud/spanner/version.h"
#include <google/protobuf/struct.pb.h>
#include <google/spanner/v1/type.pb.h>
#include <string>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {
/**
 * This class represents a type-safe, nullable Spanner value. The available
 * Spanner types are shown at https://cloud.google.com/spanner/docs/data-types,
 * and the following table shows how they map to C++ types.
 *
 *   Spanner Type | C++ Type
 *   -----------------------
 *   BOOL         | bool
 *   INT64        | std::int64_t
 *   FLOAT64      | double
 *   STRING       | std::string
 *
 * This is a regular C++ value type with support for copy, move, equality, etc,
 * but there is no default constructor. This is a type-erased class so it can
 * be stored in standard C++ containers requiring homogeneity. Callers may
 * create instances by passing any of the supported values (shown in the table
 * above) to the constructor. To consruct a "null" value of a particular type,
 * pass a disengaged google::cloud::optional<T> to the Value constructor.
 *
 * Example with a non-null value:
 *
 *   std::string msg = "hello";
 *   spanner::Value v(msg);
 *   assert(v.is<std::string>());
 *   assert(!v.is_null());
 *   std::string copy = v.get<std::string>();
 *   assert(msg == copy);
 *
 * Example with a null:
 *
 *   spanner::Value v(google::cloud::optional<std::int64_t>{});
 *   assert(v.is<std::int64_t>());
 *   assert(v.is_null());
 */
class Value {
 public:
  Value() = delete;

  // Copy and move.
  Value(Value const&) = default;
  Value(Value&&) = default;
  Value& operator=(Value const&) = default;
  Value& operator=(Value&&) = default;

  // Value constructors.
  explicit Value(bool v);
  explicit Value(std::int64_t v);
  explicit Value(double v);
  explicit Value(std::string v);
  template <typename T>
  explicit Value(google::cloud::optional<T> opt) {
    if (opt) {
      *this = Value(*opt);
    } else {
      *this = Value(T{});
      value_.set_null_value(google::protobuf::NullValue::NULL_VALUE);
    }
  }

  bool is_null() const;

  template <typename T>
  bool is() const {
    return IsType(T{});
  }

  template <typename T>
  T get() const {
    return GetValue(T{});
  }

  friend bool operator==(Value a, Value b);
  friend bool operator!=(Value a, Value b) { return !(a == b); }

 private:
  bool IsType(bool) const;
  bool IsType(std::int64_t) const;
  bool IsType(double) const;
  bool IsType(std::string) const;
  template <typename T>
  bool IsType(google::cloud::optional<T> opt) const {
    return IsType(T{});
  }

  bool GetValue(bool) const;
  std::int64_t GetValue(std::int64_t) const;
  double GetValue(double) const;
  std::string GetValue(std::string) const;
  template <typename T>
  google::cloud::optional<T> GetValue(google::cloud::optional<T> opt) const {
    if (!is_null()) return GetValue(T{});
    return {};
  }

  google::spanner::v1::Type type_;
  google::protobuf::Value value_;
};

}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_VALUE_H_
