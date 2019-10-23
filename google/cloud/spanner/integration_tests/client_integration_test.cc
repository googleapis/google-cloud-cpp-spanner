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
#include "google/cloud/spanner/database.h"
#include "google/cloud/spanner/database_admin_client.h"
#include "google/cloud/spanner/mutations.h"
#include "google/cloud/spanner/testing/database_environment.h"
#include "google/cloud/internal/getenv.h"
#include "google/cloud/internal/random.h"
#include "google/cloud/testing_util/assert_ok.h"
#include "google/cloud/testing_util/init_google_mock.h"
#include <gmock/gmock.h>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {
namespace {

using ::testing::UnorderedElementsAre;
using ::testing::UnorderedElementsAreArray;

class ClientIntegrationTest : public ::testing::Test {
 public:
  static void SetUpTestSuite() {
    client_ = google::cloud::internal::make_unique<Client>(
        MakeConnection(spanner_testing::DatabaseEnvironment::GetDatabase()));
  }

  void SetUp() override {
    auto commit_result = client_->Commit([](Transaction const&) {
      return Mutations{MakeDeleteMutation("Singers", KeySet::All())};
    });
    EXPECT_STATUS_OK(commit_result);
  }

  void InsertTwoSingers() {
    auto commit_result = client_->Commit([](Transaction const&) {
      return Mutations{InsertMutationBuilder(
                           "Singers", {"SingerId", "FirstName", "LastName"})
                           .EmplaceRow(1, "test-fname-1", "test-lname-1")
                           .EmplaceRow(2, "test-fname-2", "test-lname-2")
                           .Build()};
    });
    ASSERT_STATUS_OK(commit_result);
  }

  static void TearDownTestSuite() { client_ = nullptr; }

