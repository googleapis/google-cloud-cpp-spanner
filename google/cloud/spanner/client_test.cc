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

#include "google/cloud/spanner/client.h"
#include "google/cloud/spanner/connection.h"
#include "google/cloud/spanner/internal/time.h"
#include "google/cloud/spanner/mocks/mock_spanner_connection.h"
#include "google/cloud/spanner/mutations.h"
#include "google/cloud/spanner/results.h"
#include "google/cloud/spanner/timestamp.h"
#include "google/cloud/spanner/value.h"
#include "google/cloud/internal/make_unique.h"
#include "google/cloud/testing_util/assert_ok.h"
#include <google/protobuf/text_format.h>
#include <gmock/gmock.h>
#include <array>
#include <chrono>
#include <cstdint>
#include <utility>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {
namespace {

namespace spanner_proto = ::google::spanner::v1;

using ::google::cloud::internal::make_unique;
using ::google::cloud::spanner_mocks::MockConnection;
using ::google::cloud::spanner_mocks::MockResultSetSource;
using ::google::protobuf::TextFormat;
using ::testing::_;
using ::testing::ByMove;
using ::testing::DoAll;
using ::testing::ElementsAre;
using ::testing::HasSubstr;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::SaveArg;

TEST(ClientTest, CopyAndMove) {
  auto conn1 = std::make_shared<MockConnection>();
  auto conn2 = std::make_shared<MockConnection>();

  Client c1(conn1);
  Client c2(conn2);
  EXPECT_NE(c1, c2);

  // Copy construction
  Client c3 = c1;
  EXPECT_EQ(c3, c1);

  // Copy assignment
  c3 = c2;
  EXPECT_EQ(c3, c2);

  // Move construction
  Client c4 = std::move(c3);
  EXPECT_EQ(c4, c2);

  // Move assignment
  c1 = std::move(c4);
  EXPECT_EQ(c1, c2);
}

TEST(ClientTest, ReadSuccess) {
  auto conn = std::make_shared<MockConnection>();
  Client client(conn);

  auto source = make_unique<MockResultSetSource>();
  spanner_proto::ResultSetMetadata metadata;
  ASSERT_TRUE(TextFormat::ParseFromString(
      R"pb(
        row_type: {
          fields: {
            name: "Name",
            type: { code: INT64 }
          }
          fields: {
            name: "Id",
            type: { code: INT64 }
          }
        }
      )pb",
      &metadata));
  EXPECT_CALL(*source, Metadata()).WillRepeatedly(Return(metadata));
  EXPECT_CALL(*source, NextRow())
      .WillOnce(Return(MakeTestRow("Steve", 12)))
      .WillOnce(Return(MakeTestRow("Ann", 42)))
      .WillOnce(Return(Row()));

  EXPECT_CALL(*conn, Read(_))
      .WillOnce(Return(ByMove(RowStream(std::move(source)))));

  KeySet keys = KeySet::All();
  auto rows = client.Read("table", std::move(keys), {"column1", "column2"});

  using RowType = std::tuple<std::string, std::int64_t>;
  auto expected = std::vector<RowType>{
      RowType("Steve", 12),
      RowType("Ann", 42),
  };
  int row_number = 0;
  for (auto& row : StreamOf<RowType>(rows)) {
    EXPECT_STATUS_OK(row);
    EXPECT_EQ(*row, expected[row_number]);
    ++row_number;
  }
  EXPECT_EQ(row_number, expected.size());
}

