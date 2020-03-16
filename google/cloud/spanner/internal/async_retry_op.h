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

#ifndef GOOGLE_CLOUD_CPP_SPANNER_GOOGLE_CLOUD_SPANNER_INTERNAL_ASYNC_RETRY_OP_H
#define GOOGLE_CLOUD_CPP_SPANNER_GOOGLE_CLOUD_SPANNER_INTERNAL_ASYNC_RETRY_OP_H

#include "google/cloud/spanner/backoff_policy.h"
#include "google/cloud/spanner/retry_policy.h"
#include "google/cloud/spanner/version.h"
#include "google/cloud/completion_queue.h"
#include "google/cloud/internal/make_unique.h"
#include "google/cloud/internal/throw_delegate.h"

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {
namespace internal {

/**
 * An idempotent policy for `AsyncRetryOp` based on a pre-computed value.
 *
 * In most APIs the idempotency of the API is either known at compile-time or
 * the value is unchanged during the retry loop. This class can be used in
 * those cases as the `IdempotentPolicy` template parameter for
 * `AsyncRetryOp`.
 */
class ConstantIdempotencyPolicy {
 public:
  explicit ConstantIdempotencyPolicy(bool is_idempotent)
      : is_idempotent_(is_idempotent) {}

  bool is_idempotent() const { return is_idempotent_; }

 private:
  bool is_idempotent_;
};

}  // namespace internal
}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_SPANNER_GOOGLE_CLOUD_SPANNER_INTERNAL_ASYNC_RETRY_OP_H