 protected:
  static std::unique_ptr<Client> client_;
};

std::unique_ptr<Client> ClientIntegrationTest::client_;

/// @test Verify the basic insert operations for transaction commits.
TEST_F(ClientIntegrationTest, InsertAndCommit) {
  ASSERT_NO_FATAL_FAILURE(InsertTwoSingers());

  auto reader = client_->Read("Singers", KeySet::All(),
                              {"SingerId", "FirstName", "LastName"});
  using RowType = std::tuple<std::int64_t, std::string, std::string>;
  std::vector<RowType> returned_rows;
  int row_number = 0;
  for (auto& row : reader.Rows<RowType>()) {
    EXPECT_STATUS_OK(row);
    if (!row) break;
    SCOPED_TRACE("Parsing row[" + std::to_string(row_number++) + "]");
    returned_rows.push_back(*std::move(row));
  }
  EXPECT_THAT(returned_rows,
              UnorderedElementsAre(RowType(1, "test-fname-1", "test-lname-1"),
                                   RowType(2, "test-fname-2", "test-lname-2")));
}

/// @test Verify the basic delete mutations work.
TEST_F(ClientIntegrationTest, DeleteAndCommit) {
  ASSERT_NO_FATAL_FAILURE(InsertTwoSingers());

  auto commit_result = client_->Commit([](Transaction const&) {
    return Mutations{
        MakeDeleteMutation("Singers", KeySet().AddKey(MakeKey(1)))};
  });
  EXPECT_STATUS_OK(commit_result);

  auto reader = client_->Read("Singers", KeySet::All(),
                              {"SingerId", "FirstName", "LastName"});

  using RowType = std::tuple<std::int64_t, std::string, std::string>;
  std::vector<RowType> returned_rows;
  int row_number = 0;
  for (auto& row : reader.Rows<RowType>()) {
    EXPECT_STATUS_OK(row);
    if (!row) break;
    SCOPED_TRACE("Parsing row[" + std::to_string(row_number++) + "]");
    returned_rows.push_back(*std::move(row));
  }
  EXPECT_THAT(returned_rows,
              UnorderedElementsAre(RowType(2, "test-fname-2", "test-lname-2")));
}

/// @test Verify that read-write transactions with multiple statements work.
TEST_F(ClientIntegrationTest, MultipleInserts) {
  ASSERT_NO_FATAL_FAILURE(InsertTwoSingers());

  auto& client = *client_;
  auto commit_result =
      client_->Commit([&client](Transaction const& txn) -> StatusOr<Mutations> {
        auto insert1 = client.ExecuteDml(
            txn,
            SqlStatement("INSERT INTO Singers (SingerId, FirstName, LastName) "
                         "VALUES (@id, @fname, @lname)",
                         {{"id", Value(3)},
                          {"fname", Value("test-fname-3")},
                          {"lname", Value("test-lname-3")}}));
        if (!insert1) return std::move(insert1).status();
        auto insert2 = client.ExecuteDml(
            txn,
            SqlStatement("INSERT INTO Singers (SingerId, FirstName, LastName) "
                         "VALUES (@id, @fname, @lname)",
                         {{"id", Value(4)},
                          {"fname", Value("test-fname-4")},
                          {"lname", Value("test-lname-4")}}));
        if (!insert2) return std::move(insert2).status();
        return Mutations{};
      });
  EXPECT_STATUS_OK(commit_result);

  auto reader = client_->Read("Singers", KeySet::All(),
                              {"SingerId", "FirstName", "LastName"});

  using RowType = std::tuple<std::int64_t, std::string, std::string>;
  std::vector<RowType> returned_rows;
  int row_number = 0;
  for (auto& row : reader.Rows<RowType>()) {
    EXPECT_STATUS_OK(row);
    if (!row) break;
    SCOPED_TRACE("Parsing row[" + std::to_string(row_number++) + "]");
    returned_rows.push_back(*std::move(row));
  }
  EXPECT_THAT(returned_rows,
              UnorderedElementsAre(RowType(1, "test-fname-1", "test-lname-1"),
                                   RowType(2, "test-fname-2", "test-lname-2"),
                                   RowType(3, "test-fname-3", "test-lname-3"),
                                   RowType(4, "test-fname-4", "test-lname-4")));
}

/// @test Verify that Client::Rollback works as expected.
TEST_F(ClientIntegrationTest, TransactionRollback) {
  ASSERT_NO_FATAL_FAILURE(InsertTwoSingers());

  using RowType = std::tuple<std::int64_t, std::string, std::string>;

  // Cannot use Commit in this test because we want to call Rollback
  // explicitly. This means we need to retry ABORTED calls ourselves.
  for (auto start = std::chrono::steady_clock::now(),
            deadline = start + std::chrono::minutes(1);
       start < deadline; start = std::chrono::steady_clock::now()) {
    auto is_retryable_failure = [](StatusOr<DmlResult> const& s) {
      return !s && s.status().code() == StatusCode::kAborted;
    };

    Transaction txn = MakeReadWriteTransaction();
    auto insert1 = client_->ExecuteDml(
        txn, SqlStatement("INSERT INTO Singers (SingerId, FirstName, LastName) "
                          "VALUES (@id, @fname, @lname)",
                          {{"id", Value(3)},
                           {"fname", Value("test-fname-3")},
                           {"lname", Value("test-lname-3")}}));
    if (is_retryable_failure(insert1)) continue;
    ASSERT_STATUS_OK(insert1);

    auto insert2 = client_->ExecuteDml(
        txn, SqlStatement("INSERT INTO Singers (SingerId, FirstName, LastName) "
                          "VALUES (@id, @fname, @lname)",
                          {{"id", Value(4)},
                           {"fname", Value("test-fname-4")},
                           {"lname", Value("test-lname-4")}}));
    if (is_retryable_failure(insert2)) continue;
    ASSERT_STATUS_OK(insert2);

    auto reader = client_->Read(txn, "Singers", KeySet::All(),
                                {"SingerId", "FirstName", "LastName"});

    std::vector<RowType> returned_rows;
    int row_number = 0;
    for (auto& row : reader.Rows<RowType>()) {
      if (!row) break;
      SCOPED_TRACE("Parsing row[" + std::to_string(row_number++) + "]");
      returned_rows.push_back(*std::move(row));
    }
    ASSERT_THAT(returned_rows, UnorderedElementsAre(
                                   RowType(1, "test-fname-1", "test-lname-1"),
                                   RowType(2, "test-fname-2", "test-lname-2"),
                                   RowType(3, "test-fname-3", "test-lname-3"),
                                   RowType(4, "test-fname-4", "test-lname-4")));

    auto insert_rollback_result = client_->Rollback(txn);
    ASSERT_STATUS_OK(insert_rollback_result);
    break;
  }

  std::vector<RowType> returned_rows;
  auto reader = client_->Read("Singers", KeySet::All(),
                              {"SingerId", "FirstName", "LastName"});
  int row_number = 0;
  for (auto& row : reader.Rows<RowType>()) {
    EXPECT_STATUS_OK(row);
    if (!row) break;
    SCOPED_TRACE("Parsing row[" + std::to_string(row_number++) + "]");
    returned_rows.push_back(*std::move(row));
  }
  EXPECT_THAT(returned_rows,
              UnorderedElementsAre(RowType(1, "test-fname-1", "test-lname-1"),
                                   RowType(2, "test-fname-2", "test-lname-2")));
}

/// @test Verify the basics of Commit().
TEST_F(ClientIntegrationTest, Commit) {
  // Insert SingerIds 100, 102, and 199.
  auto inserter = [](Transaction const&) {
    auto isb =
        InsertMutationBuilder("Singers", {"SingerId", "FirstName", "LastName"})
            .EmplaceRow(100, "first-name-100", "last-name-100")
            .EmplaceRow(102, "first-name-102", "last-name-102")
            .EmplaceRow(199, "first-name-199", "last-name-199");
    return Mutations{isb.Build()};
  };
  auto insert_result = client_->Commit(inserter);
  EXPECT_STATUS_OK(insert_result);
  EXPECT_NE(Timestamp{}, insert_result->commit_timestamp);

  // Delete SingerId 102.
  auto deleter = [](Transaction const&) {
    auto mutation =
        MakeDeleteMutation("Singers", KeySet().AddKey(MakeKey(102)));
    return Mutations{mutation};
  };
  auto delete_result = client_->Commit(deleter);
  EXPECT_STATUS_OK(delete_result);
  EXPECT_LT(insert_result->commit_timestamp, delete_result->commit_timestamp);

  // Read SingerIds [100 ... 200).
  using RowType = std::tuple<std::int64_t>;
  std::vector<std::int64_t> ids;
  auto ks = KeySet().AddRange(MakeKeyBoundClosed(100), MakeKeyBoundOpen(200));
  auto results = client_->Read("Singers", std::move(ks), {"SingerId"});
  for (auto& row : results.Rows<RowType>()) {
    EXPECT_STATUS_OK(row);
    if (row) ids.push_back(std::get<0>(*row));
  }
  EXPECT_THAT(ids, UnorderedElementsAre(100, 199));
}

/// @test Test various forms of ExecuteQuery() and ExecuteDml()
TEST_F(ClientIntegrationTest, ExecuteQueryDml) {
  auto& client = *client_;
  auto insert_result =
      client_->Commit([&client](Transaction txn) -> StatusOr<Mutations> {
        auto insert = client.ExecuteDml(
            std::move(txn), SqlStatement(R"sql(
        INSERT INTO Singers (SingerId, FirstName, LastName)
        VALUES (@id, @fname, @lname))sql",
                                         {{"id", Value(1)},
                                          {"fname", Value("test-fname-1")},
                                          {"lname", Value("test-lname-1")}}));
        if (!insert) return std::move(insert).status();
        return Mutations{};
      });
  EXPECT_STATUS_OK(insert_result);