TEST(ClientTest, ReadFailure) {
  auto conn = std::make_shared<MockConnection>();
  Client client(conn);

  auto source = make_unique<MockResultSetSource>();
  spanner_proto::ResultSetMetadata metadata;
  ASSERT_TRUE(TextFormat::ParseFromString(
      R"pb(
        row_type: {
          fields: {
            name: "Name",
            type: { code: INT64 }
          }
        }
      )pb",
      &metadata));
  EXPECT_CALL(*source, Metadata()).WillRepeatedly(Return(metadata));
  EXPECT_CALL(*source, NextRow())
      .WillOnce(Return(MakeTestRow("Steve")))
      .WillOnce(Return(MakeTestRow("Ann")))
      .WillOnce(Return(Status(StatusCode::kDeadlineExceeded, "deadline!")));

  EXPECT_CALL(*conn, Read(_))
      .WillOnce(Return(ByMove(RowStream(std::move(source)))));

  KeySet keys = KeySet::All();
  auto rows = client.Read("table", std::move(keys), {"column1"});

  auto tups = StreamOf<std::tuple<std::string>>(rows);
  auto iter = tups.begin();
  EXPECT_NE(iter, tups.end());
  EXPECT_STATUS_OK(*iter);
  EXPECT_EQ(std::get<0>(**iter), "Steve");

  ++iter;
  EXPECT_NE(iter, tups.end());
  EXPECT_STATUS_OK(*iter);
  EXPECT_EQ(std::get<0>(**iter), "Ann");

  ++iter;
  EXPECT_FALSE((*iter).ok());
  EXPECT_EQ((*iter).status().code(), StatusCode::kDeadlineExceeded);
}

TEST(ClientTest, ExecuteQuerySuccess) {
  auto conn = std::make_shared<MockConnection>();
  Client client(conn);

  auto source = make_unique<MockResultSetSource>();
  spanner_proto::ResultSetMetadata metadata;
  ASSERT_TRUE(TextFormat::ParseFromString(
      R"pb(
        row_type: {
          fields: {
            name: "Name",
            type: { code: INT64 }
          }
          fields: {
            name: "Id",
            type: { code: INT64 }
          }
        }
      )pb",
      &metadata));
  EXPECT_CALL(*source, Metadata()).WillRepeatedly(Return(metadata));
  EXPECT_CALL(*source, NextRow())
      .WillOnce(Return(MakeTestRow("Steve", 12)))
      .WillOnce(Return(MakeTestRow("Ann", 42)))
      .WillOnce(Return(Row()));

  EXPECT_CALL(*conn, ExecuteQuery(_))
      .WillOnce(Return(ByMove(RowStream(std::move(source)))));

  KeySet keys = KeySet::All();
  auto rows = client.ExecuteQuery(SqlStatement("select * from table;"));

  using RowType = std::tuple<std::string, std::int64_t>;
  auto expected = std::vector<RowType>{
      RowType("Steve", 12),
      RowType("Ann", 42),
  };
  int row_number = 0;
  for (auto& row : StreamOf<RowType>(rows)) {
    EXPECT_STATUS_OK(row);
    EXPECT_EQ(*row, expected[row_number]);
    ++row_number;
  }
  EXPECT_EQ(row_number, expected.size());
}

TEST(ClientTest, ExecuteQueryFailure) {
  auto conn = std::make_shared<MockConnection>();
  Client client(conn);

  auto source = make_unique<MockResultSetSource>();
  spanner_proto::ResultSetMetadata metadata;
  ASSERT_TRUE(TextFormat::ParseFromString(
      R"pb(
        row_type: {
          fields: {
            name: "Name",
            type: { code: INT64 }
          }
        }
      )pb",
      &metadata));
  EXPECT_CALL(*source, Metadata()).WillRepeatedly(Return(metadata));
  EXPECT_CALL(*source, NextRow())
      .WillOnce(Return(MakeTestRow("Steve")))
      .WillOnce(Return(MakeTestRow("Ann")))
      .WillOnce(Return(Status(StatusCode::kDeadlineExceeded, "deadline!")));

  EXPECT_CALL(*conn, ExecuteQuery(_))
      .WillOnce(Return(ByMove(RowStream(std::move(source)))));

  KeySet keys = KeySet::All();
  auto rows = client.ExecuteQuery(SqlStatement("select * from table;"));

  auto tups = StreamOf<std::tuple<std::string>>(rows);
  auto iter = tups.begin();
  EXPECT_NE(iter, tups.end());
  EXPECT_STATUS_OK(*iter);
  EXPECT_EQ(std::get<0>(**iter), "Steve");

  ++iter;
  EXPECT_NE(iter, tups.end());
  EXPECT_STATUS_OK(*iter);
  EXPECT_EQ(std::get<0>(**iter), "Ann");

  ++iter;
  EXPECT_FALSE((*iter).ok());
  EXPECT_EQ((*iter).status().code(), StatusCode::kDeadlineExceeded);
}

