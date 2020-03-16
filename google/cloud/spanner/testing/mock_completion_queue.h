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

#ifndef GOOGLE_CLOUD_CPP_SPANNER_GOOGLE_CLOUD_SPANNER_TESTING_MOCK_COMPLETION_QUEUE_H
#define GOOGLE_CLOUD_CPP_SPANNER_GOOGLE_CLOUD_SPANNER_TESTING_MOCK_COMPLETION_QUEUE_H

#include "google/cloud/completion_queue.h"

namespace google {
namespace cloud {
namespace spanner {
namespace testing {
// Tests typically create an instance of this class, then create a
// `google::cloud::CompletionQueue` to wrap it, keeping a reference to
// the instance to manipulate its state directly.
class MockCompletionQueue
    : public google::cloud::internal::CompletionQueueImpl {
 public:
  std::unique_ptr<grpc::Alarm> CreateAlarm() const override {
    // grpc::Alarm objects are really hard to cleanup when mocking their
    // behavior, so we do not create an alarm, instead we return nullptr, which
    // the classes that care (AsyncTimerFunctor) know what to do with.
    return std::unique_ptr<grpc::Alarm>();
  }

  using CompletionQueueImpl::empty;
  using CompletionQueueImpl::SimulateCompletion;
  using CompletionQueueImpl::size;
};
}  // namespace testing
}  // namespace spanner
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_SPANNER_GOOGLE_CLOUD_SPANNER_TESTING_MOCK_COMPLETION_QUEUE_H