  using RowType = std::tuple<std::int64_t, std::string, std::string>;
  std::vector<RowType> expected_rows;
  auto commit_result = client_->Commit(
      [&client, &expected_rows](Transaction const& txn) -> StatusOr<Mutations> {
        expected_rows.clear();
        for (int i = 2; i != 10; ++i) {
          auto s = std::to_string(i);
          auto insert = client.ExecuteDml(
              txn, SqlStatement(R"sql(
        INSERT INTO Singers (SingerId, FirstName, LastName)
        VALUES (@id, @fname, @lname))sql",
                                {{"id", Value(i)},
                                 {"fname", Value("test-fname-" + s)},
                                 {"lname", Value("test-lname-" + s)}}));
          if (!insert) return std::move(insert).status();
          expected_rows.emplace_back(
              std::make_tuple(i, "test-fname-" + s, "test-lname-" + s));
        }

        auto delete_result =
            client.ExecuteDml(txn, SqlStatement(R"sql(
        DELETE FROM Singers WHERE SingerId = @id)sql",
                                                {{"id", Value(1)}}));
        if (!delete_result) return std::move(delete_result).status();

        return Mutations{};
      });
  ASSERT_STATUS_OK(commit_result);

  auto reader = client_->ExecuteQuery(
      SqlStatement("SELECT SingerId, FirstName, LastName FROM Singers", {}));

  std::vector<RowType> actual_rows;
  int row_number = 0;
  for (auto& row : reader.Rows<RowType>()) {
    EXPECT_STATUS_OK(row);
    if (!row) break;
    SCOPED_TRACE("Parsing row[" + std::to_string(row_number++) + "]");
    actual_rows.push_back(*std::move(row));
  }
  EXPECT_THAT(actual_rows, UnorderedElementsAreArray(expected_rows));
}

