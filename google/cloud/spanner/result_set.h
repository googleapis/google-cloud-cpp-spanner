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

#ifndef GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_RESULT_SET_H_
#define GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_RESULT_SET_H_

#include "google/cloud/optional.h"
#include "google/cloud/spanner/column.h"
#include "google/cloud/spanner/result_stats.h"
#include "google/cloud/spanner/row.h"
#include "google/cloud/spanner/version.h"
#include "google/cloud/status_or.h"
#include <vector>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {
/**
 * This class represents the results of a spanner::Client::Read and
 * spanner::Client::ExecuteSql function call. It gives callers access to the
 * result's columns and rows, as well as some ancillary optional data, such as
 * stats.
 */
class ResultSet {
 public:
  ResultSet() = default;
  // Regular type with copy, move, equality, etc.

  ColumnRange columns() const { return {}; }
  std::vector<StatusOr<Row>> rows() const { return {}; }

  // Can only be called after consuming the whole stream.
  optional<ResultStats> stats() const;

  // TODO: Add getter for the optional transaction timestamp.
};
}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_RESULT_SET_H_
