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
#include <ostream>
#include <string>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {
/**
 * This class represents a type-safe, nullable Spanner value. It's conceptually
 * similar to a `std::any` except the only allowed types are those supported by
 * Spanner, and a "null" value (similar to a `std::any` without a value) still
 * has an associated type that can be queried with `Value::is<T>()`. The
 * supported types are shown in the following table along with how they map to
 * the Spanner types (https://cloud.google.com/spanner/docs/data-types):
 *
 *   Spanner Type | C++ Type
 *   -----------------------
 *   BOOL         | bool
 *   INT64        | std::int64_t
 *   FLOAT64      | double
 *   STRING       | std::string
 *
 * This is a regular C++ value type with support for copy, move, equality, etc,
 * but there is no default constructor because there is no default type.
 * Callers may create instances by passing any of the supported values (shown
 * in the table above) to the constructor. "Null" values are created by passing
 * a disengaged (i.e., no value) `google::cloud::optional<T>` to the
 * constructor. Passing a `google::cloud::optional<T>` that contains a value to
 * the constructor creates a non-null Value and is equivalent to passing the
 * value without the optional.
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

  // Constructs a non-null instance with the specified value and type.
  explicit Value(bool v);
  explicit Value(std::int64_t v);
  explicit Value(double v);
  explicit Value(std::string v);

  // Constructs a possibly null instance from the optional value and type.
  template <typename T>
  explicit Value(google::cloud::optional<T> opt) {
    if (opt) {
      *this = Value(*opt);
    } else {
      *this = Value(T{});
      value_.set_null_value(google::protobuf::NullValue::NULL_VALUE);
    }
  }

  friend bool operator==(Value a, Value b);
  friend bool operator!=(Value a, Value b) { return !(a == b); }
  friend std::ostream& operator<<(std::ostream& os, Value v);

  // Returns true if there is no contained value.
  bool is_null() const;

  // Returns true if the contained value is of the specified type `T`. All
  // Value instances have some type, even null values. Since all Values are
  // potentially nullable, `v.is<T>()` will return true if and only if
  // `v.is<google::cloud::optional<T>>()` returns true.
  //
  // Example:
  //
  //   spanner::Value v{true};
  //   assert(v.is<bool>());
  //   assert(v.is<google::cloud::optional<bool>>());
  //
  template <typename T>
  bool is() const {
    return IsType(T{});
  }

  // Returns the contained value if the specified type `T` matches the Value's
  // type and if the contained value is not null. Otherwise, a default
  // constructed `T` is returned.
  //
  // If the requested type is specified as `google::cloud::optional<T>`, then
  // the returned optional will contain the value if it was not null,
  // otherwise, the optional will contain no value.
  //
  // Example:
  //
  //   spanner::Value v{true};
  //   assert(true == v.get<bool>());
  //   assert(true == *v.get<google::cloud::optional<bool>>());
  //
  //   // Now using a "null" std::int64_t
  //   v = spanner::Value(google::cloud::optional<std::int64_t>{});
  //   assert(v.is_null());
  //   assert(!v.get<google::cloud::optional<std::int64_t>());
  //
  template <typename T>
  T get() const {
    return GetValue(T{});
  }

 private:
  // Tag-dispatched function overloads. The arugment type is important, the
  // value is ignored.
  bool IsType(bool) const;
  bool IsType(std::int64_t) const;
  bool IsType(double) const;
  bool IsType(std::string) const;
  template <typename T>
  bool IsType(google::cloud::optional<T> opt) const {
    return IsType(T{});
  }

  // Tag-dispatched function overloads. The arugment type is important, the
  // value is ignored.
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