/// @test Test ExecutePartitionedDml
TEST_F(ClientIntegrationTest, ExecutePartitionedDml) {
  auto& client = *client_;
  auto insert_result =
      client_->Commit([&client](Transaction txn) -> StatusOr<Mutations> {
        auto insert = client.ExecuteDml(
            std::move(txn), SqlStatement(R"sql(
        INSERT INTO Singers (SingerId, FirstName, LastName)
        VALUES (@id, @fname, @lname))sql",
                                         {{"id", Value(1)},
                                          {"fname", Value("test-fname-1")},
                                          {"lname", Value("test-lname-1")}}));
        if (!insert) return std::move(insert).status();
        return Mutations{};
      });
  EXPECT_STATUS_OK(insert_result);

  auto result = client_->ExecutePartitionedDml(
      SqlStatement("UPDATE Singers SET LastName = 'test-only'"
                   " WHERE SingerId >= 1"));
  EXPECT_STATUS_OK(result);
}

void CheckReadWithOptions(
    Client client,
    std::function<Transaction::SingleUseOptions(CommitResult const&)> const&
        options_generator) {
  using RowValues = std::vector<Value>;
  std::vector<RowValues> expected_rows;
  auto commit = client.Commit(
      [&expected_rows](Transaction const&) -> StatusOr<Mutations> {
        expected_rows.clear();
        InsertMutationBuilder insert("Singers",
                                     {"SingerId", "FirstName", "LastName"});
        for (int i = 1; i != 10; ++i) {
          auto s = std::to_string(i);
          auto row = RowValues{Value(i), Value("test-fname-" + s),
                               Value("test-lname-" + s)};
          insert.AddRow(row);
          expected_rows.push_back(row);
        }
        return Mutations{std::move(insert).Build()};
      });
  ASSERT_STATUS_OK(commit);

  auto reader =
      client.Read(options_generator(*commit), "Singers", KeySet::All(),
                  {"SingerId", "FirstName", "LastName"});

  std::vector<RowValues> actual_rows;
  int row_number = 0;
  for (auto& row :
       reader.Rows<std::tuple<std::int64_t, std::string, std::string>>()) {
    SCOPED_TRACE("Reading row[" + std::to_string(row_number++) + "]");
    EXPECT_STATUS_OK(row);
    if (!row) break;
    std::vector<Value> v;
    v.emplace_back(std::get<0>(*row));
    v.emplace_back(std::get<1>(*row));
    v.emplace_back(std::get<2>(*row));
    actual_rows.push_back(std::move(v));
  }
  EXPECT_THAT(actual_rows, UnorderedElementsAreArray(expected_rows));
}

/// @test Test Read() with bounded staleness set by a timestamp.
TEST_F(ClientIntegrationTest, Read_BoundedStaleness_Timestamp) {
  CheckReadWithOptions(*client_, [](CommitResult const& result) {
    return Transaction::SingleUseOptions(
        /*min_read_timestamp=*/result.commit_timestamp);
  });
}

/// @test Test Read() with bounded staleness set by duration.
TEST_F(ClientIntegrationTest, Read_BoundedStaleness_Duration) {
  CheckReadWithOptions(*client_, [](CommitResult const&) {
    // We want a duration sufficiently recent to include the latest commit.
    return Transaction::SingleUseOptions(
        /*max_staleness=*/std::chrono::nanoseconds(1));
  });
}

