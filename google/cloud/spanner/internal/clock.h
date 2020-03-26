// Copyright 2020 Google LLC
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

#ifndef GOOGLE_CLOUD_CPP_SPANNER_GOOGLE_CLOUD_SPANNER_INTERNAL_CLOCK_H
#define GOOGLE_CLOUD_CPP_SPANNER_GOOGLE_CLOUD_SPANNER_INTERNAL_CLOCK_H

#include "google/cloud/spanner/version.h"
#include <chrono>
#include <mutex>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {
namespace internal {

/**
 * A simple `Clock` class that can be overridden for testing.
 *
 * All implementations of this class are required to be thread-safe.
 */
class Clock {
 public:
  // It would be better if these types were independent of any underlying
  // clock but there is no clear way to do that, so use `steady_clock`.
  using time_point = std::chrono::steady_clock::time_point;
  using duration = std::chrono::steady_clock::duration;

  virtual ~Clock() = default;
  virtual time_point Now() const = 0;
};

class SteadyClock : public Clock {
 public:
  // `Now()` can never decrease as physical time moves forward.
  Clock::time_point Now() const override {
    return std::chrono::steady_clock::now();
  }
};

class FakeClock : public Clock {
 public:
  Clock::time_point Now() const override {
    std::lock_guard<std::mutex> lock(mu_);
    return now_;
  }

  /// Sets the time to `now`.
  void SetTime(Clock::time_point now) {
    std::lock_guard<std::mutex> lock(mu_);
    now_ = now;
  }
  /// Advances the time by `increment`.
  void AdvanceTime(Clock::duration increment) {
    std::lock_guard<std::mutex> lock(mu_);
    now_ += increment;
  }

 private:
  mutable std::mutex mu_;
  Clock::time_point now_;
};

}  // namespace internal
}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_SPANNER_GOOGLE_CLOUD_SPANNER_INTERNAL_CLOCK_H
