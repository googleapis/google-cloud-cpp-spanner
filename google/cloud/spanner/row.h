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

#ifndef GOOGLE_CLOUD_CPP_SPANNER_GOOGLE_CLOUD_SPANNER_ROW_H_
#define GOOGLE_CLOUD_CPP_SPANNER_GOOGLE_CLOUD_SPANNER_ROW_H_

#include "google/cloud/spanner/internal/tuple_utils.h"
#include "google/cloud/spanner/value.h"
#include "google/cloud/spanner/version.h"
#include "google/cloud/internal/disjunction.h"
#include "google/cloud/status_or.h"
#include <array>
#include <cstdint>
#include <string>
#include <tuple>
#include <utility>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {

/**
 * The `TypedRow<Ts...>` class template represents a heterogeneous set of C++
 * values.
 *
 * The values stored in a row may have any of the valid Spanner types that can
 * be stored in `Value`. Each value in a row is identified by its column index,
 * with the first value corresponding to column 0. The number of columns in the
 * row is given by the `TypedRow::size()` member function (This exactly matches
 * the number of types the `TypedRow` was created with).
 *
 * A `TypedRow<Ts...>` is a regular C++ type, supporting default construction,
 * copy, assignment, move, and equality as expected. It should typically be
 * constructed with values using the non-member `spanner::MakeRow(Ts...)`
 * function template (see below). Once an instance is created, you can access
 * the values in the row using the `TypedRow::get()` overloaded functions.
 *
 * @par Example
 *
 * @code
 * spanner::TypedRow<bool, std::int64_t, std::string> row;
 * row = spanner::MakeRow(true, 42, "hello");
 * static_assert(row.size() == 3, "Created with three types");
 *
 * // Gets all values as a std::tuple
 * std::tuple<bool, std::int64_t, std::string> tup = row.get();
 *
 * // Gets one value at the specified column index
 * std::int64_t i = row.get<1>();
 *
 * // Gets the values from the specified columns (any order), and returns a
 * // tuple. Since C++17, this tuple will work with structured bindings to
 * // allow assigning to individually  named variables.
 * auto [name, age] = row.get<2, 1>();
 * @endcode
 *
 * @tparam Types... The C++ types for each column in the row.
 */
template <typename... Ts>
using TypedRow = std::tuple<Ts...>;

namespace internal {

// A helper functor to be used with `internal::ForEach` that adds each
// element of the values_ tuple to an array of `Value` objects.
struct PushBackValue {
  std::vector<Value>& v;
  template <typename T>
  void operator()(T&& t) {
    v.push_back(Value(std::forward<T>(t)));
  }
};

// A helper functor to be used with `internal::ForEach` to iterate the columns
// of a `TypedRow<Ts...>` in the ParseRow() function.
struct ExtractValue {
  Status& status;
  template <typename T, typename It>
  void operator()(T& t, It& it) const {
    auto x = it++->template get<T>();
    if (!x) {
      status = std::move(x).status();
    } else {
      t = *std::move(x);
    }
  }
};

}  // namespace internal

template <typename... Ts>
std::vector<Value> MakeValues(TypedRow<Ts...> row) {
  std::vector<Value> v;
  internal::ForEach(std::move(std::move(row)), internal::PushBackValue{v});
  return v;
}

/**
 * Parses a `std::array` of `Value` objects into a `TypedRow<Ts...>` holding C++
 * types.
 *
 * If parsing fails, an error `Status` is returned. The given array size must
 * exactly match the number of types in the specified `TypedRow<Ts...>`. See
 * `TypedRow::size()`.
 *
 * @par Example
 *
 * @code
 * std::array<Value, 3> array = {Value(true), Value(42), Value("hello")};
 * using RowType = TypedRow<bool, std::int64_t, std::string>;
 * auto row = ParseRow<RowType>(array);
 * assert(row.ok());
 * assert(MakeRow(true, 42, "hello"), *row);
 * @endcode
 */
template <typename RowType, std::size_t N>
StatusOr<RowType> ParseRow(std::array<Value, N> const& array) {
  RowType row;
  auto it = array.begin();
  Status status;
  internal::ForEach(row, internal::ExtractValue{status}, it);
  if (!status.ok()) return status;
  return row;
}

}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_SPANNER_GOOGLE_CLOUD_SPANNER_ROW_H_
