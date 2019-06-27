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
#include "google/cloud/spanner/internal/date.h"
#include "google/cloud/spanner/internal/time.h"
#include "google/cloud/log.h"
#include <google/protobuf/util/field_comparator.h>
#include <google/protobuf/util/message_differencer.h>
#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <ios>
#include <string>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {

namespace {

// Compares two sets of Type and Value protos for equality. This method calls
// itself recursively to compare subtypes and subvalues.
bool Equal(google::spanner::v1::Type const& pt1,
           google::protobuf::Value const& pv1,
           google::spanner::v1::Type const& pt2,
           google::protobuf::Value const& pv2) {
  if (pt1.code() != pt2.code()) return false;
  if (pv1.kind_case() != pv2.kind_case()) return false;
  switch (pt1.code()) {
    case google::spanner::v1::TypeCode::BOOL:
      return pv1.bool_value() == pv2.bool_value();
    case google::spanner::v1::TypeCode::INT64:
      return pv1.string_value() == pv2.string_value();
    case google::spanner::v1::TypeCode::FLOAT64:
      // NaN should always compare not equal, even to itself.
      if (pv1.string_value() == "NaN" || pv2.string_value() == "NaN") {
        return false;
      }
      return pv1.string_value() == pv2.string_value() &&
             pv1.number_value() == pv2.number_value();
    case google::spanner::v1::TypeCode::STRING:
      return pv1.string_value() == pv2.string_value();
    case google::spanner::v1::TypeCode::ARRAY: {
      auto const& etype1 = pt1.array_element_type();
      auto const& etype2 = pt2.array_element_type();
      if (etype1.code() != etype2.code()) return false;
      auto const& v1 = pv1.list_value().values();
      auto const& v2 = pv2.list_value().values();
      if (v1.size() != v2.size()) return false;
      for (int i = 0; i < v1.size(); ++i) {
        if (!Equal(etype1, v1.Get(i), etype1, v2.Get(i))) {
          return false;
        }
      }
      return true;
    }
    case google::spanner::v1::TypeCode::STRUCT: {
      auto const& fields1 = pt1.struct_type().fields();
      auto const& fields2 = pt2.struct_type().fields();
      if (fields1.size() != fields2.size()) return false;
      auto const& v1 = pv1.list_value().values();
      auto const& v2 = pv2.list_value().values();
      if (fields1.size() != v1.size() || v1.size() != v2.size()) return false;
      for (int i = 0; i < fields1.size(); ++i) {
        auto const& f1 = fields1.Get(i);
        auto const& f2 = fields2.Get(i);
        if (f1.name() != f2.name()) return false;
        if (!Equal(f1.type(), v1.Get(i), f2.type(), v2.Get(i))) {
          return false;
        }
      }
      return true;
    }
    default:
      return true;
  }
}

}  // namespace

namespace internal {

Value FromProto(google::spanner::v1::Type t, google::protobuf::Value v) {
  return Value(std::move(t), std::move(v));
}

std::pair<google::spanner::v1::Type, google::protobuf::Value> ToProto(Value v) {
  return std::make_pair(std::move(v.type_), std::move(v.value_));
}

}  // namespace internal

bool operator==(Value const& a, Value const& b) {
  return Equal(a.type_, a.value_, b.type_, b.value_);
}

void PrintTo(Value const& v, std::ostream* os) {
  *os << v.type_.ShortDebugString() << "; " << v.value_.ShortDebugString();
}

//
// Value::MakeTypeProto
//

google::spanner::v1::Type Value::MakeTypeProto(bool) {
  google::spanner::v1::Type t;
  t.set_code(google::spanner::v1::TypeCode::BOOL);
  return t;
}

google::spanner::v1::Type Value::MakeTypeProto(std::int64_t) {
  google::spanner::v1::Type t;
  t.set_code(google::spanner::v1::TypeCode::INT64);
  return t;
}

google::spanner::v1::Type Value::MakeTypeProto(double) {
  google::spanner::v1::Type t;
  t.set_code(google::spanner::v1::TypeCode::FLOAT64);
  return t;
}

google::spanner::v1::Type Value::MakeTypeProto(std::string const&) {
  google::spanner::v1::Type t;
  t.set_code(google::spanner::v1::TypeCode::STRING);
  return t;
}

google::spanner::v1::Type Value::MakeTypeProto(time_point) {
  google::spanner::v1::Type t;
  t.set_code(google::spanner::v1::TypeCode::TIMESTAMP);
  return t;
}

google::spanner::v1::Type Value::MakeTypeProto(Date) {
  google::spanner::v1::Type t;
  t.set_code(google::spanner::v1::TypeCode::DATE);
  return t;
}

google::spanner::v1::Type Value::MakeTypeProto(int) {
  return MakeTypeProto(std::int64_t{});
}

google::spanner::v1::Type Value::MakeTypeProto(char const*) {
  return MakeTypeProto(std::string{});
}