TEST(ClientTest, ExecuteBatchDmlSuccess) {
  auto request = {
      SqlStatement("UPDATE Foo SET Bar = 1"),
      SqlStatement("UPDATE Foo SET Bar = 1"),
      SqlStatement("UPDATE Foo SET Bar = 1"),
  };

  BatchDmlResult result;
  result.stats = std::vector<BatchDmlResult::Stats>{
      {10},
      {10},
      {10},
  };

  auto conn = std::make_shared<MockConnection>();
  EXPECT_CALL(*conn, ExecuteBatchDml(_)).WillOnce(Return(result));

  Client client(conn);
  auto txn = MakeReadWriteTransaction();
  auto actual = client.ExecuteBatchDml(txn, request);

  EXPECT_STATUS_OK(actual);
  EXPECT_STATUS_OK(actual->status);
  EXPECT_EQ(actual->stats.size(), request.size());
}

TEST(ClientTest, ExecuteBatchDmlError) {
  auto request = {
      SqlStatement("UPDATE Foo SET Bar = 1"),
      SqlStatement("UPDATE Foo SET Bar = 1"),
      SqlStatement("UPDATE Foo SET Bar = 1"),
  };

  BatchDmlResult result;
  result.status = Status(StatusCode::kUnknown, "some error");
  result.stats = std::vector<BatchDmlResult::Stats>{
      {10},
      // Oops: Only one SqlStatement was processed, then "some error"
  };

  auto conn = std::make_shared<MockConnection>();
  EXPECT_CALL(*conn, ExecuteBatchDml(_)).WillOnce(Return(result));

  Client client(conn);
  auto txn = MakeReadWriteTransaction();
  auto actual = client.ExecuteBatchDml(txn, request);

  EXPECT_STATUS_OK(actual);
  EXPECT_FALSE(actual->status.ok());
  EXPECT_EQ(actual->status.message(), "some error");
  EXPECT_NE(actual->stats.size(), request.size());
  EXPECT_EQ(actual->stats.size(), 1);
}

TEST(ClientTest, ExecutePartitionedDml_Success) {
  auto source = make_unique<MockResultSetSource>();
  spanner_proto::ResultSetMetadata metadata;
  EXPECT_CALL(*source, Metadata()).WillRepeatedly(Return(metadata));
  EXPECT_CALL(*source, NextRow()).WillRepeatedly(Return(Row()));

  std::string const sql_statement = "UPDATE Singers SET MarketingBudget = 1000";
  auto conn = std::make_shared<MockConnection>();
  EXPECT_CALL(*conn, ExecutePartitionedDml(_))
      .WillOnce([&sql_statement](
                    Connection::ExecutePartitionedDmlParams const& epdp) {
        EXPECT_EQ(sql_statement, epdp.statement.sql());
        return PartitionedDmlResult{7};
      });

  Client client(conn);
  auto result = client.ExecutePartitionedDml(SqlStatement(sql_statement));
  EXPECT_STATUS_OK(result);
  EXPECT_EQ(7, result->row_count_lower_bound);
}

TEST(ClientTest, CommitSuccess) {
  auto conn = std::make_shared<MockConnection>();

  auto ts = Timestamp(std::chrono::seconds(123));
  CommitResult result;
  result.commit_timestamp = ts;

  Client client(conn);
  EXPECT_CALL(*conn, Commit(_)).WillOnce(Return(result));

  auto txn = MakeReadWriteTransaction();
  auto commit = client.Commit(txn, {});
  EXPECT_STATUS_OK(commit);
  EXPECT_EQ(ts, commit->commit_timestamp);
}

TEST(ClientTest, CommitError) {
  auto conn = std::make_shared<MockConnection>();

  Client client(conn);
  EXPECT_CALL(*conn, Commit(_))
      .WillOnce(Return(Status(StatusCode::kPermissionDenied, "blah")));

  auto txn = MakeReadWriteTransaction();
  auto commit = client.Commit(txn, {});
  EXPECT_EQ(StatusCode::kPermissionDenied, commit.status().code());
  EXPECT_THAT(commit.status().message(), HasSubstr("blah"));
}

