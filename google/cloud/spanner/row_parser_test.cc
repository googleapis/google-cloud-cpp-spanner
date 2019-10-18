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

#include "google/cloud/spanner/row_parser.h"
#include "google/cloud/spanner/value.h"
#include <gmock/gmock.h>
#include <string>
#include <utility>
#include <vector>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {
namespace {

ValueSource MakeUnsharedValueSource(std::vector<Value> const& v) {
  std::size_t index = 0;
  return [=]() mutable -> StatusOr<optional<Value>> {
    if (index == v.size()) return optional<Value>{};
    return {v[(index)++]};
  };
}

ValueSource MakeSharedValueSource(std::vector<Value> const& v) {
  auto index = std::make_shared<std::size_t>(0);
  return [=]() mutable -> StatusOr<optional<Value>> {
    if (*index == v.size()) return optional<Value>{};
    return {v[(*index)++]};
  };
}

template <typename RowType>
RowParser<RowType> MakeRowParser(std::vector<Value> const& v) {
  return RowParser<RowType>(MakeSharedValueSource(v));
}

TEST(RowParser, IteratorEquality) {
  // Empty range of values.
  std::vector<Value> values = {};
  auto rp = MakeRowParser<std::tuple<std::int64_t>>(values);
  auto it = rp.begin();
  EXPECT_EQ(it, it);
  auto end = rp.end();
  EXPECT_EQ(end, end);
  EXPECT_EQ(it, end);

  // Non-empty range of values.
  values = {Value(0), Value(1), Value(2)};
  rp = MakeRowParser<std::tuple<std::int64_t>>(values);
  it = rp.begin();
  EXPECT_EQ(it, it);
  end = rp.end();
  EXPECT_EQ(end, end);
  EXPECT_NE(it, end);
}

TEST(RowParser, SuccessEmpty) {
  std::vector<Value> const values = {};
  auto rp = MakeRowParser<std::tuple<std::int64_t>>(values);
  auto it = rp.begin();
  auto end = rp.end();
  EXPECT_EQ(it, end);
}

TEST(RowParser, SuccessOneColumn) {
  std::vector<Value> const values = {
      Value(0),  // Row 0
      Value(1),  // Row 1
      Value(2),  // Row 2
      Value(3),  // Row 3
  };
  std::int64_t expected_value = 0;
  for (auto row : MakeRowParser<std::tuple<std::int64_t>>(values)) {
    EXPECT_TRUE(row.ok());
    EXPECT_EQ(expected_value, std::get<0>(*row));
    ++expected_value;
  }
  EXPECT_EQ(values.size(), expected_value);
}

TEST(RowParser, SuccessTwoColumns) {
  std::vector<Value> const values = {
      Value(true), Value(0),  // Row 0
      Value(true), Value(1),  // Row 1
      Value(true), Value(2),  // Row 2
      Value(true), Value(3),  // Row 3
  };
  std::int64_t expected_value = 0;
  for (auto row : MakeRowParser<std::tuple<bool, std::int64_t>>(values)) {
    EXPECT_TRUE(row.ok());
    EXPECT_EQ(true, std::get<0>(*row));
    EXPECT_EQ(expected_value, std::get<1>(*row));
    ++expected_value;
  }
  EXPECT_EQ(values.size() / 2, expected_value);
}

TEST(RowParser, SuccessMovedRowParser) {
  std::vector<Value> const values = {
      Value(0),  // Row 0
      Value(1),  // Row 1
      Value(2),  // Row 2
      Value(3),  // Row 3
  };

  // Makes a RowParser, and consumes the first two values.
  auto rp1 = MakeRowParser<std::tuple<std::int64_t>>(values);
  auto it1 = rp1.begin();
  auto end1 = rp1.end();

  EXPECT_NE(it1, end1);
  auto row = *it1;
  EXPECT_TRUE(row.ok());
  EXPECT_EQ(0, std::get<0>(*row));

  ++it1;
  EXPECT_NE(it1, end1);
  row = *it1;
  EXPECT_TRUE(row.ok());
  EXPECT_EQ(1, std::get<0>(*row));  // <-- Line (A)

  // Now we move the RowParser to a new object, and continue the iteration.
  // We should resume consuming where the first RowParser left off.
  auto rp2 = std::move(rp1);
  auto it2 = rp2.begin();
  auto end2 = rp2.end();

  EXPECT_NE(it2, end2);
  row = *it2;
  EXPECT_TRUE(row.ok());
  EXPECT_EQ(1,
            std::get<0>(*row));  // Same value as line (A) since op++ not called

  ++it2;
  EXPECT_NE(it2, end2);
  row = *it2;
  EXPECT_TRUE(row.ok());
  EXPECT_EQ(2, std::get<0>(*row));

  ++it2;
  EXPECT_NE(it2, end2);
  row = *it2;
  EXPECT_TRUE(row.ok());
  EXPECT_EQ(3, std::get<0>(*row));

  ++it2;
  EXPECT_EQ(it2, end2);
}

TEST(RowParser, ConstructingRowParserDoesNotConsume) {
  std::vector<Value> const values = {
      Value(0),  // Row 0
      Value(1),  // Row 1
      Value(2),  // Row 2
      Value(3),  // Row 3
  };
  ValueSource vs = MakeSharedValueSource(values);
  using RowType = std::tuple<std::int64_t>;
  auto rp1_ignored = RowParser<RowType>(vs);
  static_cast<void>(rp1_ignored);
  auto rp2_ignored = RowParser<RowType>(vs);
  static_cast<void>(rp2_ignored);

  std::int64_t expected = 0;
  for (auto row : RowParser<RowType>(vs)) {
    EXPECT_TRUE(row.ok());
    EXPECT_EQ(expected++, std::get<0>(*row));
  }
  EXPECT_EQ(values.size(), expected);
}

TEST(RowParser, RowParserCopiesValueSource) {
  std::vector<Value> const values = {
      Value(0),  // Row 0
      Value(1),  // Row 1
      Value(2),  // Row 2
      Value(3),  // Row 3
  };

  using RowType = std::tuple<std::int64_t>;
  RowParser<RowType> rp1(MakeUnsharedValueSource(values));
  RowParser<RowType> rp2 = rp1;

  std::int64_t expected = 0;
  for (auto row : rp1) {
    EXPECT_TRUE(row.ok());
    EXPECT_EQ(expected++, std::get<0>(*row));
  }
  EXPECT_EQ(values.size(), expected);

  expected = 0;
  for (auto row : rp2) {
    EXPECT_TRUE(row.ok());
    EXPECT_EQ(expected++, std::get<0>(*row));
  }
  EXPECT_EQ(values.size(), expected);

  RowParser<RowType> rp3(MakeSharedValueSource(values));
  RowParser<RowType> rp4 = rp3;

  expected = 0;
  for (auto row : rp3) {
    EXPECT_TRUE(row.ok());
    EXPECT_EQ(expected++, std::get<0>(*row));
  }
  EXPECT_EQ(values.size(), expected);

  EXPECT_EQ(rp4.begin(), rp4.end());
}

TEST(RowParser, FailOneIncompleteRow) {
  std::vector<Value> const values = {
      Value(true)  // Row 0 (incomplete)
  };
  auto rp = MakeRowParser<std::tuple<bool, std::int64_t>>(values);
  auto it = rp.begin();
  auto end = rp.end();

  // Row 0
  EXPECT_NE(it, end);
  EXPECT_FALSE(it->ok());
  EXPECT_THAT(it->status().message(), testing::HasSubstr("incomplete row"));
  ++it;

  EXPECT_EQ(it, end);
}

TEST(RowParser, FailOneRow) {
  // 4 rows of bool, std::int64_t
  std::vector<Value> const values = {
      Value(true),  Value(0),             // Row 0
      Value(false), Value(1),             // Row 1
      Value(true),  Value("WRONG TYPE"),  // Row 2
      Value(false), Value(3),             // Row 3
  };
  auto rp = MakeRowParser<std::tuple<bool, std::int64_t>>(values);
  auto it = rp.begin();
  auto end = rp.end();

  // Row 0
  EXPECT_NE(it, end);
  EXPECT_TRUE(it->ok());
  EXPECT_EQ(std::make_tuple(true, 0), **it);
  ++it;

  // Row 1
  EXPECT_NE(it, end);
  EXPECT_TRUE(it->ok());
  EXPECT_EQ(std::make_tuple(false, 1), **it);
  ++it;

  // Row 2 (this row fails to parse)
  EXPECT_NE(it, end);
  EXPECT_FALSE(it->ok());  // Error
  EXPECT_THAT(it->status().message(), testing::HasSubstr("wrong type"));
  ++it;

  EXPECT_EQ(it, end);  // Done
}

TEST(RowParser, FailAllRows) {
  // 4 rows of bool, std::int64_t
  std::vector<Value> const values = {
      Value(true),  Value(0),  // Row 0
      Value(false), Value(1),  // Row 1
      Value(true),  Value(2),  // Row 2
      Value(false), Value(3),  // Row 3
  };
  auto rp = MakeRowParser<std::tuple<std::string>>(values);
  auto it = rp.begin();
  auto end = rp.end();

  EXPECT_NE(it, end);
  EXPECT_FALSE(it->ok());  // Error
  EXPECT_THAT(it->status().message(), testing::HasSubstr("wrong type"));
  ++it;

  EXPECT_EQ(it, end);
}

TEST(RowParser, InputIteratorTraits) {
  // Tests a bunch of static traits about the RowIterator. These *could* be
  // tested in the .h file, but that'd be a bunch of noise in the header.

  std::vector<Value> const values = {Value(true), Value(0)};
  auto rp = MakeRowParser<std::tuple<bool, std::int64_t>>(values);

  auto it = rp.begin();
  auto end = rp.end();
  using It = decltype(it);

  static_assert(std::is_same<decltype(it), decltype(end)>::value, "");

  // Member alias: iterator_category
  static_assert(std::is_same<std::input_iterator_tag,
                             typename It::iterator_category>::value,
                "");
  // Member alias: value_type
  static_assert(std::is_same<StatusOr<std::tuple<bool, std::int64_t>>,
                             typename It::value_type>::value,
                "");

  // Member alias: difference_type
  static_assert(
      std::is_same<std::ptrdiff_t, typename It::difference_type>::value, "");

  // Member alias: pointer
  static_assert(
      std::is_same<typename It::value_type*, typename It::pointer>::value, "");

  // Member alias: reference
  static_assert(
      std::is_same<typename It::value_type&, typename It::reference>::value,
      "");

  // it != it2
  static_assert(std::is_same<bool, decltype(it != end)>::value, "");
  static_assert(std::is_same<bool, decltype(!(it == end))>::value, "");

  // *it
  static_assert(std::is_reference<decltype(*it)>::value, "");
  static_assert(std::is_convertible<decltype(*it), It::value_type>::value, "");

  // it->m same as (*it).m
  static_assert(std::is_same<decltype(it->ok()), decltype((*it).ok())>::value,
                "");

  // ++it same as It&
  static_assert(std::is_same<It&, decltype(++it)>::value, "");

  // it++ convertible to value_type
  static_assert(std::is_convertible<decltype(*it++), It::value_type>::value,
                "");
}

}  // namespace
}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google