/// @test Test Read() with exact staleness set to "all previous transactions".
TEST_F(ClientIntegrationTest, Read_ExactStaleness_Latest) {
  CheckReadWithOptions(*client_, [](CommitResult const&) {
    return Transaction::SingleUseOptions(Transaction::ReadOnlyOptions());
  });
}

/// @test Test Read() with exact staleness set by a timestamp.
TEST_F(ClientIntegrationTest, Read_ExactStaleness_Timestamp) {
  CheckReadWithOptions(*client_, [](CommitResult const& result) {
    return Transaction::SingleUseOptions(Transaction::ReadOnlyOptions(
        /*read_timestamp=*/result.commit_timestamp));
  });
}

/// @test Test Read() with exact staleness set by duration.
TEST_F(ClientIntegrationTest, Read_ExactStaleness_Duration) {
  CheckReadWithOptions(*client_, [](CommitResult const&) {
    return Transaction::SingleUseOptions(Transaction::ReadOnlyOptions(
        /*exact_staleness=*/Timestamp::duration(0)));
  });
}

void CheckExecuteQueryWithSingleUseOptions(
    Client client,
    std::function<Transaction::SingleUseOptions(CommitResult const&)> const&
        options_generator) {
  using RowValues = std::vector<Value>;
  std::vector<RowValues> expected_rows;
  auto commit = client.Commit(
      [&expected_rows](Transaction const&) -> StatusOr<Mutations> {
        InsertMutationBuilder insert("Singers",
                                     {"SingerId", "FirstName", "LastName"});
        for (int i = 1; i != 10; ++i) {
          auto s = std::to_string(i);
          auto row = RowValues{Value(i), Value("test-fname-" + s),
                               Value("test-lname-" + s)};
          insert.AddRow(row);
          expected_rows.push_back(row);
        }
        return Mutations{std::move(insert).Build()};
      });
  ASSERT_STATUS_OK(commit);

  auto reader = client.ExecuteQuery(
      options_generator(*commit),
      SqlStatement("SELECT SingerId, FirstName, LastName FROM Singers"));

  std::vector<RowValues> actual_rows;
  int row_number = 0;
  for (auto& row :
       reader.Rows<std::tuple<std::int64_t, std::string, std::string>>()) {
    SCOPED_TRACE("Reading row[" + std::to_string(row_number++) + "]");
    EXPECT_STATUS_OK(row);
    if (!row) break;
    std::vector<Value> v;
    v.emplace_back(std::get<0>(*row));
    v.emplace_back(std::get<1>(*row));
    v.emplace_back(std::get<2>(*row));
    actual_rows.push_back(std::move(v));
  }

  EXPECT_THAT(actual_rows, UnorderedElementsAreArray(expected_rows));
}

/// @test Test ExecuteQuery() with bounded staleness set by a timestamp.
TEST_F(ClientIntegrationTest, ExecuteQuery_BoundedStaleness_Timestamp) {
  CheckExecuteQueryWithSingleUseOptions(
      *client_, [](CommitResult const& result) {
        return Transaction::SingleUseOptions(
            /*min_read_timestamp=*/result.commit_timestamp);
      });
}

/// @test Test ExecuteQuery() with bounded staleness set by duration.
TEST_F(ClientIntegrationTest, ExecuteQuery_BoundedStaleness_Duration) {
  CheckExecuteQueryWithSingleUseOptions(*client_, [](CommitResult const&) {
    // We want a duration sufficiently recent to include the latest commit.
    return Transaction::SingleUseOptions(
        /*max_staleness=*/std::chrono::nanoseconds(1));
  });
}

/// @test Test ExecuteQuery() with exact staleness set to "all previous
/// transactions".
TEST_F(ClientIntegrationTest, ExecuteQuery_ExactStaleness_Latest) {
  CheckExecuteQueryWithSingleUseOptions(*client_, [](CommitResult const&) {
    return Transaction::SingleUseOptions(Transaction::ReadOnlyOptions());
  });
}

