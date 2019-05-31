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

#include "google/cloud/spanner/value.h"
#include <ios>
#include <sstream>
#include <cmath>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {

Value::Value(bool v) {
  type_.set_code(google::spanner::v1::TypeCode::BOOL);
  value_.set_bool_value(v);
}

Value::Value(std::int64_t v) {
  type_.set_code(google::spanner::v1::TypeCode::INT64);
  std::stringstream ss;
  ss << std::dec << v;
  value_.set_string_value(ss.str());
}

Value::Value(double v) {
  type_.set_code(google::spanner::v1::TypeCode::FLOAT64);
  if (std::isnan(v)) {
    value_.set_string_value("NaN");
  } else if (std::isinf(v)) {
    std::string const s = v < 0 ? "-Infinity" : "Infinity";
    value_.set_string_value(s);
  } else {
    value_.set_number_value(v);
  }
}

Value::Value(std::string v) {
  type_.set_code(google::spanner::v1::TypeCode::STRING);
  value_.set_string_value(v);
}

bool operator==(Value a, Value b) {
  auto const& at = a.type_;
  auto const& bt = b.type_;
  auto const& av = a.value_;
  auto const& bv = b.value_;
  if (at.code() != bt.code() || av.kind_case() != bv.kind_case()) return false;
  switch (av.kind_case()) {
    case google::protobuf::Value::kNullValue:
      return true;
    case google::protobuf::Value::kNumberValue:
      return av.number_value() == bv.number_value();
    case google::protobuf::Value::kStringValue:
      return av.string_value() == bv.string_value();
    default:
      return false;
  }
}

//
// Value::is_null
//

bool Value::is_null() const {
  return value_.kind_case() == google::protobuf::Value::kNullValue;
}

//
// Value::IsType
//

bool Value::IsType(bool) const {
  return type_.code() == google::spanner::v1::TypeCode::BOOL;
}

bool Value::IsType(std::int64_t) const {
  return type_.code() == google::spanner::v1::TypeCode::INT64;
}

bool Value::IsType(double) const {
  return type_.code() == google::spanner::v1::TypeCode::FLOAT64;
}

bool Value::IsType(std::string) const {
  return type_.code() == google::spanner::v1::TypeCode::STRING;
}

//
// Value::GetValue
//

bool Value::GetValue(bool) const {
  if (is_null()) return {};
  return value_.bool_value();
}

std::int64_t Value::GetValue(std::int64_t) const {
  if (is_null()) return {};
  std::int64_t n;
  std::istringstream(value_.string_value()) >> std::dec >> n;
  return n;
}

double Value::GetValue(double) const {
  if (is_null()) return {};
  if (value_.kind_case() == google::protobuf::Value::kStringValue) {
    std::string const& s = value_.string_value();
    auto const inf = std::numeric_limits<double>::infinity();
    if (s == "-Infinity") return -inf;
    if (s == "Infinity") return inf;
    return std::nan(s.c_str());
  }
  return value_.number_value();
}

std::string Value::GetValue(std::string) const {
  if (is_null()) return {};
  return value_.string_value();
}

}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google
