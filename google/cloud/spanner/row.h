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

#include "google/cloud/optional.h"
#include "google/cloud/spanner/column.h"
#include "google/cloud/spanner/value.h"
#include "google/cloud/spanner/version.h"
#include <vector>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {
/**
 * A spanner::Row represents a range of "values" and their corresponding
 * columns. A spanner::Value is a strongly-typed object at a row-column
 * intersection that may be "null". Rows provide type-safe access to values via
 * accessors taking a spanner::Column<T>.
 */
class Row {
 public:
  using iterator = std::vector<Value>::iterator;
  // …

  iterator begin() { return {}; }
  iterator end() { return {}; }
  // …

  template <typename T>
  T get(Column<T> const& col) const {
    return {};
  }
};
}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_ROW_H_
