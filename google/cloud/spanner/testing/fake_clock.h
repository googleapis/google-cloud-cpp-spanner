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

#ifndef GOOGLE_CLOUD_CPP_SPANNER_GOOGLE_CLOUD_SPANNER_TESTING_FAKE_CLOCK_H
#define GOOGLE_CLOUD_CPP_SPANNER_GOOGLE_CLOUD_SPANNER_TESTING_FAKE_CLOCK_H

#include "google/cloud/spanner/internal/clock.h"
#include "google/cloud/spanner/version.h"
#include <chrono>
#include <memory>
#include <mutex>

namespace google {
namespace cloud {
namespace spanner_testing {

/**
 * A helper to provide a fake Clock for testing.
 *
 * This class is thread safe.
 */
class FakeClock {
 public:
  using time_point = google::cloud::spanner::internal::Clock::time_point;
  using duration = google::cloud::spanner::internal::Clock::duration;

  /// Construct a `FakeClock` with an optional starting `time_point` of `now`.
  explicit FakeClock(time_point now = {}) : state_(std::make_shared<State>()) {
    state_->now = now;
  }

  time_point now() const {
    std::lock_guard<std::mutex> lock(state_->mu);
    return state_->now;
  }
  time_point operator()() const { return now(); }

  /// Sets the time to `now`.
  void Set(time_point now) {
    std::lock_guard<std::mutex> lock(state_->mu);
    state_->now = now;
  }
  /// Advances the time by `increment`.
  void Advance(duration increment) {
    std::lock_guard<std::mutex> lock(state_->mu);
    state_->now += increment;
  }

 private:
  struct State {
    std::mutex mu;
    time_point now;
  };
  std::shared_ptr<State> state_;
};

}  // namespace spanner_testing
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_SPANNER_GOOGLE_CLOUD_SPANNER_TESTING_FAKE_CLOCK_H
