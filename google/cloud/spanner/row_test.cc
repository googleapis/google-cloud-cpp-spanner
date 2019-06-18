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

#include "google/cloud/spanner/row.h"
#include <gmock/gmock.h>
#include <cstdint>
#include <string>
#include <tuple>
#include <type_traits>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {
template <typename... Ts>
void VerifyRegularType(Ts&&... ts) {
  auto const row = MakeRow(std::forward<Ts>(ts)...);
  EXPECT_EQ(row, row);

  auto const copy = row;
  EXPECT_EQ(copy, row);

  using RowType = typename std::decay<decltype(row)>::type;
  RowType assign;  // Default construction.
  assign = copy;
  EXPECT_EQ(assign, copy);

  auto const moved = std::move(copy);
  EXPECT_EQ(moved, row);
}

TEST(Row, RegularType) {
  VerifyRegularType();
  VerifyRegularType(true);
  VerifyRegularType(true, std::int64_t{42});
}

TEST(Row, ZeroTypes) {
  Row<> row;
  EXPECT_EQ(0, row.size());
  EXPECT_EQ(std::tuple<>{}, row.get());
}

TEST(Row, OneType) {
  Row<bool> row;
  EXPECT_EQ(1, row.size());
  row = Row<bool>{true};
  EXPECT_EQ(true, row.get<0>());
  EXPECT_EQ(std::make_tuple(true), row.get());
}

TEST(Row, TwoTypes) {
  Row<bool, std::int64_t> row(true, 42);
  EXPECT_EQ(2, row.size());
  EXPECT_EQ(true, row.get<0>());
  EXPECT_EQ(42, row.get<1>());
  EXPECT_EQ(std::make_tuple(true, std::int64_t{42}), row.get());
  EXPECT_EQ(std::make_tuple(true, std::int64_t{42}), (row.get<0, 1>()));
  EXPECT_EQ(std::make_tuple(std::int64_t{42}, true), (row.get<1, 0>()));
}

TEST(Row, ThreeTypes) {
  Row<bool, std::int64_t, std::string> row(true, 42, "hello");
  EXPECT_EQ(3, row.size());
  EXPECT_EQ(true, row.get<0>());
  EXPECT_EQ(42, row.get<1>());
  EXPECT_EQ(std::make_tuple(true, std::int64_t{42}, std::string("hello")),
            row.get());
  EXPECT_EQ(std::make_tuple(true, std::int64_t{42}, std::string("hello")),
            (row.get<0, 1, 2>()));
  EXPECT_EQ(std::make_tuple(true, std::int64_t{42}), (row.get<0, 1>()));
  EXPECT_EQ(std::int64_t{42}, (row.get<1>()));
}

struct Foo {};
TEST(Row, BadTypes) { Row<Foo> row; }

}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google