/// @test Test ExecuteQuery() with exact staleness set by a timestamp.
TEST_F(ClientIntegrationTest, ExecuteQuery_ExactStaleness_Timestamp) {
  CheckExecuteQueryWithSingleUseOptions(
      *client_, [](CommitResult const& result) {
        return Transaction::SingleUseOptions(Transaction::ReadOnlyOptions(
            /*read_timestamp=*/result.commit_timestamp));
      });
}

/// @test Test ExecuteQuery() with exact staleness set by duration.
TEST_F(ClientIntegrationTest, ExecuteQuery_ExactStaleness_Duration) {
  CheckExecuteQueryWithSingleUseOptions(*client_, [](CommitResult const&) {
    return Transaction::SingleUseOptions(Transaction::ReadOnlyOptions(
        /*exact_staleness=*/Timestamp::duration(0)));
  });
}

StatusOr<std::vector<std::vector<Value>>> AddSingerDataToTable(Client client) {
  std::vector<std::vector<Value>> expected_rows;
  auto commit = client.Commit(
      [&expected_rows](Transaction const&) -> StatusOr<Mutations> {
        expected_rows.clear();
        InsertMutationBuilder insert("Singers",
                                     {"SingerId", "FirstName", "LastName"});
        for (int i = 1; i != 10; ++i) {
          auto s = std::to_string(i);
          auto row = std::vector<Value>{Value(i), Value("test-fname-" + s),
                                        Value("test-lname-" + s)};
          insert.AddRow(row);
          expected_rows.push_back(row);
        }
        return Mutations{std::move(insert).Build()};
      });
  if (!commit.ok()) {
    return commit.status();
  }
  return expected_rows;
}

TEST_F(ClientIntegrationTest, PartitionRead) {
  auto expected_rows = AddSingerDataToTable(*client_);
  ASSERT_STATUS_OK(expected_rows);

  auto ro_transaction = MakeReadOnlyTransaction();
  auto read_partitions =
      client_->PartitionRead(ro_transaction, "Singers", KeySet::All(),
                             {"SingerId", "FirstName", "LastName"});
  ASSERT_STATUS_OK(read_partitions);

  std::vector<std::string> serialized_partitions;
  for (auto& partition : *read_partitions) {
    auto serialized_partition = SerializeReadPartition(partition);
    ASSERT_STATUS_OK(serialized_partition);
    serialized_partitions.push_back(*serialized_partition);
  }

  std::vector<std::vector<Value>> actual_rows;
  int partition_number = 0;
  for (auto& partition : serialized_partitions) {
    int row_number = 0;
    auto deserialized_partition = DeserializeReadPartition(partition);
    ASSERT_STATUS_OK(deserialized_partition);
    auto result_set = client_->Read(*deserialized_partition);
    for (auto& row :
         result_set
             .Rows<std::tuple<std::int64_t, std::string, std::string>>()) {
      SCOPED_TRACE("Reading partition[" + std::to_string(partition_number++) +
                   "] row[" + std::to_string(row_number++) + "]");
      EXPECT_STATUS_OK(row);
      if (!row) break;
      std::vector<Value> v;
      v.emplace_back(std::get<0>(*row));
      v.emplace_back(std::get<1>(*row));
      v.emplace_back(std::get<2>(*row));
      actual_rows.push_back(std::move(v));
    }
  }

  EXPECT_THAT(actual_rows, UnorderedElementsAreArray(*expected_rows));
}

