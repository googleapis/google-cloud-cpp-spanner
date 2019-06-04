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
#include "google/cloud/log.h"
#include <google/protobuf/util/field_comparator.h>
#include <google/protobuf/util/message_differencer.h>
#include <cmath>
#include <ios>
#include <string>

// Implementation note: See
// https://github.com/googleapis/googleapis/blob/master/google/spanner/v1/type.proto
// for details about how Spanner types should be encoded.

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {
namespace {

template <typename T>
Status CheckValidity(Value const& v) {
  if (!v.is<T>()) return Status(StatusCode::kInvalidArgument, "wrong type");
  if (v.is_null()) return Status(StatusCode::kInvalidArgument, "null value");
  return {};  // OK status
}

// A custom protobuf comparator to make Spanner-encoded FLOAT64 "NaN" values
// compare not equal to each other. This is the behavior required by IEEE 754.
class CustomComparator : public google::protobuf::util::DefaultFieldComparator {
 public:
  ComparisonResult Compare(
      const google::protobuf::Message& message_1,
      const google::protobuf::Message& message_2,
      const google::protobuf::FieldDescriptor* field, int index_1, int index_2,
      const google::protobuf::util::FieldContext* field_context) override {
    auto const* const reflection = message_1.GetReflection();
    // Spanner requires that FLOAT64 values (i.e., double) that represent nan
    // are stored as the string "NaN". But IEEE 754 requires that NaN compares
    // not equal to anything, including itself.
    if (field->name() == "string_value") {
      if (reflection->GetString(message_1, field) == "NaN") {
        return DIFFERENT;
      }
    }
    return DefaultFieldComparator::Compare(message_1, message_2, field, index_1,
                                           index_2, field_context);
  }
};

}  // namespace

Value::Value(bool v) {
  type_.set_code(google::spanner::v1::TypeCode::BOOL);
  value_.set_bool_value(v);
}

Value::Value(std::int64_t v) {
  type_.set_code(google::spanner::v1::TypeCode::INT64);
  value_.set_string_value(std::to_string(v));
}

Value::Value(double v) {
  type_.set_code(google::spanner::v1::TypeCode::FLOAT64);
  if (std::isnan(v)) {
    value_.set_string_value("NaN");
  } else if (std::isinf(v)) {
    value_.set_string_value(v < 0 ? "-Infinity" : "Infinity");
  } else {
    value_.set_number_value(v);
  }
}

Value::Value(std::string v) {
  type_.set_code(google::spanner::v1::TypeCode::STRING);
  value_.set_string_value(std::move(v));
}

bool operator==(Value a, Value b) {
  google::protobuf::util::MessageDifferencer diff;
  CustomComparator comp;
  diff.set_field_comparator(&comp);
  return diff.Compare(a.type_, b.type_) && diff.Compare(a.value_, b.value_);
}

void PrintTo(Value const& v, std::ostream* os) {
  *os << v.type_.ShortDebugString() << "; " << v.value_.ShortDebugString();
}

bool Value::is_null() const {
  return value_.kind_case() == google::protobuf::Value::kNullValue;
}

//
// Value::GetValue
//

StatusOr<bool> Value::GetValue(bool) const {
  auto const status = CheckValidity<bool>(*this);
  if (!status.ok()) return status;
  return value_.bool_value();
}

StatusOr<std::int64_t> Value::GetValue(std::int64_t) const {
  auto const status = CheckValidity<std::int64_t>(*this);
  if (!status.ok()) return status;
  auto const& s = value_.string_value();
  std::size_t processed = 0;
  long long x = std::stoll(s, &processed, 10);
  if (processed != s.size()) {
    GCP_LOG(FATAL) << "Failed to parse number from string: \"" << s << "\"";
  }
  return {x};
}

StatusOr<double> Value::GetValue(double) const {
  auto const status = CheckValidity<double>(*this);
  if (!status.ok()) return status;
  if (value_.kind_case() == google::protobuf::Value::kStringValue) {
    std::string const& s = value_.string_value();
    auto const inf = std::numeric_limits<double>::infinity();
    if (s == "-Infinity") return -inf;
    if (s == "Infinity") return inf;
    return std::nan(s.c_str());
  }
  return value_.number_value();
}

StatusOr<std::string> Value::GetValue(std::string) const {
  auto const status = CheckValidity<std::string>(*this);
  if (!status.ok()) return status;
  return value_.string_value();
}

}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google
