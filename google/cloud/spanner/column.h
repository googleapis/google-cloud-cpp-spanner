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

#ifndef GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_COLUMN_H_
#define GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_COLUMN_H_

#include "google/cloud/spanner/version.h"
#include "google/cloud/status_or.h"
#include <cstddef>
#include <string>
#include <vector>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {
/**
 * Spanner columns can have a name, a type, and may optionally allow "null"
 * values. This information is encapsulated by the ColumnRange and Column<T>
 * classes. After a caller gets a spanner::ResultSet from either
 * spanner::Client::Read or spanner::Client::ExecuteSql they should look up the
 * Column<T> objects for the columns they'd like to access from each row. This
 * is where column names and types are validated, and the caller can check this
 * validity once before iterating the rows, and then will not need to check
 * again. While iterating rows, callers can use their Column<T> instances to
 * access the fields of the rows in a type safe way without having to repeat
 * types nor check for mismatched type errors.
 */
template <typename T>
class Column {
 public:
  using type = T;
  // Regular value type, copy, move, equality, etc.
  Column() = default;

  std::string name() const { return {}; }
};

class ColumnRange {
 public:
  // TODO: Provide an iterator to a non-templated "column" class that can be
  // used to access all columns without knowing their types apriori.

  std::size_t size() const { return 0; }
  bool empty() const { return false; }

  // Type-safe accessors to Column<T> objects. Allows access via the
  // column's name, or its index in the range.
  template <typename T>
  StatusOr<Column<T>> get(std::string name) const {
    return {};
  }
  template <typename T>
  StatusOr<Column<T>> get(std::size_t index) const {
    return {};
  }
};
}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_COLUMN_H_