TEST_F(ClientIntegrationTest, PartitionQuery) {
  auto expected_rows = AddSingerDataToTable(*client_);
  ASSERT_STATUS_OK(expected_rows);

  auto ro_transaction = MakeReadOnlyTransaction();
  auto query_partitions = client_->PartitionQuery(
      ro_transaction,
      SqlStatement("select SingerId, FirstName, LastName from Singers"));
  ASSERT_STATUS_OK(query_partitions);

  std::vector<std::string> serialized_partitions;
  for (auto& partition : *query_partitions) {
    auto serialized_partition = SerializeQueryPartition(partition);
    ASSERT_STATUS_OK(serialized_partition);
    serialized_partitions.push_back(*serialized_partition);
  }

  std::vector<std::vector<Value>> actual_rows;
  int partition_number = 0;
  for (auto& partition : serialized_partitions) {
    int row_number = 0;
    auto deserialized_partition = DeserializeQueryPartition(partition);
    ASSERT_STATUS_OK(deserialized_partition);
    auto result_set = client_->ExecuteQuery(*deserialized_partition);
    for (auto& row :
         result_set
             .Rows<std::tuple<std::int64_t, std::string, std::string>>()) {
      SCOPED_TRACE("Reading partition[" + std::to_string(partition_number++) +
                   "] row[" + std::to_string(row_number++) + "]");
      EXPECT_STATUS_OK(row);
      if (!row) break;
      std::vector<Value> v;
      v.emplace_back(std::get<0>(*row));
      v.emplace_back(std::get<1>(*row));
      v.emplace_back(std::get<2>(*row));
      actual_rows.push_back(std::move(v));
    }
  }

  EXPECT_THAT(actual_rows, UnorderedElementsAreArray(*expected_rows));
}

TEST_F(ClientIntegrationTest, ExecuteBatchDml) {
  auto statements = {
      SqlStatement("INSERT INTO Singers (SingerId, FirstName, LastName) "
                   "VALUES(1, 'Foo1', 'Bar1')"),
      SqlStatement("INSERT INTO Singers (SingerId, FirstName, LastName) "
                   "VALUES(2, 'Foo2', 'Bar2')"),
      SqlStatement("INSERT INTO Singers (SingerId, FirstName, LastName) "
                   "VALUES(3, 'Foo3', 'Bar3')"),
      SqlStatement("UPDATE Singers SET FirstName = \"FOO\" "
                   "WHERE FirstName = 'Foo1' or FirstName = 'Foo3'"),
  };

  auto& client = *client_;
  StatusOr<BatchDmlResult> batch_result;
  auto commit_result =
      client_->Commit([&client, &batch_result,
                       &statements](Transaction txn) -> StatusOr<Mutations> {
        batch_result = client.ExecuteBatchDml(std::move(txn), statements);
        if (!batch_result) return batch_result.status();
        if (!batch_result->status.ok()) return batch_result->status;
        return Mutations{};
      });

  ASSERT_STATUS_OK(commit_result);
  ASSERT_STATUS_OK(batch_result);
  ASSERT_STATUS_OK(batch_result->status);
  ASSERT_EQ(batch_result->stats.size(), 4);
  ASSERT_EQ(batch_result->stats[0].row_count, 1);
  ASSERT_EQ(batch_result->stats[1].row_count, 1);
  ASSERT_EQ(batch_result->stats[2].row_count, 1);
  ASSERT_EQ(batch_result->stats[3].row_count, 2);

  auto query = client_->ExecuteQuery(SqlStatement(
      "SELECT SingerId, FirstName, LastName FROM Singers ORDER BY SingerId"));

  struct Expectation {
    std::int64_t id;
    std::string fname;
    std::string lname;
  };
  auto expected = std::vector<Expectation>{
      Expectation{1, "FOO", "Bar1"},
      Expectation{2, "Foo2", "Bar2"},
      Expectation{3, "FOO", "Bar3"},
  };
  std::size_t counter = 0;
  for (auto const& row :
       query.Rows<std::tuple<std::int64_t, std::string, std::string>>()) {
    ASSERT_STATUS_OK(row);
    ASSERT_EQ(std::get<0>(*row), expected[counter].id);
    ASSERT_EQ(std::get<1>(*row), expected[counter].fname);
    ASSERT_EQ(std::get<2>(*row), expected[counter].lname);
    ++counter;
  }
  ASSERT_EQ(counter, expected.size());
}

