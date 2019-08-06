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

#include "google/cloud/spanner/row.h"
#include "google/cloud/spanner/value.h"
#include "google/cloud/spanner/version.h"
#include "google/cloud/status_or.h"
#include <string>
#include <vector>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {

template <typename KeyType>
class KeyRange;

/**
 * The `Bound` class is a regular type that represents one endpoint of an
 * interval of keys.
 *
 * `Bound`s are `Closed` by default, meaning the row matching
 * the Bound will be included in the result. `Bound`s can also be
 * specified as `Open`, which will exclude the Bounds from
 * the results.
 *
 * @tparam KeyType spanner::Row<Types...> that corresponds to the desired index
 * definition.
 */
template <typename KeyType>
class Bound {
 public:
  Bound() = default;
  explicit Bound(KeyType key) : key_(std::move(key)), mode_(Mode::CLOSED) {}

  // Copy and move constructors and assignment operators.
  Bound(Bound const& key_range) = default;
  Bound& operator=(Bound const& rhs) = default;
  Bound(Bound&& key_range) = default;
  Bound& operator=(Bound&& rhs) = default;

  KeyType const& key() const { return key_; }
  bool IsClosed() const { return mode_ == Mode::CLOSED; }
  bool IsOpen() const { return mode_ == Mode::OPEN; }

  friend bool operator==(Bound const& lhs, Bound const& rhs) {
    return lhs.key_ == rhs.key_ && lhs.mode_ == rhs.mode_;
  }
  friend bool operator!=(Bound const& lhs, Bound const& rhs) {
    return !(lhs == rhs);
  }

 private:
  enum class Mode { CLOSED, OPEN };

  Bound(KeyType key, Mode mode) : key_(std::move(key)), mode_(mode) {}

  template <typename... ValueTypes>
  friend Bound<Row<internal::PromoteLiteral<ValueTypes>...>>
  MakeKeyRangeBoundClosed(ValueTypes... values);

  template <typename... ValueTypes>
  friend Bound<Row<internal::PromoteLiteral<ValueTypes>...>>
  MakeKeyRangeBoundOpen(ValueTypes... values);

  template<typename RowType>
  friend KeyRange<RowType> MakeKeyRange(RowType start, RowType end);


  KeyType key_;
  Mode mode_;
};

/**
 * The `KeyRange` class is a regular type that represents the pair of `Bound`s
 * necessary to uniquely identify a contiguous group of key `spanner::Row`s in
 * its index.
 *
 * @tparam KeyType spanner::Row<Types...> that corresponds to the desired index
 * definition.
 */
template <typename KeyType>
class KeyRange {
 public:
  /**
   * Constructs and empty `KeyRange`.
   */
  KeyRange() : start_(), end_() {}
  ~KeyRange() = default;

  /**
   * Constructs a `KeyRange` with closed `Bound`s on the keys provided.
   */
  explicit KeyRange(Bound<KeyType> start, Bound<KeyType> end)
      : start_(std::move(start)), end_(std::move(end)) {}

  /**
   * Constructs a `KeyRange` closed on both `Bound`s.
   */
  explicit KeyRange(KeyType start, KeyType end)
      : KeyRange(std::move(Bound<KeyType>(start)),
          std::move(Bound<KeyType>(end))) {}

  // Copy and move constructors and assignment operators.
  KeyRange(KeyRange const& key_range) = default;
  KeyRange& operator=(KeyRange const& rhs) = default;
  KeyRange(KeyRange&& key_range) = default;
  KeyRange& operator=(KeyRange&& rhs) = default;

  Bound<KeyType> const& start() const { return start_; }
  Bound<KeyType> const& end() const { return end_; }

 private:
  friend bool operator==(KeyRange const& lhs, KeyRange const& rhs) {
    return lhs.start_ == rhs.start_ && lhs.end_ == rhs.end_;
  }
  friend bool operator!=(KeyRange const& lhs, KeyRange const& rhs) {
    return !(lhs == rhs);
  }

  Bound<KeyType> start_;
  Bound<KeyType> end_;
};

/**
 * The `KeySet` class is a regular type that represents the collection of
 * `spanner::Row`s necessary to uniquely identify a arbitrary group of rows in
 * its index.
 *
 * A `KeySet` can consist of multiple `KeyRange` and/or `spanner::Row`
 * instances.
 */
