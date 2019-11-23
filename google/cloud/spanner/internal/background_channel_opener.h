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

#ifndef GOOGLE_CLOUD_CPP_SPANNER_GOOGLE_CLOUD_SPANNER_INTERNAL_BACKGROUND_CHANNEL_OPENER_H_
#define GOOGLE_CLOUD_CPP_SPANNER_GOOGLE_CLOUD_SPANNER_INTERNAL_BACKGROUND_CHANNEL_OPENER_H_

#include "google/cloud/spanner/background_threads.h"
#include "google/cloud/grpc_utils/completion_queue.h"
#include <google/longrunning/operations.grpc.pb.h>
#include <chrono>
#include <thread>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {
namespace internal {

/**
 * A function for trying to open the grpc channel in background.
 */
void BackgroundChannelOpener(
    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    grpc_utils::CompletionQueue cq, std::shared_ptr<grpc::Channel> channel) {
  using ms = std::chrono::milliseconds;
  // TODO(xxx): Find out if this blocks.
  auto state = channel->GetState(true);
  if (state != GRPC_CHANNEL_CONNECTING && state != GRPC_CHANNEL_READY) {
    // Reschedule ourselves to run in 200 ms
    cq.MakeRelativeTimer(ms(200)).then(
        [cq, channel](future<std::chrono::system_clock::time_point>) {
          // NOLINTNEXTLINE(performance-move-const-arg)
          BackgroundChannelOpener(std::move(cq), std::move(channel));
        });
  }
}

}  // namespace internal
}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_SPANNER_GOOGLE_CLOUD_SPANNER_INTERNAL_BACKGROUND_CHANNEL_OPENER_H_
