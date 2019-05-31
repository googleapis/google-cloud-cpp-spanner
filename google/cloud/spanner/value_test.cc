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
#include "google/cloud/optional.h"
#include <gmock/gmock.h>
#include <limits>
#include <string>
#include <vector>
#include <cmath>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {

using google::cloud::optional;

// A unit test that shows a brief demo of using the spanner::Value API.
TEST(Value, Demo) {
  std::string s = "hello";
  Value v(s);
  EXPECT_TRUE(v.is<std::string>());
  EXPECT_FALSE(v.is_null());
  EXPECT_EQ(s, v.get<std::string>());
  EXPECT_EQ(s, *v.get<optional<std::string>>());

  // The value and type stored in `v` can be changed.
  std::int64_t n = 42;
  v = Value(n);
  EXPECT_TRUE(v.is<std::int64_t>());
  EXPECT_FALSE(v.is_null());
  EXPECT_EQ(n, v.get<std::int64_t>());
  EXPECT_EQ(n, *v.get<optional<std::int64_t>>());

  // Sets a boolean "null" value.
  v = Value(optional<bool>{});
  EXPECT_TRUE(v.is<bool>());
  EXPECT_TRUE(v.is<optional<bool>>());
  EXPECT_TRUE(v.is_null());
  EXPECT_EQ(bool{}, v.get<bool>());
  EXPECT_EQ(optional<bool>{}, v.get<optional<bool>>());
  optional<bool> const null_bool = v.get<optional<bool>>();
  EXPECT_FALSE(null_bool);
}

TEST(Value, RegularTypeSemantics) {
  Value v1(std::int64_t{123});
  EXPECT_EQ(123, v1.get<std::int64_t>());

  Value v2 = v1;
  EXPECT_EQ(v1, v2);
  EXPECT_EQ(123, v2.get<std::int64_t>());

  Value v3(std::string("Hello"));
  EXPECT_NE(v1, v3);

  Value v4 = std::move(v3);
  EXPECT_EQ("Hello", v4.get<std::string>());
}

template <typename T>
void TestNonNullSemantics(T init) {
  Value const v{init};
  EXPECT_TRUE(v.is<T>());
  EXPECT_EQ(init, v.get<T>());
  EXPECT_EQ(init, *v.get<optional<T>>());
}

template <typename T>
void TestNullSemantics() {
  optional<T> const null;
  Value v{null};
  EXPECT_TRUE(v.is<T>());
  EXPECT_TRUE(v.is_null());
  EXPECT_EQ(T{}, v.get<T>());  // Defined to return T{} even if null
  optional<T> const null_value = v.get<optional<T>>();
  EXPECT_FALSE(null_value);
}

TEST(Value, Semantics) {
  TestNullSemantics<bool>();
  for (auto x : {false, true}) {
    SCOPED_TRACE("Testing: bool " + std::to_string(x));
    TestNonNullSemantics<bool>(x);
    TestNonNullSemantics<optional<bool>>(x);
  }

  TestNullSemantics<std::int64_t>();
  auto const min64 = std::numeric_limits<std::int64_t>::min();
  auto const max64 = std::numeric_limits<std::int64_t>::max();
  for (auto x : std::vector<std::int64_t>{min64, -1, 0, 1, max64}) {
    SCOPED_TRACE("Testing: std::int64_t " + std::to_string(x));
    TestNonNullSemantics<std::int64_t>(x);
    TestNonNullSemantics<optional<std::int64_t>>(x);
  }

  // Note: We skip testing the NaN case here because NaN always compares not
  // equal, even with itself. So NaN is handled in a separate test.
  TestNullSemantics<double>();
  auto const inf = std::numeric_limits<double>::infinity();
  for (auto x : {-inf, -1.0, -0.5, 0.0, 0.5, 1.0, inf}) {
    SCOPED_TRACE("Testing: double " + std::to_string(x));
    TestNonNullSemantics<double>(x);
    TestNonNullSemantics<optional<double>>(x);
  }

  TestNullSemantics<std::string>();
  for (auto x : {"", "f", "foo", "12345678901234567890"}) {
    SCOPED_TRACE("Testing: std::string " + std::string(x));
    TestNonNullSemantics<std::string>(x);
    TestNonNullSemantics<optional<std::string>>(x);
  }
}

TEST(Value, DoubleNaN) {
  double const d = std::nan("NaN");
  Value v{d};
  EXPECT_TRUE(v.is<double>());
  EXPECT_FALSE(v.is_null());
  EXPECT_TRUE(std::isnan(v.get<double>()));
  EXPECT_TRUE(std::isnan(*v.get<optional<double>>()));
}

}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google
