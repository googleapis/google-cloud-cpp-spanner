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

#ifndef GOOGLE_CLOUD_CPP_SPANNER_GOOGLE_CLOUD_SPANNER_TIMESTAMP_H_
#define GOOGLE_CLOUD_CPP_SPANNER_GOOGLE_CLOUD_SPANNER_TIMESTAMP_H_

#include "google/cloud/spanner/version.h"
#include "google/cloud/status_or.h"
#include <google/protobuf/timestamp.pb.h>
#include <chrono>
#include <cstdint>
#include <limits>
#include <ostream>
#include <string>
#include <tuple>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {

/**
 * A representation of the Spanner TIMESTAMP type: An instant in time.
 *
 * A `Timestamp` represents an absolute point in time (i.e., is independent of
 * any time zone), with at least nanosecond precision, and with a range of at
 * least 0001-01-01T00:00:00Z to 9999-12-31T23:59:59.999999999Z.
 *
 * Note that the string and protobuf representations share those precision and
 * range guarantees.
 *
 * The `std::chrono::time_point` conversions depend on the precision/range of
 * the underlying duration and on the `std::chrono::system_clock` epoch.
 */
class Timestamp {
 private:
  // Convenience alias. `std::chrono::sys_time` since C++20.
  template <class Duration>
  using sys_time = std::chrono::time_point<std::chrono::system_clock, Duration>;

 public:
  /// Default construction yields the Unix epoch.
  Timestamp() : Timestamp(0, 0) {}

  /// Conversion from/to RFC3339 string.
  ///@{
  static StatusOr<Timestamp> FromRFC3339(std::string const&);
  std::string ToRFC3339() const;
  ///@}

  /// Conversion from/to `google::protobuf::Timestamp`. These conversions never
  /// fail, but may accept/produce protobufs outside their documented range.
  ///@{
  static Timestamp FromProto(google::protobuf::Timestamp const&);
  google::protobuf::Timestamp ToProto() const;
  ///@}

  /// Conversion from/to `std::chrono::time_point` on the system clock.
  /// May produce out-of-range errors in either direction, depending on the
  /// properties of `Duration` and the `std::chrono::system_clock` epoch.
  ///@{
  template <class Duration>
  static StatusOr<Timestamp> FromChrono(sys_time<Duration> const& tp) {
    return FromRatio((tp - UnixEpoch<Duration>()).count(),
                     Duration::period::num, Duration::period::den);
  }
  template <class Duration>
  StatusOr<sys_time<Duration>> ToChrono() const {
    auto const count =
        ToRatio(std::numeric_limits<typename Duration::rep>::min(),
                std::numeric_limits<typename Duration::rep>::max(),
                Duration::period::num, Duration::period::den);
    if (!count) return count.status();
    return UnixEpoch<Duration>() +
           Duration(static_cast<typename Duration::rep>(*count));
  }
  ///@}

  /// Build a `Timestamp` from raw counts of seconds/nanoseconds since the Unix
  /// epoch (minus leap seconds), like a C++17 `std::timespec` but out-of-range
  /// `nsec` values are normalized. Fails if the time cannot be represented.
  static StatusOr<Timestamp> FromCounts(std::intmax_t sec, std::intmax_t nsec);

  /// Factory functions for special `Timestamp` values.
  ///@{
  static Timestamp Min();
  static Timestamp Max();
  ///@}

  /// @name Relational operators
  ///@{
  friend bool operator==(Timestamp const& a, Timestamp const& b) {
    return std::make_tuple(a.sec_, a.nsec_) == std::make_tuple(b.sec_, b.nsec_);
  }
  friend bool operator!=(Timestamp const& a, Timestamp const& b) {
    return !(a == b);
  }
  friend bool operator<(Timestamp const& a, Timestamp const& b) {
    return std::make_tuple(a.sec_, a.nsec_) < std::make_tuple(b.sec_, b.nsec_);
  }
  friend bool operator<=(Timestamp const& a, Timestamp const& b) {
    return !(b < a);
  }
  friend bool operator>=(Timestamp const& a, Timestamp const& b) {
    return !(a < b);
  }
  friend bool operator>(Timestamp const& a, Timestamp const& b) {
    return b < a;
  }
  ///@}

  /// @name Output streaming
  friend std::ostream& operator<<(std::ostream& os, Timestamp ts) {
    return os << ts.ToRFC3339();
  }

 private:
  // Arguments must have already been normalized.
  Timestamp(std::int64_t sec, std::int32_t nsec) : sec_(sec), nsec_(nsec) {}

  // Helpers for `FromChrono()` and `ToChrono()`.
  template <class Duration>
  static sys_time<Duration> UnixEpoch() {
    return std::chrono::time_point_cast<Duration>(
        sys_time<Duration>::clock::from_time_t(0));
  }
  static StatusOr<Timestamp> FromRatio(std::intmax_t const count,
                                       std::intmax_t const numerator,
                                       std::intmax_t const denominator);
  StatusOr<std::intmax_t> ToRatio(std::intmax_t min, std::intmax_t max,
                                  std::intmax_t const numerator,
                                  std::intmax_t const denominator) const;

  std::int64_t sec_;   // The number of seconds that have elapsed since
                       // 1970-01-01T00:00:00Z, minus leap seconds.
  std::int32_t nsec_;  // The number of nanoseconds [0..999999999] that
                       // have elapsed within that second.
};

}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_SPANNER_GOOGLE_CLOUD_SPANNER_TIMESTAMP_H_
