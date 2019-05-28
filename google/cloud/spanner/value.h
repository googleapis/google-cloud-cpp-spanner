#ifndef GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_VALUE_H_
#define GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_VALUE_H_

#include "google/cloud/optional.h"
#include "google/cloud/spanner/version.h"

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {

class Value {
 public:
  template <typename T>  // TODO: Add appropriate enablers
  explicit Value(T&& v) {
    // ...
  }

  // Returns true if the value's type is T.
  //   bool b = value.is<int64_t>();
  template <typename T>
  bool is() const {
    return {};
  }

  // Type-safe getter, allows
  //   auto opt = value.get<int64_t>();
  //
  // Returns nullopt if the "null" value.
  // Crashes (or UB) if T is the wrong type.
  template <typename T>
  google::cloud::optional<T> get() const {
    return {};
  }

 private:
  // Basically holds a spanner Value proto.
};

}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_VALUE_H_
