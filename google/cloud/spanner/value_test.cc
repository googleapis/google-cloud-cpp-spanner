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
#include <cmath>
#include <limits>
#include <string>
#include <vector>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {
namespace {

using google::cloud::optional;

// When testing the basic semantics of a Value object, the result of
// v.is_null() depends on whether the Value was constructed with an empty
// optional<T> or not. Factoring this check for an an empty optional out allows
// us to reuse the same TestBasicSemantics<T>() function template with and
// without nulls.
template <typename T>
bool IsEmptyOptional(T) {
  return false;
}
template <typename T>
bool IsEmptyOptional(optional<T> opt) {
  return !opt.has_value();
}

}  // namespace

template <typename T>
void TestBasicSemantics(T init) {
  Value const v{init};

  EXPECT_EQ(IsEmptyOptional(init), v.is_null()) << v;

  EXPECT_TRUE(v.is<T>()) << v;
  EXPECT_TRUE(v.is<optional<T>>()) << v;

  EXPECT_EQ(init, v.get<T>()) << v;
  EXPECT_EQ(init, *v.get<optional<T>>()) << v;

  Value const copy = v;
  EXPECT_EQ(copy, v);

  Value const moved = std::move(copy);
  EXPECT_EQ(moved, v);
}

TEST(Value, BasicSemantics) {
  for (auto x : {false, true}) {
    SCOPED_TRACE("Testing: bool " + std::to_string(x));
    TestBasicSemantics(x);
    TestBasicSemantics(optional<bool>{x});
    TestBasicSemantics(optional<bool>{});
  }

  auto const min64 = std::numeric_limits<std::int64_t>::min();
  auto const max64 = std::numeric_limits<std::int64_t>::max();
  for (auto x : std::vector<std::int64_t>{min64, -1, 0, 1, max64}) {
    SCOPED_TRACE("Testing: std::int64_t " + std::to_string(x));
    TestBasicSemantics(x);
    TestBasicSemantics(optional<std::int64_t>{x});
    TestBasicSemantics(optional<std::int64_t>{});
  }

  // Note: We skip testing the NaN case here because NaN always compares not
  // equal, even with itself. So NaN is handled in a separate test.
  auto const inf = std::numeric_limits<double>::infinity();
  for (auto x : {-inf, -1.0, -0.5, 0.0, 0.5, 1.0, inf}) {
    SCOPED_TRACE("Testing: double " + std::to_string(x));
    TestBasicSemantics(x);
    TestBasicSemantics(optional<double>{x});
    TestBasicSemantics(optional<double>{});
  }

  for (auto x :
       std::vector<std::string>{"", "f", "foo", "123456789012345678"}) {
    SCOPED_TRACE("Testing: std::string " + std::string(x));
    TestBasicSemantics(x);
    TestBasicSemantics(optional<std::string>{x});
    TestBasicSemantics(optional<std::string>{});
  }
}

TEST(Value, DoubleNaN) {
  double const nan = std::nan("NaN");
  Value v{nan};
  EXPECT_TRUE(v.is<double>());
  EXPECT_FALSE(v.is_null());
  EXPECT_TRUE(std::isnan(v.get<double>()));
  EXPECT_TRUE(std::isnan(*v.get<optional<double>>()));

  // Since IEEE 754 defines that nan is not equal to itself, then a Value with
  // NaN should not be equal to itself.
  EXPECT_NE(nan, nan);
  EXPECT_NE(v, v);
}

}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google
