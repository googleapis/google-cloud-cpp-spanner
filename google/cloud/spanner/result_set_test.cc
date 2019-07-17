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

#include "google/cloud/spanner/result_set.h"
#include "google/cloud/spanner/internal/time.h"
#include "google/cloud/spanner/timestamp.h"
#include <gmock/gmock.h>
#include <chrono>
#include <string>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {
namespace {

namespace spanner_proto = ::google::spanner::v1;

using ::testing::Return;

// NOLINTNEXTLINE(clang-analyzer-optin.cplusplus.VirtualCall)
class MockResultSetValueSource : public internal::ResultSetValueSource {
 public:
  // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
  MOCK_METHOD0(NextValue, StatusOr<optional<Value>>());
  // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
  MOCK_METHOD0(Metadata, optional<spanner_proto::ResultSetMetadata>());
  // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
  MOCK_METHOD0(Stats, optional<spanner_proto::ResultSetStats>());
};

TEST(ResultSet, IterateNoRows) {
  auto* mock_source = new MockResultSetValueSource;
  ResultSet result_set{
      std::unique_ptr<internal::ResultSetValueSource>(mock_source)};

  EXPECT_CALL(*mock_source, NextValue()).WillOnce(Return(optional<Value>()));
  int num_rows = 0;
  for (auto const& row : result_set.Rows<bool>()) {
    static_cast<void>(row);
    ++num_rows;
  }
  EXPECT_EQ(num_rows, 0);
}

TEST(ResultSet, IterateOverRows) {
  auto* mock_source = new MockResultSetValueSource;
  ResultSet result_set{
      std::unique_ptr<internal::ResultSetValueSource>(mock_source)};

  EXPECT_CALL(*mock_source, NextValue())
      .WillOnce(Return(optional<Value>(5)))
      .WillOnce(Return(optional<Value>(true)))
      .WillOnce(Return(optional<Value>("foo")))
      .WillOnce(Return(optional<Value>(10)))
      .WillOnce(Return(optional<Value>(false)))
      .WillOnce(Return(optional<Value>("bar")))
      .WillOnce(Return(optional<Value>()));

  int num_rows = 0;
  for (auto const& row : result_set.Rows<std::int64_t, bool, std::string>()) {
    EXPECT_TRUE(row.ok());
    switch (num_rows++) {
      case 0:
        EXPECT_EQ(row->get<0>(), 5);
        EXPECT_EQ(row->get<1>(), true);
        EXPECT_EQ(row->get<2>(), "foo");
        break;

      case 1:
        EXPECT_EQ(row->get<0>(), 10);
        EXPECT_EQ(row->get<1>(), false);
        EXPECT_EQ(row->get<2>(), "bar");
        break;

      default:
        ADD_FAILURE() << "Unexpected row number " << num_rows;
        break;
    }
  }
  EXPECT_EQ(num_rows, 2);
}

TEST(ResultSet, IterateError) {
  auto* mock_source = new MockResultSetValueSource;
  ResultSet result_set{
      std::unique_ptr<internal::ResultSetValueSource>(mock_source)};

  EXPECT_CALL(*mock_source, NextValue())
      .WillOnce(Return(optional<Value>(5)))
      .WillOnce(Return(optional<Value>(true)))
      .WillOnce(Return(optional<Value>("foo")))
      .WillOnce(Return(optional<Value>(10)))
      .WillOnce(Return(Status(StatusCode::kUnknown, "oops")));

  int num_rows = 0;
  for (auto const& row : result_set.Rows<std::int64_t, bool, std::string>()) {
    switch (num_rows++) {
      case 0:
        EXPECT_TRUE(row.ok());
        EXPECT_EQ(row->get<0>(), 5);
        EXPECT_EQ(row->get<1>(), true);
        EXPECT_EQ(row->get<2>(), "foo");
        break;

      case 1:
        EXPECT_FALSE(row.ok());
        EXPECT_EQ(row.status().code(), StatusCode::kUnknown);
        EXPECT_EQ(row.status().message(), "oops");
        break;

      default:
        ADD_FAILURE() << "Unexpected row number " << num_rows;
        break;
    }
  }
  EXPECT_EQ(num_rows, 2);
}

TEST(ResultSet, TimestampNoTransaction) {
  auto* mock_source = new MockResultSetValueSource;
  ResultSet result_set{
      std::unique_ptr<internal::ResultSetValueSource>(mock_source)};

  spanner_proto::ResultSetMetadata no_transaction;
  EXPECT_CALL(*mock_source, Metadata()).WillOnce(Return(no_transaction));
  EXPECT_FALSE(result_set.ReadTimestamp().has_value());
}

TEST(ResultSet, TimestampNotPresent) {
  auto* mock_source = new MockResultSetValueSource;
  ResultSet result_set{
      std::unique_ptr<internal::ResultSetValueSource>(mock_source)};

  spanner_proto::ResultSetMetadata transaction_no_timestamp;
  transaction_no_timestamp.mutable_transaction()->set_id("dummy");

  EXPECT_CALL(*mock_source, Metadata())
      .WillOnce(Return(transaction_no_timestamp));
  EXPECT_FALSE(result_set.ReadTimestamp().has_value());
}

TEST(ResultSet, TimestampPresent) {
  auto* mock_source = new MockResultSetValueSource;
  ResultSet result_set{
      std::unique_ptr<internal::ResultSetValueSource>(mock_source)};

  spanner_proto::ResultSetMetadata transaction_with_timestamp;
  transaction_with_timestamp.mutable_transaction()->set_id("dummy2");
  Timestamp timestamp = std::chrono::system_clock::now();
  *transaction_with_timestamp.mutable_transaction()->mutable_read_timestamp() =
      internal::ToProto(timestamp);

  EXPECT_CALL(*mock_source, Metadata())
      .WillOnce(Return(transaction_with_timestamp));

  EXPECT_EQ(*result_set.ReadTimestamp(), timestamp);
}

TEST(ResultSet, Stats) {
  // TODO(#217) this test is a placeholder until we decide on what type to
  // return from Stats().
  auto* mock_source = new MockResultSetValueSource;
  ResultSet result_set{
      std::unique_ptr<internal::ResultSetValueSource>(mock_source)};
  EXPECT_CALL(*mock_source, Stats())
      .WillOnce(Return(spanner_proto::ResultSetStats()));
  auto stats = result_set.Stats();
  EXPECT_TRUE(stats.has_value());
}

}  // namespace
}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google
