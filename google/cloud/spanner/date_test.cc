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

#include "google/cloud/spanner/date.h"
#include <gmock/gmock.h>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {
namespace {

TEST(Date, Basics) {
  Date d(2019, 6, 21);
  EXPECT_EQ(2019, d.year());
  EXPECT_EQ(6, d.month());
  EXPECT_EQ(21, d.day());

  Date copy = d;
  EXPECT_EQ(copy, d);

  Date d2(2019, 6, 22);
  EXPECT_NE(d2, d);
}

TEST(Date, Normalization) {
  // Non-leap-year day overflow.
  Date d(2019, 2, 29);
  EXPECT_EQ(2019, d.year());
  EXPECT_EQ(3, d.month());
  EXPECT_EQ(1, d.day());

  // Non-leap-year day underflow.
  d = Date(2019, 3, 0);
  EXPECT_EQ(2019, d.year());
  EXPECT_EQ(2, d.month());
  EXPECT_EQ(28, d.day());

  // Leap-year day overflow.
  d = Date(2020, 2, 30);
  EXPECT_EQ(2020, d.year());
  EXPECT_EQ(3, d.month());
  EXPECT_EQ(1, d.day());

  // Leap-year day underflow.
  d = Date(2020, 3, 0);
  EXPECT_EQ(2020, d.year());
  EXPECT_EQ(2, d.month());
  EXPECT_EQ(29, d.day());

  // Month overflow.
  d = Date(2016, 25, 28);
  EXPECT_EQ(2018, d.year());
  EXPECT_EQ(1, d.month());
  EXPECT_EQ(28, d.day());

  // Month underflow.
  d = Date(2016, -25, 28);
  EXPECT_EQ(2013, d.year());
  EXPECT_EQ(11, d.month());
  EXPECT_EQ(28, d.day());

  // Four-century overflow.
  d = Date(2016, 1, 292195);
  EXPECT_EQ(2816, d.year());
  EXPECT_EQ(1, d.month());
  EXPECT_EQ(1, d.day());

  // Four-century underflow.
  d = Date(2016, 1, -292195);
  EXPECT_EQ(1215, d.year());
  EXPECT_EQ(12, d.month());
  EXPECT_EQ(30, d.day());

  // Mixed.
  d = Date(2016, -42, 122);
  EXPECT_EQ(2012, d.year());
  EXPECT_EQ(9, d.month());
  EXPECT_EQ(30, d.day());
}

}  // namespace
}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google