TEST(ClientTest, RollbackSuccess) {
  auto conn = std::make_shared<MockConnection>();

  Client client(conn);
  EXPECT_CALL(*conn, Rollback(_)).WillOnce(Return(Status()));

  auto txn = MakeReadWriteTransaction();
  auto rollback = client.Rollback(txn);
  EXPECT_TRUE(rollback.ok());
}

TEST(ClientTest, RollbackError) {
  auto conn = std::make_shared<MockConnection>();

  Client client(conn);
  EXPECT_CALL(*conn, Rollback(_))
      .WillOnce(Return(Status(StatusCode::kInvalidArgument, "oops")));

  auto txn = MakeReadWriteTransaction();
  auto rollback = client.Rollback(txn);
  EXPECT_EQ(StatusCode::kInvalidArgument, rollback.code());
  EXPECT_THAT(rollback.message(), HasSubstr("oops"));
}

TEST(ClientTest, MakeConnectionOptionalArguments) {
  Database db("foo", "bar", "baz");
  auto conn = MakeConnection(db);
  EXPECT_NE(conn, nullptr);

  conn = MakeConnection(db, ConnectionOptions());
  EXPECT_NE(conn, nullptr);
}

TEST(ClientTest, CommitMutatorSuccess) {
  auto timestamp = internal::TimestampFromString("2019-08-14T21:16:21.123Z");
  ASSERT_STATUS_OK(timestamp);

  auto conn = std::make_shared<MockConnection>();
  Transaction txn = MakeReadWriteTransaction();  // dummy
  Connection::ReadParams actual_read_params{txn, {}, {}, {}, {}};
  Connection::CommitParams actual_commit_params{txn, {}};

  auto source = make_unique<MockResultSetSource>();
  spanner_proto::ResultSetMetadata metadata;
  ASSERT_TRUE(TextFormat::ParseFromString(
      R"pb(
        row_type: {
          fields: {
            name: "Name",
            type: { code: STRING }
          }
        }
      )pb",
      &metadata));
  EXPECT_CALL(*source, Metadata()).WillRepeatedly(Return(metadata));
  EXPECT_CALL(*source, NextRow())
      .WillOnce(Return(MakeTestRow("Bob")))
      .WillOnce(Return(Row()));

  EXPECT_CALL(*conn, Read(_))
      .WillOnce(DoAll(SaveArg<0>(&actual_read_params),
                      Return(ByMove(RowStream(std::move(source))))));
  EXPECT_CALL(*conn, Commit(_))
      .WillOnce(DoAll(SaveArg<0>(&actual_commit_params),
                      Return(CommitResult{*timestamp})));

  Client client(conn);
  auto mutation = MakeDeleteMutation("table", KeySet::All());
  auto mutator = [&client, &mutation](Transaction txn) -> StatusOr<Mutations> {
    auto rows = client.Read(std::move(txn), "T", KeySet::All(), {"C"});
    for (auto& row : StreamOf<std::tuple<std::string>>(rows)) {
      if (!row) return row.status();
    }
    return Mutations{mutation};
  };

  auto result = client.Commit(mutator);
  EXPECT_STATUS_OK(result);
  EXPECT_EQ(*timestamp, result->commit_timestamp);

  EXPECT_EQ("T", actual_read_params.table);
  EXPECT_EQ(KeySet::All(), actual_read_params.keys);
  EXPECT_THAT(actual_read_params.columns, ElementsAre("C"));
  EXPECT_THAT(actual_commit_params.mutations, ElementsAre(mutation));
}

