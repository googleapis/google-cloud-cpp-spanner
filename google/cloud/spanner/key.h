#ifndef GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_KEY_H_
#define GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_KEY_H_

#include "google/cloud/optional.h"
#include "google/cloud/spanner/value.h"
#include "google/cloud/spanner/version.h"
#include <string>
#include <vector>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {

// Represents a range of values that correspond to a DB table index. Similar to
// a Row (both contain a range of values), but this doesn't have column names,
// nor any of the accessors. Maybe we want to have some other accessors here?
//
// TODO: Need some way to specify a null value
class Key {
 public:
  template <typename... Ts>  // XXX: Add appropriate enablers.
  explicit Key(Ts&&... ts) : v{Value(std::forward<Ts>(ts))...} {}

 private:
  std::vector<Value> v;
};

// Represents the name of a database table index, along with a bunch of Keys
// and (TODO) key ranges for the specified index.
//
// TODO: Add support for key ranges as well.
// TODO: Do we want some fancy syntax to build key sets and ranges nicely?
//       Maybe op+, op+=, etc?
class KeySet {
 public:
  static KeySet All() { return KeySet(all_tag{}); }

  KeySet() = default;  // uses the primary index.
  explicit KeySet(std::string index_name) : index_(std::move(index_name)) {}
  KeySet(std::string index_name, std::vector<Key> keys)
      : index_(std::move(index_name)), keys_(std::move(keys)) {}

  void Add(Key key) { keys_.push_back(std::move(key)); }
  void Limit(int limit) { limit_ = limit; }

 private:
  struct all_tag {};
  explicit KeySet(all_tag) : all_(true) {}
  std::string index_;
  std::vector<Key> keys_;
  bool all_ = false;
  google::cloud::optional<int> limit_;
};

}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_KEY_H_
