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

#ifndef GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_ROW_H_
#define GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_ROW_H_

#include <cstdint>
#include <string>
#include <tuple>
#include <utility>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {
/**
 * The `Row<Ts...>` class template represents a heterogeneous set of C++ values.
 *
 * The values stored in a row may have any of the valid Spanner types described
 * in value.h. Each value in a row is identified by its column index, with the
 * first value corresponding to column 0. The number of columns in the row is
 * given by the `Row::size()` member function (This exactly matches the number
 * of types the Row was created with).
 *
 * A `Row<Ts...>` is a regular C++ type, supporting default construction, copy,
 * assignment, move, and equality as expected. It should typically be
 * constructed with values using the non-member `spanner::MakeRow(Ts...)`
 * function template (see below). Once an instance is created, you can access
 * the values in the row using the `Row::get()` overloaded functions.
 *
 * Example:
 *
 *     spanner::Row<bool, std::int64_t, std::string> row;
 *     row = spanner::MakeRow(true, 42, "hello");
 *     static_assert(row.size() == 3, "Created with three types");
 *
 *     // Gets all values as a std::tuple
 *     std::tuple<bool, std::int64_t, std::string> tup = row.get();
 *
 *     // Gets one value at the specified column index
 *     std::int64_t i = row.get<1>();
 *
 *     // Gets the values from the specified columns (any order), and returns a
 *     // tuple. Since C++17, this tuple will work with structured bindings to
 *     // allow assigning to individually  named variables.
 *     auto [name, age] = row.get<2, 1>();
 *
 */
template <typename... Types>
class Row {
  template <std::size_t I>
  using ColumnType = typename std::tuple_element<I, std::tuple<Types...>>::type;

 public:
  /// Regular value type, supporting copy, assign, move, etc.
  Row() = default;
  Row(Row const&) = default;
  Row& operator=(Row const&) = default;
  Row(Row&) = default;
  Row& operator=(Row&&) = default;

  /**
   * Constructs a Row from the specified values.
   *
   * See also the `MakeRow()` helper function template for a convenient way to
   * make rows without having to also specify the value's types.
   */
  template <typename... Ts>
  explicit Row(Ts... ts) : values_(std::forward<Types>(ts)...) {}

  /// Returns the number of columns in this row.
  constexpr std::size_t size() const { return sizeof...(Types); }

  /**
   *  Returns the value at position `I`.
   *
   *  Example:
   *
   *      auto row = MakeRow(true, "foo");
   *      assert(row.get<0>() == true);
   *      assert(row.get<1>() == "foo");
   */
  template <std::size_t I>
  ColumnType<I> get() const {
    return std::get<I>(values_);
  }

  /**
   * Returns a std::tuple of the values at the specified positions (specified
   * in any order).
   *
   *  Example:
   *
   *      auto row = MakeRow(true, "foo", 42);
   *      std::tuple<std::int64_t, bool> tup = row.get<2, 0>();
   *      assert(std::get<0>(tup) == 42);
   *      assert(std::get<1>(tup) == true);
   */
  template <std::size_t... I,
            typename std::enable_if<(sizeof...(I) > 1), int>::type = 0>
  std::tuple<ColumnType<I>...> get() const {
    return std::make_tuple(get<I>()...);
  }

  /**
   * Returns a std::tuple containing all the values in the row.
   *
   *  Example:
   *
   *      auto row = MakeRow(true, "foo", 42);
   *      std::tuple<bool, std::string, std::int64_t> tup
   *          = row.get();
   *      assert(std::get<0>(tup) == true);
   *      assert(std::get<1>(tup) == "foo");
   *      assert(std::get<2>(tup) == 42);
   */
  std::tuple<Types...> get() const { return values_; }

  /// Equality operators.
  friend bool operator==(Row const& a, Row const& b) {
    return a.values_ == b.values_;
  }
  friend bool operator!=(Row const& a, Row const& b) { return !(a == b); }

 private:
  std::tuple<Types...> values_;
};

namespace internal {
// A helper metafunction that promots some C++ literal types to the types
// required by Cloud Spanner. For example, a literal 42 is of type int, but
// this will promote it to type std::int64_t. Similarly, a C++ string literal
// will be "promoted" to type std::string.
template <typename T>
constexpr T PromoteLiteralImpl(T);
constexpr std::string PromoteLiteralImpl(char const*);
constexpr std::int64_t PromoteLiteralImpl(int);
template <typename T>
using PromoteLiteral = decltype(PromoteLiteralImpl(std::declval<T>()));
}  // namespace internal

/**
 * Helper factory function to construct `Row<Ts...>` objects holding the
 * specified values.
 *
 * This is helpful because it deduces the types of the arguments, and it
 * promotes integer and string literals to the necessary Spanner types
 * `std::int64_t` and `std::string`, respectively.
 *
 * Example:
 *
 *     auto row = MakeRow(42, "hello");
 *     static_assert(
 *         std::is_same<Row<std::int64_t, std::string>, decltype(row)>::value,
 *         "");
 */
template <typename... Ts>
Row<internal::PromoteLiteral<Ts>...> MakeRow(Ts&&... ts) {
  return Row<internal::PromoteLiteral<Ts>...>{std::forward<Ts>(ts)...};
}

}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_ROW_H_
