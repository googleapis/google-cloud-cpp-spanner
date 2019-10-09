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

#ifndef GOOGLE_CLOUD_CPP_SPANNER_GOOGLE_CLOUD_SPANNER_KEYS_H_
#define GOOGLE_CLOUD_CPP_SPANNER_GOOGLE_CLOUD_SPANNER_KEYS_H_

#include "google/cloud/spanner/value.h"
#include "google/cloud/spanner/version.h"
#include <google/spanner/v1/keys.pb.h>
#include <string>
#include <utility>
#include <vector>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {

/**
 * A `Key` is a collection of `Value` objects where the i'th value corresponds
 * to the i'th component of the table or primary index key.
 *
 * In C++, this is implemented as a `std::vector<Value>`. See the `MakeKey`
 * factory function below for an easy way to construct a valid `Key` instance.
 */
using Key = std::vector<Value>;

/**
 * Constructs a `Key` from the given arguments.
 *
 * @par Example
 *
 * @code
 *  Key key = MakeKey(123, "hello");
 *  assert(key.size() == 2);
 *  assert(key[0] == Value(123));
 *  assert(key[1] == Value("hello"));
 * @endcode
 */
template <typename... Ts>
Key MakeKey(Ts&&... ts) {
  return Key{Value(std::forward<Ts>(ts))...};
}

/**
 * The `KeyBound` class is a regular type that represents an open or closed
 * endpoint for a range of keys.
 *
 * `KeyBound`s can be "open", meaning the matching row will be excluded from
 * the results, or "closed" meaning the matching row will be included.
 * `KeyBound` instances should be created with the `MakeKeyBoundOpen()` or
 * `MakeKeyBoundClosed()` factory functions.
 *
 * See the `MakeKeyBoundClosed()` and `MakeKeyBoundOpen()` factory functions
 * below for convenient ways to construct instances of KeyBound.
 */
class KeyBound {
 public:
  /// An enum indicating whether the `Key` is included (closed) or excluded
  /// (open).
  enum Bound { kClosed, kOpen };

  /// Not default constructible
  KeyBound() = delete;

  /// Constructs an instance with the given @p key and @p bound.
  KeyBound(Key key, Bound bound) : key_(std::move(key)), bound_(bound) {}

  /// @name Copy and move semantics.
  ///@{
  KeyBound(KeyBound const&) = default;
  KeyBound& operator=(KeyBound const&) = default;
  KeyBound(KeyBound&&) = default;
  KeyBound& operator=(KeyBound&&) = default;
  ///@}

  /// Returns the `Key`.
  Key const& key() const& { return key_; }

  /// Returns the `Key` (by move).
  Key&& key() && { return std::move(key_); }

  /// Returns the `Bound`.
  Bound bound() const { return bound_; }

  /// @name Equality.
  ///@{
  friend bool operator==(KeyBound const& a, KeyBound const& b);
  friend bool operator!=(KeyBound const& a, KeyBound const& b) {
    return !(a == b);
  }
  ///@}

 private:
  Key key_;
  Bound bound_;
};

/**
 * Returns a "closed" `KeyBound` with a `Key` constructed from the given
 * arguments.
 */
template <typename... Ts>
KeyBound MakeKeyBoundClosed(Ts&&... ts) {
  return KeyBound(MakeKey(std::forward<Ts>(ts)...), KeyBound::kClosed);
}

/**
 * Returns an "open" `KeyBound` with a `Key` constructed from the given
 * arguments.
 */
template <typename... Ts>
KeyBound MakeKeyBoundOpen(Ts&&... ts) {
  return KeyBound(MakeKey(std::forward<Ts>(ts)...), KeyBound::kOpen);
}

class KeySet;
namespace internal {
::google::spanner::v1::KeySet ToProto(KeySet);
KeySet FromProto(::google::spanner::v1::KeySet);
}  // namespace internal

/**
 * The `KeySet` class is a regular type that represents a collection of `Key`s.
 *
 * Users can construct a `KeySet` instance, then add `Key`s and ranges of
 * `Key`s to the set. The caller is responsible for ensuring that all keys in a
 * given `KeySet` instance contain the same number and types of values.
 *
 * Users may also optionally construct an instance that
 * represents all keys with `KeySet::All()`.
 */
class KeySet {
 public:
  /// Returns a `KeySet` that represents the set of "All" keys for the index.
  static KeySet All() {
    KeySet ks;
    ks.proto_.set_all(true);
    return ks;
  }

  /// Constructs an empty `KeySet`.
  KeySet() = default;

  // Copy and move constructors and assignment operators.
  KeySet(KeySet const& key_range) = default;
  KeySet& operator=(KeySet const& rhs) = default;
  KeySet(KeySet&& key_range) = default;
  KeySet& operator=(KeySet&& rhs) = default;

  /// Adds the given @p key to the `KeySet`.
  KeySet& AddKey(Key key); 

  /// Constructs a `Key` from the given args and adds it to the `KeySet`.
  template <typename... Ts>
  KeySet& AddKey(Ts&&... ts) {
    return AddKey(MakeKey(std::forward<Ts>(ts)...));
  }

  /// Adds a range of keys defined by the given `KeyBound`s.
  KeySet& AddRange(KeyBound start, KeyBound end);

  /// @name Equality
  ///@{
  friend bool operator==(KeySet const& a, KeySet const& b);
  friend bool operator!=(KeySet const& a, KeySet const& b) { return !(a == b); }
  ///@}

 private:
  friend ::google::spanner::v1::KeySet internal::ToProto(KeySet);
  friend KeySet internal::FromProto(::google::spanner::v1::KeySet);
  explicit KeySet(google::spanner::v1::KeySet proto)
      : proto_(std::move(proto)) {}

  google::spanner::v1::KeySet proto_;
};

}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_SPANNER_GOOGLE_CLOUD_SPANNER_KEYS_H_