//
// Value::MakeValueProto
//

google::protobuf::Value Value::MakeValueProto(bool b) {
  google::protobuf::Value v;
  v.set_bool_value(b);
  return v;
}

google::protobuf::Value Value::MakeValueProto(std::int64_t i) {
  google::protobuf::Value v;
  v.set_string_value(std::to_string(i));
  return v;
}

google::protobuf::Value Value::MakeValueProto(double d) {
  google::protobuf::Value v;
  if (std::isnan(d)) {
    v.set_string_value("NaN");
  } else if (std::isinf(d)) {
    v.set_string_value(d < 0 ? "-Infinity" : "Infinity");
  } else {
    v.set_number_value(d);
  }
  return v;
}

google::protobuf::Value Value::MakeValueProto(std::string s) {
  google::protobuf::Value v;
  v.set_string_value(std::move(s));
  return v;
}

google::protobuf::Value Value::MakeValueProto(time_point ts) {
  google::protobuf::Value v;
  v.set_string_value(internal::TimestampToString(ts));
  return v;
}

google::protobuf::Value Value::MakeValueProto(Date d) {
  google::protobuf::Value v;
  v.set_string_value(internal::DateToString(d));
  return v;
}

google::protobuf::Value Value::MakeValueProto(int i) {
  return MakeValueProto(std::int64_t{i});
}

google::protobuf::Value Value::MakeValueProto(char const* s) {
  return MakeValueProto(std::string(s));
}

//
// Value::GetValue
//

StatusOr<bool> Value::GetValue(bool, google::protobuf::Value const& pv,
                               google::spanner::v1::Type const&) {
  if (pv.kind_case() == google::protobuf::Value::kNullValue) {
    return Status(StatusCode::kInvalidArgument, "value null");
  }
  return pv.bool_value();
}

StatusOr<std::int64_t> Value::GetValue(std::int64_t,
                                       google::protobuf::Value const& pv,
                                       google::spanner::v1::Type const&) {
  if (pv.kind_case() == google::protobuf::Value::kNullValue) {
    return Status(StatusCode::kInvalidArgument, "value null");
  }
  auto const& s = pv.string_value();
  char* end = nullptr;
  errno = 0;
  long long x = std::strtoll(s.c_str(), &end, 10);
  if (errno != 0) {
    auto const err = std::string(std::strerror(errno));
    return Status(StatusCode::kUnknown, err + ": \"" + s + "\"");
  }
  if (end == s.c_str()) {
    return Status(StatusCode::kUnknown, "No numeric conversion: \"" + s + "\"");
  }
  if (*end != '\0') {
    return Status(StatusCode::kUnknown, "Trailing data: \"" + s + "\"");
  }
  return x;
}

StatusOr<double> Value::GetValue(double, google::protobuf::Value const& pv,
                                 google::spanner::v1::Type const&) {
  if (pv.kind_case() == google::protobuf::Value::kNullValue) {
    return Status(StatusCode::kInvalidArgument, "value null");
  }
  if (pv.kind_case() == google::protobuf::Value::kStringValue) {
    std::string const& s = pv.string_value();
    auto const inf = std::numeric_limits<double>::infinity();
    if (s == "-Infinity") return -inf;
    if (s == "Infinity") return inf;
    if (s == "NaN") return std::nan("");
    return Status(StatusCode::kUnknown, "Bad FLOAT64 data: \"" + s + "\"");
  }
  return pv.number_value();
}

StatusOr<std::string> Value::GetValue(std::string const&,
                                      google::protobuf::Value const& pv,
                                      google::spanner::v1::Type const&) {
  if (pv.kind_case() == google::protobuf::Value::kNullValue) {
    return Status(StatusCode::kInvalidArgument, "value null");
  }
  return pv.string_value();
}

StatusOr<std::chrono::system_clock::time_point> Value::GetValue(
    time_point, google::protobuf::Value const& pv,
    google::spanner::v1::Type const&) {
  if (pv.kind_case() == google::protobuf::Value::kNullValue) {
    return Status(StatusCode::kInvalidArgument, "value null");
  }
  return internal::TimestampFromString(pv.string_value());
}

StatusOr<Date> Value::GetValue(Date, google::protobuf::Value const& pv,
                               google::spanner::v1::Type const&) {
  if (pv.kind_case() == google::protobuf::Value::kNullValue) {
    return Status(StatusCode::kInvalidArgument, "value null");
  }
  return internal::DateFromString(pv.string_value());
}

bool Value::EqualTypeProtoIgnoringNames(google::spanner::v1::Type const& a,
                                        google::spanner::v1::Type const& b) {
  google::protobuf::util::MessageDifferencer diff;
  auto const* field = google::spanner::v1::StructType::Field::descriptor();
  diff.IgnoreField(field->FindFieldByName("name"));
  return diff.Compare(a, b);
}

}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google
