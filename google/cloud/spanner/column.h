#ifndef GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_COLUMN_H_
#define GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_COLUMN_H_

#include "google/cloud/spanner/row.h"
#include "google/cloud/spanner/version.h"
#include "google/cloud/status_or.h"
#include <cstddef>
#include <string>
#include <vector>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {

class ColumnBase {
 public:
  std::string name() const { return {}; }

  template <typename T>
  bool is() const {
    return {};
  }

  friend bool operator==(ColumnBase a, ColumnBase b) {
    return a.name_ == b.name_ && a.index_ == b.index_;
  }

  friend bool operator!=(ColumnBase a, ColumnBase b) { return !(a == b); }

 private:
  friend class Row;

  std::size_t index() const { return index_; }

  // TODO: Add an argument for the enum value for the type.
  explicit ColumnBase(std::string name, std::size_t index)
      : name_(std::move(name)), index_(index) {}

  std::string name_;
  std::size_t index_;
  // some Type enum to implement is<T>()
};

template <typename T>
struct Column : ColumnBase {
  using type = T;
};

class ColumnRange {
 public:
  using iterator = std::vector<ColumnBase>::iterator;

  iterator begin() { return columns_.begin(); }
  iterator end() { return columns_.end(); }

  std::size_t size() const { return columns_.size(); }
  bool empty() const { return columns_.empty(); }

  google::cloud::StatusOr<ColumnBase> get(std::string name) const { return {}; }
  google::cloud::StatusOr<ColumnBase> get(std::size_t index) const {
    return {};
  }

  template <typename T>
  google::cloud::StatusOr<Column<T>> get(std::string name) const {
    return {};
  }
  template <typename T>
  google::cloud::StatusOr<Column<T>> get(std::size_t index) const {
    return {};
  }

 private:
  std::vector<ColumnBase> columns_;
  // Maybe a map to make lookup by name faster enough?
};

}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_COLUMN_H_
