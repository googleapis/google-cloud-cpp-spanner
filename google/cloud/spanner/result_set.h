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

#include "google/cloud/spanner/internal/time.h"
#include "google/cloud/spanner/row_parser.h"
#include "google/cloud/spanner/timestamp.h"
#include "google/cloud/optional.h"
#include <google/spanner/v1/spanner.pb.h>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {

namespace internal {
class ResultSetValueSource {
 public:
  virtual ~ResultSetValueSource() = default;
  // Returns OK Status with no Value to indicate end-of-stream.
  virtual StatusOr<optional<Value>> GetNextValue() = 0;
  virtual optional<google::spanner::v1::ResultSetMetadata> GetMetadata() = 0;
  virtual optional<google::spanner::v1::ResultSetStats> GetStats() = 0;
};
}  // namespace internal

/**
 * ResultSet is returned from "query" operations (`Read`, `ExecuteSql`).
 */
class ResultSet {
 public:
  ResultSet() = default;
  explicit ResultSet(std::unique_ptr<internal::ResultSetValueSource> source)
      : source_(std::move(source)) {}

  // This class is moveable but not copyable.
  ResultSet(ResultSet&&) = default;
  ResultSet& operator=(ResultSet&&) = default;

  /**
   * Return a `RowParser` which can be used to iterate through the results.
   */
  template <typename... Ts>
  RowParser<Ts...> rows() {
    return RowParser<Ts...>(
        [this]() mutable { return source_->GetNextValue(); });
  }

  /**
   * Retrieve the timestamp at which the read occurred.
   *
   * Only available if a read-only transaction was used, and the timestamp
   * was requested by setting `return_read_timestamp` true.
   */
  optional<Timestamp> read_timestamp() const {
    auto metadata = source_->GetMetadata();
    if (metadata.has_value() && metadata->has_transaction() &&
        metadata->transaction().has_read_timestamp()) {
      return internal::FromProto(metadata->transaction().read_timestamp());
    }
    return optional<Timestamp>();
  }

  /**
   * Return statistics about the transaction.
   *
   * Statistics are only available for SQL requests with `query_mode` set to
   * `PROFILE`, and only after consuming the entire result stream (i.e.
   * iterating through `rows()` until the end).
   *
   * TODO(#217) Determine what type we should return from this method.
   */
  optional<google::spanner::v1::ResultSetStats> GetStats() {
    return source_->GetStats();
  }

 private:
  std::unique_ptr<internal::ResultSetValueSource> source_;
};

}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_RESULT_SET_H_
