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
/**
 * A spanner::KeySet is an argument to the spanner::Client::Read functions. It
 * represents the name of a Spanner database table index, such as the primary
 * key index or a secondary index, and a set of keys for that index. Keys may be
 * specified as individual keys, key ranges (with open or closed intervals), or
 * a special "all keys" token. Additionally, the KeySet may contain a "limit",
 * which limits the number of results returned. A "key" consists of the value(s)
 * for the column(s) used in the named index. This is encapsulated by the
 * spanner::Key class. The following is an outline of what both of these classes
 * would look like:
 */
class Key {
 public:
  template <typename... Ts>  // TODO: Add appropriate enabler
  explicit Key(Ts&&... ts) {}
  // Value type, op==, op!=
  // ...
};

class KeySet {
 public:
  static KeySet All() { return KeySet(/* all_tag{} */); }

  KeySet() = default;  // uses the primary key index.
  explicit KeySet(std::string index_name, std::vector<Key> keys = {})
      : index_name_(std::move(index_name)), keys_(std::move(keys)) {}
  // TODO: Make this a value type with op==, op!=
  // TODO: Add support for key ranges

  void Add(Key key) { keys_.emplace_back(std::move(key)); }
  void SetLimit(int limit) { limit_ = limit; }

 private:
  std::string index_name_;
  std::vector<Key> keys_;
  int limit_;
};
}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_KEY_H_