TEST_F(ClientIntegrationTest, ExecuteBatchDmlMany) {
  std::vector<SqlStatement> v;
  constexpr auto kBatchSize = 200;
  for (int i = 0; i < kBatchSize; ++i) {
    std::string const singer_id = std::to_string(i);
    std::string const first_name = "Foo" + singer_id;
    std::string const last_name = "Bar" + singer_id;
    std::string insert =
        "INSERT INTO Singers (SingerId, FirstName, LastName) Values(";
    insert += singer_id + ", '";
    insert += first_name + "', '";
    insert += last_name + "')";
    v.emplace_back(insert);
  }

  std::vector<SqlStatement> left(v.begin(), v.begin() + v.size() / 2);
  std::vector<SqlStatement> right(v.begin() + v.size() / 2, v.end());

  auto& client = *client_;
  StatusOr<BatchDmlResult> batch_result_left;
  StatusOr<BatchDmlResult> batch_result_right;
  auto commit_result =
      client_->Commit([&client, &batch_result_left, &batch_result_right, &left,
                       &right](Transaction txn) -> StatusOr<Mutations> {
        batch_result_left = client.ExecuteBatchDml(txn, left);
        if (!batch_result_left) return batch_result_left.status();
        if (!batch_result_left->status.ok()) return batch_result_left->status;

        batch_result_right = client.ExecuteBatchDml(std::move(txn), right);
        if (!batch_result_right) return batch_result_right.status();
        if (!batch_result_right->status.ok()) return batch_result_right->status;

        return Mutations{};
      });

  ASSERT_STATUS_OK(commit_result);

  ASSERT_STATUS_OK(batch_result_left);
  EXPECT_EQ(batch_result_left->stats.size(), left.size());
  EXPECT_STATUS_OK(batch_result_left->status);
  for (auto const& stats : batch_result_left->stats) {
    ASSERT_EQ(stats.row_count, 1);
  }

  ASSERT_STATUS_OK(batch_result_right);
  EXPECT_EQ(batch_result_right->stats.size(), right.size());
  EXPECT_STATUS_OK(batch_result_right->status);
  for (auto const& stats : batch_result_right->stats) {
    ASSERT_EQ(stats.row_count, 1);
  }

  auto query = client_->ExecuteQuery(SqlStatement(
      "SELECT SingerId, FirstName, LastName FROM Singers ORDER BY SingerId"));

  auto counter = 0;
  for (auto const& row :
       query.Rows<std::tuple<std::int64_t, std::string, std::string>>()) {
    ASSERT_STATUS_OK(row);
    std::string const singer_id = std::to_string(counter);
    std::string const first_name = "Foo" + singer_id;
    std::string const last_name = "Bar" + singer_id;
    ASSERT_EQ(std::get<0>(*row), counter);
    ASSERT_EQ(std::get<1>(*row), first_name);
    ASSERT_EQ(std::get<2>(*row), last_name);
    ++counter;
  }

  ASSERT_EQ(counter, kBatchSize);
}

TEST_F(ClientIntegrationTest, ExecuteBatchDmlFailure) {
  auto statements = {
      SqlStatement("INSERT INTO Singers (SingerId, FirstName, LastName) "
                   "VALUES(1, 'Foo1', 'Bar1')"),
      SqlStatement("INSERT INTO Singers (SingerId, FirstName, LastName) "
                   "VALUES(2, 'Foo2', 'Bar2')"),
      SqlStatement("INSERT OOPS SYNTAX ERROR"),
      SqlStatement("UPDATE Singers SET FirstName = 'FOO' "
                   "WHERE FirstName = 'Foo1' or FirstName = 'Foo3'"),
  };

  auto& client = *client_;
  StatusOr<BatchDmlResult> batch_result;
  auto commit_result =
      client_->Commit([&client, &batch_result,
                       &statements](Transaction txn) -> StatusOr<Mutations> {
        batch_result = client.ExecuteBatchDml(std::move(txn), statements);
        if (!batch_result) return batch_result.status();
        if (!batch_result->status.ok()) return batch_result->status;
        return Mutations{};
      });

  ASSERT_FALSE(commit_result.ok());
  ASSERT_STATUS_OK(batch_result);
  ASSERT_FALSE(batch_result->status.ok());
  ASSERT_EQ(batch_result->stats.size(), 2);
  ASSERT_EQ(batch_result->stats[0].row_count, 1);
  ASSERT_EQ(batch_result->stats[1].row_count, 1);
}

}  // namespace
}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google

int main(int argc, char* argv[]) {
  ::google::cloud::testing_util::InitGoogleMock(argc, argv);
  (void)::testing::AddGlobalTestEnvironment(
      new google::cloud::spanner_testing::DatabaseEnvironment());

  return RUN_ALL_TESTS();
}