TEST(ClientTest, CommitMutatorRollback) {
  auto conn = std::make_shared<MockConnection>();
  Transaction txn = MakeReadWriteTransaction();  // dummy
  Connection::ReadParams actual_read_params{txn, {}, {}, {}, {}};

  auto source = make_unique<MockResultSetSource>();
  spanner_proto::ResultSetMetadata metadata;
  ASSERT_TRUE(TextFormat::ParseFromString(
      R"pb(
        row_type: {
          fields: {
            name: "Name",
            type: { code: INT64 }
          }
        }
      )pb",
      &metadata));
  EXPECT_CALL(*source, Metadata()).WillRepeatedly(Return(metadata));
  EXPECT_CALL(*source, NextRow())
      .WillOnce(Return(Status(StatusCode::kInvalidArgument, "blah")));

  EXPECT_CALL(*conn, Read(_))
      .WillOnce(DoAll(SaveArg<0>(&actual_read_params),
                      Return(ByMove(RowStream(std::move(source))))));
  EXPECT_CALL(*conn, Rollback(_)).WillOnce(Return(Status()));

  Client client(conn);
  auto mutation = MakeDeleteMutation("table", KeySet::All());
  auto mutator = [&client, &mutation](Transaction txn) -> StatusOr<Mutations> {
    auto rows = client.Read(std::move(txn), "T", KeySet::All(), {"C"});
    for (auto& row : rows) {
      if (!row) return row.status();
    }
    return Mutations{mutation};
  };

  auto result = client.Commit(mutator);
  EXPECT_FALSE(result.ok());
  EXPECT_EQ(StatusCode::kInvalidArgument, result.status().code());
  EXPECT_THAT(result.status().message(), HasSubstr("blah"));

  EXPECT_EQ("T", actual_read_params.table);
  EXPECT_EQ(KeySet::All(), actual_read_params.keys);
  EXPECT_THAT(actual_read_params.columns, ElementsAre("C"));
}

TEST(ClientTest, CommitMutatorRollbackError) {
  auto conn = std::make_shared<MockConnection>();
  Transaction txn = MakeReadWriteTransaction();  // dummy
  Connection::ReadParams actual_read_params{txn, {}, {}, {}, {}};

  auto source = make_unique<MockResultSetSource>();
  spanner_proto::ResultSetMetadata metadata;
  ASSERT_TRUE(TextFormat::ParseFromString(
      R"pb(
        row_type: {
          fields: {
            name: "Name",
            type: { code: INT64 }
          }
        }
      )pb",
      &metadata));
  EXPECT_CALL(*source, Metadata()).WillRepeatedly(Return(metadata));
  EXPECT_CALL(*source, NextRow())
      .WillOnce(Return(Status(StatusCode::kInvalidArgument, "blah")));

  EXPECT_CALL(*conn, Read(_))
      .WillOnce(DoAll(SaveArg<0>(&actual_read_params),
                      Return(ByMove(RowStream(std::move(source))))));
  EXPECT_CALL(*conn, Rollback(_))
      .WillOnce(Return(Status(StatusCode::kInternal, "oops")));

  Client client(conn);
  auto mutation = MakeDeleteMutation("table", KeySet::All());
  auto mutator = [&client, &mutation](Transaction txn) -> StatusOr<Mutations> {
    auto rows = client.Read(std::move(txn), "T", KeySet::All(), {"C"});
    for (auto& row : rows) {
      if (!row) return row.status();
    }
    return Mutations{mutation};
  };

  auto result = client.Commit(mutator);
  EXPECT_FALSE(result.ok());
  EXPECT_EQ(StatusCode::kInvalidArgument, result.status().code());
  EXPECT_THAT(result.status().message(), HasSubstr("blah"));

  EXPECT_EQ("T", actual_read_params.table);
  EXPECT_EQ(KeySet::All(), actual_read_params.keys);
  EXPECT_THAT(actual_read_params.columns, ElementsAre("C"));
}

