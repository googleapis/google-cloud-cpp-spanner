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

// Represents a range of Values.
class Row {
 public:
  using iterator = std::vector<Value>::iterator;

  iterator begin() { return v_.begin(); }
  iterator end() { return v_.end(); }

  template <typename T>
  google::cloud::optional<T> get(ColumnBase c) {
    return v_[c.index()].get<T>();
  }

  // Overload that deduces T given a Column<T> argument.
  template <typename T>
  google::cloud::optional<T> get(Column<T> c) {
    return get<T>(ColumnBase(c));
  }

 private:
  friend class Client;
  void AddValue(Value value) { v_.push_back(std::move(value)); }
  std::vector<Value> v_;
};

class RowStream {
 public:
  using value_type = StatusOr<Row>;
  using iterator = std::vector<value_type>::iterator;

  iterator begin() { return {}; }
  iterator end() { return {}; }
};

}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_ROW_H_
