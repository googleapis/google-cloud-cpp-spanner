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

#include "google/cloud/spanner/internal/retry_loop.h"
#include "google/cloud/testing_util/assert_ok.h"
#include <gmock/gmock.h>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {
namespace internal {
namespace {

using ::testing::ElementsAre;
using ::testing::HasSubstr;
using ::testing::Return;

std::unique_ptr<RetryPolicy> TestRetryPolicy() {
  return LimitedErrorCountRetryPolicy(5).clone();
}

std::unique_ptr<BackoffPolicy> TestBackoffPolicy() {
  return ExponentialBackoffPolicy(std::chrono::microseconds(1),
                                  std::chrono::microseconds(5), 2.0)
      .clone();
}

TEST(RetryLoopTest, Success) {
  StatusOr<int> actual = RetryLoop(
      TestRetryPolicy(), TestBackoffPolicy(), true,
      [](grpc::ClientContext&, int request) {
        return StatusOr<int>(2 * request);
      },
      42, "error message");
  EXPECT_STATUS_OK(actual);
  EXPECT_EQ(84, *actual);
}

TEST(RetryLoopTest, TransientThenSuccess) {
  int counter = 0;
  StatusOr<int> actual = RetryLoop(
      TestRetryPolicy(), TestBackoffPolicy(), true,
      [&counter](grpc::ClientContext&, int request) {
        if (++counter < 3) {
          return StatusOr<int>(Status(StatusCode::kUnavailable, "try again"));
        }
        return StatusOr<int>(2 * request);
      },
      42, "error message");
  EXPECT_STATUS_OK(actual);
  EXPECT_EQ(84, *actual);
}

TEST(RetryLoopTest, ReturnJustStatus) {
  int counter = 0;
  Status actual = RetryLoop(
      TestRetryPolicy(), TestBackoffPolicy(), true,
      [&counter](grpc::ClientContext&, int) {
        if (++counter <= 3) {
          return Status(StatusCode::kResourceExhausted, "slow-down");
        }
        return Status();
      },
      42, "error message");
  EXPECT_STATUS_OK(actual);
}

class MockBackoffPolicy : public BackoffPolicy {
 public:
  MOCK_CONST_METHOD0(clone, std::unique_ptr<BackoffPolicy>());
  MOCK_METHOD0(OnCompletion, std::chrono::milliseconds());
};

/**
 * @test Verify the backoff policy is queried after each failure.
 *
 * Note that this is not testing if the
 */
TEST(RetryLoopTest, UsesBackoffPolicy) {
  using ms = std::chrono::milliseconds;

  std::unique_ptr<MockBackoffPolicy> mock(new MockBackoffPolicy);
  EXPECT_CALL(*mock, OnCompletion())
      .WillOnce(Return(ms(10)))
      .WillOnce(Return(ms(20)))
      .WillOnce(Return(ms(30)));

  int counter = 0;
  std::vector<ms> sleep_for;
  StatusOr<int> actual = RetryLoopImpl(
      TestRetryPolicy(), std::move(mock), true,
      [&counter](grpc::ClientContext&, int request) {
        if (++counter <= 3) {
          return StatusOr<int>(Status(StatusCode::kUnavailable, "try again"));
        }
        return StatusOr<int>(2 * request);
      },
      42, "error message", [&sleep_for](ms p) { sleep_for.push_back(p); });
  EXPECT_STATUS_OK(actual);
  EXPECT_EQ(84, *actual);
  EXPECT_THAT(sleep_for,
              ElementsAre(ms(10), std::chrono::milliseconds(20), ms(30)));
}

TEST(RetryLoopTest, TransientFailureNonIdempotent) {
  StatusOr<int> actual = RetryLoop(
      TestRetryPolicy(), TestBackoffPolicy(), false,
      [](grpc::ClientContext&, int) {
        return StatusOr<int>(Status(StatusCode::kUnavailable, "try again"));
      },
      42, "the answer to everything");
  EXPECT_EQ(StatusCode::kUnavailable, actual.status().code());
  EXPECT_THAT(actual.status().message(), HasSubstr("try again"));
  EXPECT_THAT(actual.status().message(), HasSubstr("the answer to everything"));
  EXPECT_THAT(actual.status().message(), HasSubstr("Error in non-idempotent"));
}

TEST(RetryLoopTest, PermanentFailureFailureIdempotent) {
  StatusOr<int> actual = RetryLoop(
      TestRetryPolicy(), TestBackoffPolicy(), true,
      [](grpc::ClientContext&, int) {
        return StatusOr<int>(Status(StatusCode::kPermissionDenied, "uh oh"));
      },
      42, "the answer to everything");
  EXPECT_EQ(StatusCode::kPermissionDenied, actual.status().code());
  EXPECT_THAT(actual.status().message(), HasSubstr("uh oh"));
  EXPECT_THAT(actual.status().message(), HasSubstr("the answer to everything"));
  EXPECT_THAT(actual.status().message(), HasSubstr("Permanent error"));
}

TEST(RetryLoopTest, TooManyTransientFailuresIdempotent) {
  StatusOr<int> actual = RetryLoop(
      TestRetryPolicy(), TestBackoffPolicy(), true,
      [](grpc::ClientContext&, int) {
        return StatusOr<int>(Status(StatusCode::kUnavailable, "try again"));
      },
      42, "the answer to everything");
  EXPECT_EQ(StatusCode::kUnavailable, actual.status().code());
  EXPECT_THAT(actual.status().message(), HasSubstr("try again"));
  EXPECT_THAT(actual.status().message(), HasSubstr("the answer to everything"));
  EXPECT_THAT(actual.status().message(), HasSubstr("Retry policy exhausted"));
}

}  // namespace
}  // namespace internal
}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google
