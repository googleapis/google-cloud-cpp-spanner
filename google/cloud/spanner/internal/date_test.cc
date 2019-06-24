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

#include "google/cloud/spanner/internal/date.h"
#include <gmock/gmock.h>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {
namespace internal {
inline namespace {

TEST(Date, DateToString) {
  EXPECT_EQ("2019-06-21", DateToString(Date(2019, 6, 21)));
}

TEST(Date, DateFromString) {
  EXPECT_EQ(Date(2019, 6, 21), DateFromString("2019-06-21"));
}

TEST(Date, DateFromStringFailure) {
  EXPECT_EQ(Date(), DateFromString(""));  // empty
  EXPECT_EQ(Date(), DateFromString("garbage in"));
  EXPECT_EQ(Date(), DateFromString("2018-13-02"));  // month out of range
  EXPECT_EQ(Date(), DateFromString("2019-06-32"));  // day out of range
}

}  // namespace
}  // namespace internal
}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google