template <typename KeyType>
class KeySet {
 public:
  /**
   * Constructs an empty `KeySet`.
   */
  KeySet() = default;

  /**
   * Constructs a `KeySet` with a single key `spanner::Row`.
   * @param key
   */
  explicit KeySet(KeyType key) : keys_(), key_ranges_() {
    keys_.emplace_back(std::move(key));
  }

  /**
   * Constructs a `KeySet` with a single `KeyRange`.
   * @param key_range
   */
  explicit KeySet(KeyRange<KeyType> key_range) : keys_(), key_ranges_() {
    key_ranges_.emplace_back(std::move(key_range));
  }

  /**
   * Returns the key `spanner::Row`s in the `KeySet`.
   * These keys are separate from the collection of `KeyRange`s.
   */
  std::vector<KeyType> const& keys() const { return keys_; }

  /**
   * Returns the `KeyRange`s in the `KeySet`.
   * These are separate from the collection of individual key `spanner::Row`s.
   */
  std::vector<KeyRange<KeyType>> const& key_ranges() const {
    return key_ranges_;
  }

  /**
   * Adds a key `spanner::Row` to the `KeySet`.
   *
   * @param key
   */
  void Add(KeyType key) { keys_.emplace_back(std::move(key)); }

  /**
   * Adds a `KeyRange` to the `KeySet`.
   *
   * @param key_range
   */
  void Add(KeyRange<KeyType> key_range) {
    key_ranges_.emplace_back(std::move(key_range));
  }

  /**
   * Creates a key `spanner::Row` from the values provided and adds it to the
   * `KeySet`.
   *
   * @tparam ValueTypes
   * @param values
   */
  template <typename... ValueTypes>
  void AddKey(ValueTypes... values) {
    keys_.emplace_back(std::move(MakeRow(values...)));
  }

  // TODO(sdhart): Add methods to insert ranges of Keys and KeyRanges.
  // TODO(sdhart): Add methods to remove Keys or KeyRanges.

 private:
  std::vector<KeyType> keys_;
  std::vector<KeyRange<KeyType>> key_ranges_;
};

/**
 * Helper function to create a closed `Bound` on the values provided.
 *
 * @tparam ValueTypes
 * @param values
 * @return Bound<Row<ValueTypes>>
 */
template <typename... ValueTypes>
Bound<Row<internal::PromoteLiteral<ValueTypes>...>> MakeKeyRangeBoundClosed(
    ValueTypes... values) {
  return Bound<Row<internal::PromoteLiteral<ValueTypes>...>>(
      MakeRow(values...),
      Bound<Row<internal::PromoteLiteral<ValueTypes>...>>::Mode::CLOSED);
}

/**
 * Helper function to create an open `Bound` on the values provided.
 *
 * @tparam ValueTypes
 * @param values
 * @return Bound<Row<ValueTypes>>
 */
template <typename... ValueTypes>
Bound<Row<internal::PromoteLiteral<ValueTypes>...>> MakeKeyRangeBoundOpen(
    ValueTypes... values) {
  return Bound<Row<internal::PromoteLiteral<ValueTypes>...>>(
      MakeRow(values...),
      Bound<Row<internal::PromoteLiteral<ValueTypes>...>>::Mode::OPEN);
}

/**
 * Helper function to create a `KeyRange` between two keys `spanner::Row`s with
 * both `Bound`s closed.
 *
 * @tparam RowType
 * @param start
 * @param end
 * @return
 */
template<typename RowType>
KeyRange<RowType> MakeKeyRange(RowType start, RowType end) {
  return KeyRange<RowType>(
      std::move(Bound<RowType>(start, Bound<RowType>::Mode::CLOSED)),
      std::move(Bound<RowType>(end, Bound<RowType>::Mode::CLOSED)));
}

/**
 * Helper function to create a `KeyRange` between the `Bound`s provided.
 *
 * @tparam RowType
 * @param start
 * @param end
 * @return
 */
template<typename RowType>
KeyRange<RowType> MakeKeyRange(Bound<RowType> start, Bound<RowType> end) {
  return KeyRange<RowType>(std::move(start), std::move(end));
}

}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_SPANNER_GOOGLE_CLOUD_SPANNER_KEYS_H_