#if GOOGLE_CLOUD_CPP_HAVE_EXCEPTIONS
TEST(ClientTest, CommitMutatorException) {
  auto conn = std::make_shared<MockConnection>();

  auto source = make_unique<MockResultSetSource>();
  spanner_proto::ResultSetMetadata metadata;
  ASSERT_TRUE(TextFormat::ParseFromString(
      R"pb(
        row_type: {
          fields: {
            name: "Name",
            type: { code: INT64 }
          }
        }
      )pb",
      &metadata));
  EXPECT_CALL(*source, Metadata()).WillRepeatedly(Return(metadata));
  EXPECT_CALL(*source, NextRow())
      .WillOnce(Return(Status(StatusCode::kInvalidArgument, "blah")));

  EXPECT_CALL(*conn, Read(_))
      .WillOnce(Return(ByMove(RowStream(std::move(source)))));
  EXPECT_CALL(*conn, Rollback(_)).WillOnce(Return(Status()));

  Client client(conn);
  auto mutation = MakeDeleteMutation("table", KeySet::All());
  auto mutator = [&client, &mutation](Transaction txn) -> StatusOr<Mutations> {
    auto rows = client.Read(std::move(txn), "T", KeySet::All(), {"C"});
    for (auto& row : rows) {
      if (!row) throw "Read() error";
    }
    return Mutations{mutation};
  };

  try {
    auto result = client.Commit(mutator);
    FAIL();
  } catch (char const* e) {
    EXPECT_STREQ(e, "Read() error");
  } catch (...) {
    FAIL();
  }
}
#endif

TEST(ClientTest, CommitMutatorRerunTransientFailures) {
  auto timestamp = internal::TimestampFromString("2019-08-14T21:16:21.123Z");
  ASSERT_STATUS_OK(timestamp);

  auto conn = std::make_shared<MockConnection>();
  EXPECT_CALL(*conn, Commit(_))
      .WillOnce(Invoke([](Connection::CommitParams const&) {
        return Status(StatusCode::kAborted, "Aborted transaction");
      }))
      .WillOnce(Invoke([&timestamp](Connection::CommitParams const&) {
        return CommitResult{*timestamp};
      }));

  auto mutator = [](Transaction const&) -> StatusOr<Mutations> {
    return Mutations{MakeDeleteMutation("table", KeySet::All())};
  };

  Client client(conn);
  auto result = client.Commit(mutator);
  EXPECT_STATUS_OK(result);
  EXPECT_EQ(*timestamp, result->commit_timestamp);
}

TEST(ClientTest, CommitMutatorTooManyFailures) {
  int commit_attempts = 0;
  int const maximum_failures = 2;

  auto conn = std::make_shared<MockConnection>();
  EXPECT_CALL(*conn, Commit(_))
      .WillRepeatedly(
          Invoke([&commit_attempts](Connection::CommitParams const&) {
            ++commit_attempts;
            return Status(StatusCode::kAborted, "Aborted transaction");
          }));

  auto mutator = [](Transaction const&) -> StatusOr<Mutations> {
    return Mutations{MakeDeleteMutation("table", KeySet::All())};
  };

  Client client(conn);
  // Use a rerun policy with a limited number of errors, or this will wait for a
  // long time, also change the backoff policy to sleep for very short periods,
  // so the unit tests run faster.
  auto result = client.Commit(
      mutator,
      LimitedErrorCountTransactionRerunPolicy(maximum_failures).clone(),
      ExponentialBackoffPolicy(std::chrono::microseconds(10),
                               std::chrono::microseconds(10), 2.0)
          .clone());
  EXPECT_EQ(StatusCode::kAborted, result.status().code());
  EXPECT_THAT(result.status().message(), HasSubstr("Aborted transaction"));
  EXPECT_EQ(maximum_failures + 1, commit_attempts);  // one too many
}

TEST(ClientTest, CommitMutatorPermanentFailure) {
  int commit_attempts = 0;

  auto conn = std::make_shared<MockConnection>();
  EXPECT_CALL(*conn, Commit(_))
      .WillOnce(Invoke([&commit_attempts](Connection::CommitParams const&) {
        ++commit_attempts;
        return Status(StatusCode::kPermissionDenied, "uh-oh");
      }));

  auto mutator = [](Transaction const&) -> StatusOr<Mutations> {
    return Mutations{MakeDeleteMutation("table", KeySet::All())};
  };

  Client client(conn);
  auto result = client.Commit(mutator);
  EXPECT_EQ(StatusCode::kPermissionDenied, result.status().code());
  EXPECT_THAT(result.status().message(), HasSubstr("uh-oh"));
  EXPECT_EQ(1, commit_attempts);  // no reruns
}

}  // namespace
}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google
