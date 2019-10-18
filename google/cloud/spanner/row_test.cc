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
#include "google/cloud/optional.h"
#include "google/cloud/testing_util/assert_ok.h"
#include <gmock/gmock.h>
#include <array>
#include <cstdint>
#include <string>
#include <tuple>
#include <type_traits>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {
namespace {

/* using ::testing::HasSubstr; */

/* template <typename... Ts> */
/* void VerifyRegularType(Ts&&... ts) { */
/*   auto const row = MakeRow(std::forward<Ts>(ts)...); */
/*   EXPECT_EQ(row, row); */

/*   auto copy_implicit = row; */
/*   EXPECT_EQ(copy_implicit, row); */

/*   auto copy_explicit(copy_implicit); */
/*   EXPECT_EQ(copy_explicit, copy_implicit); */

/*   using RowType = typename std::decay<decltype(row)>::type; */
/*   RowType assign;  // Default construction. */
/*   assign = row; */
/*   EXPECT_EQ(assign, row); */

/*   auto const moved = std::move(assign); */
/*   EXPECT_EQ(moved, row); */

/*   EXPECT_EQ(RowType::size(), row.size()); */
/*   static_assert(RowType::size() == sizeof...(Ts), */
/*                 "This method must be constexpr"); */
/* } */

/* TEST(TypedRow, RegularType) { */
/*   VerifyRegularType(); */
/*   VerifyRegularType(true); */
/*   VerifyRegularType(true, std::int64_t{42}); */
/* } */

/* TEST(TypedRow, ZeroTypes) { */
/*   TypedRow<> const row; */
/*   EXPECT_EQ(0, row.size()); */
/*   EXPECT_EQ(std::tuple<>{}, row.get()); */
/* } */

/* TEST(TypedRow, OneType) { */
/*   TypedRow<bool> row; */
/*   EXPECT_EQ(1, row.size()); */
/*   row = TypedRow<bool>{true}; */
/*   EXPECT_EQ(true, std::get<0>(row.get())); */
/*   EXPECT_EQ(std::make_tuple(true), row.get()); */
/* } */

/* /1* TEST(TypedRow, TwoTypes) { *1/ */
/* /1*   TypedRow<bool, std::int64_t> const row(true, 42); *1/ */
/* /1*   EXPECT_EQ(2, row.size()); *1/ */
/* /1*   EXPECT_EQ(true, std::get<0>(row.get())); *1/ */
/* /1*   EXPECT_EQ(42, std::get<1>(row.get())); *1/ */
/* /1*   EXPECT_EQ(std::make_tuple(true, std::int64_t{42}), row.get()); *1/ */
/* /1*   EXPECT_EQ(std::make_tuple(true, std::int64_t{42}), (row.get<0, 1>())); *1/ */
/* /1*   EXPECT_EQ(std::make_tuple(std::int64_t{42}, true), (row.get<1, 0>())); *1/ */
/* /1* } *1/ */

/* TEST(TypedRow, WithValues) { */
/*   TypedRow<Value, Value> row(Value(42), Value("hello")); */
/*   EXPECT_EQ(2, row.size()); */
/*   EXPECT_EQ(Value(42), std::get<0>(row.get())); */
/*   EXPECT_EQ(Value("hello"), std::get<1>(row.get())); */
/* } */

/* TEST(TypedRow, WithMixedValues) { */
/*   TypedRow<std::int64_t, Value> row(42, Value("hello")); */
/*   EXPECT_EQ(2, row.size()); */
/*   EXPECT_EQ(42, std::get<0>(row.get())); */
/*   EXPECT_EQ(Value("hello"), std::get<1>(row.get())); */
/* } */

/* /1* TEST(TypedRow, ThreeTypes) { *1/ */
/* /1*   TypedRow<bool, std::int64_t, std::string> const row(true, 42, "hello"); *1/ */
/* /1*   EXPECT_EQ(3, row.size()); *1/ */
/* /1*   EXPECT_EQ(true, std::get<0>(row.get())); *1/ */
/* /1*   EXPECT_EQ(42, std::get<1>(row.get())); *1/ */
/* /1*   EXPECT_EQ(std::make_tuple(true, std::int64_t{42}, std::string("hello")), *1/ */
/* /1*             row.get()); *1/ */
/* /1*   EXPECT_EQ(std::make_tuple(true, std::int64_t{42}, std::string("hello")), *1/ */
/* /1*             (row.get<0, 1, 2>())); *1/ */
/* /1*   EXPECT_EQ(std::make_tuple(true, std::int64_t{42}), (row.get<0, 1>())); *1/ */
/* /1*   EXPECT_EQ(std::int64_t{42}, (row.get<1>())); *1/ */
/* /1* } *1/ */

/* TEST(TypedRow, WorksWithOptional) { */
/*   auto row_null = MakeRow(optional<std::string>{}); */
/*   EXPECT_EQ(1, row_null.size()); */
/*   EXPECT_FALSE(std::get<0>(row_null.get()).has_value()); */
/*   auto values = row_null.values(); */
/*   EXPECT_EQ(1, values.size()); */
/*   auto opt_string = values[0].get<optional<std::string>>(); */
/*   EXPECT_STATUS_OK(opt_string); */
/*   EXPECT_FALSE(opt_string->has_value()); */

/*   auto row_not_null = MakeRow(optional<std::string>{"hello"}); */
/*   EXPECT_EQ(1, row_not_null.size()); */
/*   EXPECT_TRUE(std::get<0>(row_not_null.get()).has_value()); */
/*   EXPECT_EQ("hello", *std::get<0>(row_not_null.get())); */
/*   values = row_not_null.values(); */
/*   EXPECT_EQ(1, values.size()); */
/*   auto val = values[0].get<std::string>(); */
/*   EXPECT_STATUS_OK(val); */
/*   EXPECT_EQ("hello", *val); */
/* } */

/* TEST(TypedRow, Equality) { */
/*   EXPECT_EQ(MakeRow(), MakeRow()); */
/*   EXPECT_EQ(MakeRow(true, 42), MakeRow(true, 42)); */
/*   EXPECT_NE(MakeRow(true, 42), MakeRow(false, 42)); */
/*   EXPECT_NE(MakeRow(true, 42), MakeRow(true, 99)); */
/* } */

/* TEST(TypedRow, Relational) { */
/*   EXPECT_LE(MakeRow(), MakeRow()); */
/*   EXPECT_GE(MakeRow(), MakeRow()); */
/*   EXPECT_LT(MakeRow(10), MakeRow(20)); */
/*   EXPECT_GT(MakeRow(20), MakeRow(10)); */
/*   EXPECT_LT(MakeRow(false, 10), MakeRow(true, 20)); */
/*   EXPECT_GT(MakeRow("abc"), MakeRow("ab")); */
/* } */

/* TEST(TypedRow, MoveFromNonConstGet) { */
/*   // This test relies on common, but unspecified behavior of std::string. */
/*   // Specifically this test creates a string that is bigger than the SSO so it */
/*   // will likely be heap allocated, and it "moves" that string, which will */
/*   // likely leave the original string empty. This is all valid code (it's not */
/*   // UB), but it's also not specified that a string *must* leave the moved-from */
/*   // string in an "empty" state (any valid state would be OK). So if this test */
/*   // becomes flaky, we may want to disable it. But until then, it's probably */
/*   // worth having. */
/*   std::string const long_string = "12345678901234567890"; */
/*   auto row = MakeRow(long_string, long_string, long_string); */
/*   auto const copy = row; */
/*   EXPECT_EQ(row, copy); */

/*   std::string col0 = std::move(std::get<0>(row.get())); */
/*   EXPECT_EQ(col0, long_string); */

/*   EXPECT_NE(std::get<0>(row.get()), long_string);  // Unspecified behvaior */
/*   EXPECT_NE(row, copy);  // The two original Rows are no longer equal */
/* } */

/* TEST(TypedRow, SetUsingNonConstGet) { */
/*   std::string const data = "data"; */
/*   auto row = MakeRow(data, data, data); */
/*   auto const copy = row; */
/*   EXPECT_EQ(row, copy); */

/*   // "Sets" the value at column 0. */
/*   std::get<0>(row.get()) = "hello"; */
/*   EXPECT_NE(row, copy); */

/*   EXPECT_EQ("hello", std::get<0>(row.get())); */
/* } */

/* TypedRow<bool, std::string> F() { return MakeRow(true, "hello"); } */

/* TEST(TypedRow, RvalueGet) { */
/*   EXPECT_TRUE(std::get<0>(F().get())); */
/*   EXPECT_EQ("hello", std::get<1>(F().get())); */
/* } */

/* TEST(TypedRow, GetAllRefOverloads) { */
/*   // Note: This test relies on unspecified moved-from behavior of std::string. */
/*   // See the comment in "MoveFromNonConstGet" for more details. */
/*   std::string const long_string = "12345678901234567890"; */
/*   auto const row_const = MakeRow(long_string, long_string); */
/*   auto row_mut = row_const; */
/*   EXPECT_EQ(row_const, row_mut); */

/*   auto tup_const = row_const.get(); */
/*   auto tup_moved = std::move(row_mut).get(); */
/*   EXPECT_EQ(tup_const, tup_moved); */
/* } */

/* TEST(TypedRow, MakeRowTypePromotion) { */
/*   auto row = MakeRow(true, 42, "hello"); */
/*   static_assert(std::is_same<TypedRow<bool, std::int64_t, std::string>, */
/*                              decltype(row)>::value, */
/*                 "Promotes int -> std::int64_t and char const* -> std::string"); */
/* } */

/* TEST(TypedRow, MakeRowCVQualifications) { */
/*   std::string const s = "hello"; */
/*   static_assert(std::is_same<std::string const, decltype(s)>::value, */
/*                 "s is an lvalue const string"); */
/*   auto row = MakeRow(s); */
/*   static_assert(std::is_same<TypedRow<std::string>, decltype(row)>::value, */
/*                 "row holds a non-const string value"); */
/* } */

/* TEST(TypedRow, ParseRowEmpty) { */
/*   std::array<Value, 0> const array = {}; */
/*   auto const row = ParseRow<TypedRow<>>(array); */
/*   EXPECT_TRUE(row.ok()); */
/*   EXPECT_EQ(MakeRow(), *row); */
/*   EXPECT_EQ(array, row->values()); */
/* } */

/* TEST(TypedRow, ParseRowOneValue) { */
/*   // The extra braces are working around an old clang bug that was fixed in 6.0 */
/*   // https://bugs.llvm.org/show_bug.cgi?id=21629 */
/*   std::array<Value, 1> const array = {{Value(42)}}; */
/*   auto const row = ParseRow<TypedRow<std::int64_t>>(array); */
/*   EXPECT_TRUE(row.ok()); */
/*   EXPECT_EQ(MakeRow(42), *row); */
/*   EXPECT_EQ(array, row->values()); */
/*   // Tests parsing the Value with the wrong type. */
/*   auto const error_row = ParseRow<TypedRow<double>>(array); */
/*   EXPECT_FALSE(error_row.ok()); */
/* } */

/* TEST(TypedRow, ParseRowThree) { */
/*   // The extra braces are working around an old clang bug that was fixed in 6.0 */
/*   // https://bugs.llvm.org/show_bug.cgi?id=21629 */
/*   std::array<Value, 3> array = {{Value(true), Value(42), Value("hello")}}; */
/*   auto row = ParseRow<TypedRow<bool, std::int64_t, std::string>>(array); */
/*   EXPECT_TRUE(row.ok()); */
/*   EXPECT_EQ(MakeRow(true, 42, "hello"), *row); */
/*   EXPECT_EQ(array, row->values()); */
/*   // Tests parsing the Value with the wrong type. */
/*   auto const error_row = ParseRow<TypedRow<bool, double, std::string>>(array); */
/*   EXPECT_FALSE(error_row.ok()); */
/* } */

/* template <typename RowType> */
/* void RoundTripRow(RowType const& row) { */
/*   auto array = row.values(); */
/*   auto parsed = ParseRow<RowType>(array); */
/*   EXPECT_TRUE(parsed.ok()); */
/*   EXPECT_EQ(row, *parsed); */
/* } */

/* TEST(TypedRow, UnparseRow) { */
/*   RoundTripRow(MakeRow()); */
/*   RoundTripRow(MakeRow(42)); */
/*   RoundTripRow(MakeRow(42, 42)); */
/*   RoundTripRow(MakeRow(42, "hello")); */
/*   RoundTripRow(MakeRow(42, "hello", 3.14)); */

/*   RoundTripRow(MakeRow(Value(42))); */
/*   RoundTripRow(MakeRow(Value(42), "hello")); */
/*   RoundTripRow(MakeRow(Value(42), Value("hello"), 3.14)); */
/*   RoundTripRow(MakeRow(Value(42), Value("hello"), Value(3.14))); */
/* } */

/* TEST(TypedRow, ValuesAccessorRvalue) { */
/*   // There's no good way to test that move semantics actually *work* when using */
/*   // types that you don't own. So this test just verifies that properly written */
/*   // move-the-values-from-the-row code compiles and produces the results users */
/*   // should expect. In particular, we do not verify that the items were */
/*   // actually *moved-from* as opposed to copied. */
/*   constexpr auto kData = "12345678901234567890"; */
/*   auto row = MakeRow(kData); */
/*   auto array = std::move(row).values(); */
/*   auto v = array[0].get<std::string>(); */
/*   EXPECT_TRUE(v.ok()); */
/*   EXPECT_EQ(kData, *v); */
/* } */

/* TEST(TypedRow, ValuesConst) { */
/*   auto const row = MakeRow("foo", 12345, "bar"); */
/*   auto values = row.values(); */
/*   EXPECT_EQ(3, values.size()); */
/*   EXPECT_EQ(Value("foo"), values[0]); */
/*   EXPECT_EQ(Value(12345), values[1]); */
/*   EXPECT_EQ(Value("bar"), values[2]); */
/* } */

/* TEST(TypedRow, ValuesMove) { */
/*   auto row = MakeRow("foo", 12345, "bar"); */
/*   auto values = std::move(row).values(); */
/*   EXPECT_EQ(3, values.size()); */
/*   EXPECT_EQ(Value("foo"), values[0]); */
/*   EXPECT_EQ(Value(12345), values[1]); */
/*   EXPECT_EQ(Value("bar"), values[2]); */
/* } */

/* TEST(TypedRow, PrintTo) { */
/*   auto row = MakeRow(1234, "foo", 2345, "bar", "baz"); */
/*   std::ostringstream os; */
/*   PrintTo(row, &os); */
/*   auto actual = os.str(); */
/*   EXPECT_THAT(actual, HasSubstr("1234")); */
/*   EXPECT_THAT(actual, HasSubstr("foo")); */
/*   EXPECT_THAT(actual, HasSubstr("2345")); */
/*   EXPECT_THAT(actual, HasSubstr("bar")); */
/*   EXPECT_THAT(actual, HasSubstr("baz")); */
/* } */

}  // namespace
}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google
