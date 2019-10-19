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
#include "google/cloud/status.h"
#include "google/cloud/status_or.h"
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {

/**
 * A `Row` is a sequence of columns each with a name and an associated `Value`.
 *
 * The `Row` class is a regular value type that may be copied, moved, assigned,
 * compared for equality, etc. Instances may be large if they hold lots of
 * `Value` data, so copy only when necessary.
 *
 * `Row` instances are typically returned as the result of queries or reads of
 * a Spanner table (see `Client::Read` and `Client::ExecuteQuery`). Users will
 * mostly just use the accessor methods on `Row, and will rarely (if ever) need
 * to construct a `Row` of their own.
 *
 * The number of columns in a `Row` can be obtained from the `size()` member
 * function. The `Value`s can be obtained using the `values()` accessor. The
 * names of each column in the row can be obtained using the `columns()`
 * accessor.
 *
 * Perhaps the most convenient way to access the `Values` in a row is through
 * the varity of "get" accessors. A user may access a column's `Value' by
 * calling `get` with a `std::size_t` 0-indexed position, or a `std::string`
 * column name. Further more, callers may directly extract the native C++ type
 * by specifying the C++ type along with the column's position or name.
 *
 * @par Example
 *
 * @code
 * Row row = ...;
 * if (StatusOr<std::string> x = row.get<std::string>("LastName")) {
 *   std::cout << "LastName=" << *x << "\n";
 * }
 * @endcode
 *
 * @note There is a helper function defined below named `MakeRow()` to make
 *     creating `Row` instances for testing easier.
 */
class Row {
 public:
  // Not default constructible.
  Row() = delete;

  /**
   * Constructs a `Row` with the given @p values and @p columns.
   *
   * @note columns must not be nullptr
   * @note columns.size() must equal values.size()
   */
  explicit Row(std::vector<Value> values,
               std::shared_ptr<std::vector<std::string>> columns);

  /// @name Copy and move.
  ///@{
  Row(Row const&) = default;
  Row& operator=(Row const&) = default;
  Row(Row&&) = default;
  Row& operator=(Row&&) = default;
  ///@}

  /// Retuns the number of columns in the row.
  std::size_t size() const { return columns_->size(); }

  /// Retuns the column names for the row.
  std::vector<std::string> const& columns() const { return *columns_; }

  /// Returns the `Value` objects in the given row.
  std::vector<Value> const& values() const& { return values_; }

  /// Returns the `Value` objects in the given row.
  std::vector<Value>&& values() && { return std::move(values_); }

  /// Returns the `Value` at the given @p pos.
  StatusOr<Value> get(std::size_t pos) const&;

  /// Returns the `Value` at the given @p pos.
  StatusOr<Value> get(std::size_t pos) &&;

  /// Returns the `Value` in the column with @p name
  StatusOr<Value> get(std::string const& name) const&;

  /// Returns the `Value` in the column with @p name
  StatusOr<Value> get(std::string const& name) &&;

  /**
   * Returns the native C++ value at the given position or column name.
   *
   * @tparam T the native C++ type, e.g., std::int64_t or std::string
   */
  template <typename T, typename Arg>
  StatusOr<T> get(Arg&& arg) const& {
    auto v = get(std::forward<Arg>(arg));
    if (v) return v->template get<T>();
    return v.status();
  }

  /**
   * Returns the native C++ value at the given position or column name.
   *
   * @tparam T the native C++ type, e.g., std::int64_t or std::string
   */
  template <typename T, typename Arg>
  StatusOr<T> get(Arg&& arg) && {
    auto v = get(std::forward<Arg>(arg));
    if (v) return (*std::move(v)).template get<T>();
    return v.status();
  }

  /**
   * Returns all the native C++ values for the whole row in a `std::tuple` with
   * the specified type.
   *
   * @tparam Tuple the `std::tuple` type that the whole row must unpack into.
   */
  template <typename Tuple>
  StatusOr<Tuple> get() const {
    Tuple tup;
    auto it = values_.begin();
    Status status;
    internal::ForEach(tup, ExtractValue{status}, it);
    if (!status.ok()) return status;
    if (it == values_.end()) return tup;
    return Status(StatusCode::kInvalidArgument,
                  "Tuple has the wrong number of elements");
  }

  /// @name Equality
  ///@{
  friend bool operator==(Row const& a, Row const& b);
  friend bool operator!=(Row const& a, Row const& b) { return !(a == b); }
  ///@}

 private:
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

  std::vector<Value> values_;
  std::shared_ptr<std::vector<std::string>> columns_;
};

/**
 * Creates a `Row` instance with the given column names and values.
 *
 * This function is mostly convenient for generating rows for testing.
 *
 * @par Example
 *
 * @code
 * Row row = MakeRow({{"a", Value(1)}, {"b", Value("hi")}});
 * assert(row.size() == 2);
 * assert("hi" == *row.get<std::string>("b"));
 * @endcode
 */
Row MakeRow(std::vector<std::pair<std::string, Value>> pairs);

}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_SPANNER_GOOGLE_CLOUD_SPANNER_ROW_H_
