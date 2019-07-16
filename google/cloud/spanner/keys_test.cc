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

#include "google/cloud/spanner/keys.h"
#include <gmock/gmock.h>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {
namespace {

TEST(KeyRangeTest, ConstructorBoundModeUnspecified) {
  std::string start_value("key0");
  std::string end_value("key1");
  KeyRange<Row<std::string>> closed_range =
      KeyRange<Row<std::string>>(MakeRow(start_value), MakeRow(end_value));
  EXPECT_EQ(start_value, closed_range.StartKey().get<0>());
  EXPECT_TRUE(closed_range.IsStartClosed());
  EXPECT_EQ(end_value, closed_range.EndKey().get<0>());
  EXPECT_TRUE(closed_range.IsEndClosed());
}

TEST(KeyRangeBoundTest, MakeKeyRangeBoundClosed) {
  std::string key_value("key0");
  auto bound = MakeKeyRangeBoundClosed(key_value);
  EXPECT_EQ(key_value, bound.Key().get<0>());
  EXPECT_TRUE(bound.IsClosed());
}

TEST(KeyRangeBoundTest, MakeKeyRangeBoundOpen) {
  std::string key_value_0("key0");
  std::int64_t key_value_1(42);
  auto bound = MakeKeyRangeBoundOpen(key_value_0, key_value_1);
  EXPECT_EQ(key_value_0, bound.Key().get<0>());
  EXPECT_EQ(key_value_1, bound.Key().get<1>());
  EXPECT_TRUE(bound.IsOpen());
}

TEST(KeyRangeTest, ConstructorClosedClosed) {
  std::string start_value("key0");
  std::string end_value("key1");
  auto start_bound = MakeKeyRangeBoundClosed(start_value);
  auto end_bound = MakeKeyRangeBoundClosed(end_value);
  auto closed_range = KeyRange<Row<std::string>>(start_bound, end_bound);
  EXPECT_EQ(start_value, closed_range.StartKey().get<0>());
  EXPECT_TRUE(closed_range.IsStartClosed());
  EXPECT_EQ(end_value, closed_range.EndKey().get<0>());
  EXPECT_TRUE(closed_range.IsEndClosed());
}

TEST(KeyRangeTest, ConstructorClosedOpen) {
  std::string start_value("key0");
  std::string end_value("key1");
  auto range = KeyRange<Row<std::string>>(MakeKeyRangeBoundClosed(start_value),
                                          MakeKeyRangeBoundOpen(end_value));
  EXPECT_EQ(start_value, range.StartKey().get<0>());
  EXPECT_TRUE(range.IsStartClosed());
  EXPECT_EQ(end_value, range.EndKey().get<0>());
  EXPECT_TRUE(range.IsEndOpen());
}

TEST(KeyRangeTest, ConstructorOpenClosed) {
  std::string start_value("key0");
  std::string end_value("key1");
  auto range = KeyRange<Row<std::string>>(MakeKeyRangeBoundOpen(start_value),
                                          MakeKeyRangeBoundClosed(end_value));
  EXPECT_EQ(start_value, range.StartKey().get<0>());
  EXPECT_TRUE(range.IsStartOpen());
  EXPECT_EQ(end_value, range.EndKey().get<0>());
  EXPECT_TRUE(range.IsEndClosed());
}

TEST(KeyRangeTest, ConstructorOpenOpen) {
  std::string start_value("key0");
  std::string end_value("key1");
  auto range = KeyRange<Row<std::string>>(MakeKeyRangeBoundOpen(start_value),
                                          MakeKeyRangeBoundOpen(end_value));
  EXPECT_EQ(start_value, range.StartKey().get<0>());
  EXPECT_TRUE(range.IsStartOpen());
  EXPECT_EQ(end_value, range.EndKey().get<0>());
  EXPECT_TRUE(range.IsEndOpen());
}

TEST(KeySetTest, ConstructorSingleKey) {
  std::string expected_value("key0");
  auto key = MakeRow("key0");
  auto ks = KeySet<Row<std::string>>(key);
  EXPECT_EQ(expected_value, ks.keys()[0].get<0>());
}

TEST(KeySetTest, ConstructorKeyRange) {
  std::string start_value("key0");
  std::string end_value("key1");
  auto ks = KeySet<Row<std::string>>(
      KeyRange<Row<std::string>>(MakeKeyRangeBoundClosed(start_value),
                                 MakeKeyRangeBoundClosed(end_value)));
  EXPECT_EQ(start_value, ks.key_ranges()[0].StartKey().get<0>());
  EXPECT_TRUE(ks.key_ranges()[0].IsStartClosed());
  EXPECT_EQ(end_value, ks.key_ranges()[0].EndKey().get<0>());
  EXPECT_TRUE(ks.key_ranges()[0].IsEndClosed());
}

TEST(KeySetTest, AddKeyToEmptyKeySet) {
  auto ks = KeySet<Row<std::int64_t, std::string>>();
  ks.Add(MakeRow(42, "key42"));
  EXPECT_EQ(42, ks.keys()[0].get<0>());
  EXPECT_EQ("key42", ks.keys()[0].get<1>());
}

TEST(KeySetTest, AddKeyToNonEmptyKeySet) {
  auto ks = KeySet<Row<std::int64_t, std::string>>(MakeRow(84, "key84"));
  ks.AddKey(42, "key42");
  EXPECT_EQ(84, ks.keys()[0].get<0>());
  EXPECT_EQ("key84", ks.keys()[0].get<1>());
  EXPECT_EQ(42, ks.keys()[1].get<0>());
  EXPECT_EQ("key42", ks.keys()[1].get<1>());
}

TEST(KeySetTest, AddKeyRangeToEmptyKeySet) {
  auto ks = KeySet<Row<std::string, std::string>>();
  auto range = KeyRange<Row<std::string, std::string>>(
      MakeKeyRangeBoundClosed("start00", "start01"),
      MakeKeyRangeBoundClosed("end00", "end01"));
  ks.Add(range);
  EXPECT_EQ("start00", ks.key_ranges()[0].StartKey().get<0>());
  EXPECT_EQ("start01", ks.key_ranges()[0].StartKey().get<1>());
  EXPECT_EQ("end00", ks.key_ranges()[0].EndKey().get<0>());
  EXPECT_EQ("end01", ks.key_ranges()[0].EndKey().get<1>());
  EXPECT_TRUE(ks.key_ranges()[0].IsStartClosed());
  EXPECT_TRUE(ks.key_ranges()[0].IsEndClosed());
}

TEST(KeySetTest, AddKeyRangeToNonEmptyKeySet) {
  auto ks = KeySet<Row<std::string, std::string>>(
      KeyRange<Row<std::string, std::string>>(MakeRow("start00", "start01"),
                                              MakeRow("end00", "end01")));
  auto range = KeyRange<Row<std::string, std::string>>(
      MakeKeyRangeBoundOpen("start10", "start11"),
      MakeKeyRangeBoundOpen("end10", "end11"));
  ks.Add(range);
  EXPECT_EQ("start00", ks.key_ranges()[0].StartKey().get<0>());
  EXPECT_EQ("start01", ks.key_ranges()[0].StartKey().get<1>());
  EXPECT_EQ("end00", ks.key_ranges()[0].EndKey().get<0>());
  EXPECT_EQ("end01", ks.key_ranges()[0].EndKey().get<1>());
  EXPECT_TRUE(ks.key_ranges()[0].IsStartClosed());
  EXPECT_TRUE(ks.key_ranges()[0].IsEndClosed());
  EXPECT_EQ("start10", ks.key_ranges()[1].StartKey().get<0>());
  EXPECT_EQ("start11", ks.key_ranges()[1].StartKey().get<1>());
  EXPECT_EQ("end10", ks.key_ranges()[1].EndKey().get<0>());
  EXPECT_EQ("end11", ks.key_ranges()[1].EndKey().get<1>());
  EXPECT_TRUE(ks.key_ranges()[1].IsStartOpen());
  EXPECT_TRUE(ks.key_ranges()[1].IsEndOpen());
}

}  // namespace
}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google
