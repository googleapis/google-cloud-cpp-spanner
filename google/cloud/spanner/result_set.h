#ifndef GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_RESULT_SET_H_
#define GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_RESULT_SET_H_

#include "google/cloud/optional.h"
#include "google/cloud/spanner/column.h"
#include "google/cloud/spanner/result_stats.h"
#include "google/cloud/spanner/row.h"
#include "google/cloud/spanner/version.h"
#include <vector>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {

// Represents the result of a Spanner query/read RPC call. Gives access to the
// returned columns, rows, and stats.
// This will be returned from the Client::Read() function.
class ResultSet {
 public:
  ResultSet() = default;
  explicit ResultSet(std::vector<Row> v) {
    // ...
  }

  // XXX: perhaps moved ColumnRange into this class, and make this class itself
  // able to iterate the rows.

  ColumnRange columns() const { return {}; }
  RowStream rows() const { return {}; }

  // XXX: Can only be called after consuming the whole stream.
  google::cloud::optional<ResultStats> stats() const { return {}; }

  // XXX: add the fetched transaction timestamp (if available)
};

}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_RESULT_SET_H_
