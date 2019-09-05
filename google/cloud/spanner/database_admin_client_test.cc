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

#include "google/cloud/spanner/database_admin_client.h"
#include "google/cloud/spanner/internal/database_admin_retry.h"
#include "google/cloud/spanner/testing/mock_database_admin_stub.h"
#include "google/cloud/testing_util/assert_ok.h"
#include <gmock/gmock.h>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {
namespace {

using ::google::cloud::spanner_testing::MockDatabaseAdminStub;
using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;
namespace gcsa = ::google::spanner::admin::database::v1;

DatabaseAdminClient CreateTestingClient(
    std::shared_ptr<internal::DatabaseAdminStub> mock) {
  auto stub = std::make_shared<internal::DatabaseAdminRetry>(
      std::move(mock), LimitedErrorCountRetryPolicy(/*maximum_failures=*/2),
      ExponentialBackoffPolicy(std::chrono::microseconds(1),
                               std::chrono::microseconds(1), 2.0));
  return DatabaseAdminClient(std::move(stub));
}

/// @test Verify that successful case works.
TEST(DatabaseAdminClientTest, CreateDatabaseSuccess) {
  auto mock = std::make_shared<MockDatabaseAdminStub>();

  EXPECT_CALL(*mock, CreateDatabase(_, _))
      .WillOnce(
          Invoke([](grpc::ClientContext&, gcsa::CreateDatabaseRequest const&) {
            gcsa::Database database;
            database.set_name("test-db");
            google::longrunning::Operation op;
            op.set_name("test-operation-name");
            op.set_done(true);
            op.mutable_response()->PackFrom(database);
            return make_status_or(op);
          }));
  EXPECT_CALL(*mock, AwaitCreateDatabase(_))
      .WillOnce(Invoke([](google::longrunning::Operation const& op) {
        EXPECT_EQ("test-operation-name", op.name());
        EXPECT_TRUE(op.done());
        EXPECT_TRUE(op.has_response());
        gcsa::Database database;
        op.response().UnpackTo(&database);
        return make_ready_future(make_status_or(database));
      }));

  DatabaseAdminClient client(mock);
  Database dbase("test-project", "test-instance", "test-db");
  auto fut = client.CreateDatabase(dbase);
  EXPECT_EQ(std::future_status::ready, fut.wait_for(std::chrono::seconds(0)));
  auto db = fut.get();
  EXPECT_STATUS_OK(db);

  EXPECT_EQ("test-db", db->name());
}

/// @test Verify that a permanent error in CreateDatabase is immediately
/// reported.
TEST(DatabaseAdminClientTest, HandleCreateDatabaseError) {
  auto mock = std::make_shared<MockDatabaseAdminStub>();

  EXPECT_CALL(*mock, CreateDatabase(_, _))
      .WillOnce(
          Invoke([](grpc::ClientContext&, gcsa::CreateDatabaseRequest const&) {
            return StatusOr<google::longrunning::Operation>(
                Status(StatusCode::kPermissionDenied, "uh-oh"));
          }));

  DatabaseAdminClient client(mock);
  Database dbase("test-project", "test-instance", "test-db");
  auto fut = client.CreateDatabase(dbase);
  EXPECT_EQ(std::future_status::ready, fut.wait_for(std::chrono::seconds(0)));
  auto db = fut.get();
  EXPECT_EQ(StatusCode::kPermissionDenied, db.status().code());
}

/// @test Verify that the successful case works.
TEST(DatabaseAdminClientTest, GetDatabase_Success) {
  auto mock = std::make_shared<MockDatabaseAdminStub>();
  std::string const expected_name =
      "projects/test-project/instances/test-instance/databases/test-database";

  EXPECT_CALL(*mock, GetDatabase(_, _))
      .WillOnce(Return(Status(StatusCode::kUnavailable, "try-again")))
      .WillOnce(
          Invoke([&expected_name](grpc::ClientContext&,
                                  gcsa::GetDatabaseRequest const& request) {
            EXPECT_EQ(expected_name, request.name());
            gcsa::Database response;
            response.set_name(request.name());
            response.set_state(gcsa::Database::READY);
            return response;
          }));

  auto client = CreateTestingClient(std::move(mock));
  auto response = client.GetDatabase(
      Database("test-project", "test-instance", "test-database"));
  EXPECT_STATUS_OK(response);
  EXPECT_EQ(gcsa::Database::READY, response->state());
  EXPECT_EQ(expected_name, response->name());
}

/// @test Verify that permanent errors are reported immediately.
TEST(DatabaseAdminClientTest, GetDatabase_PermanentError) {
  auto mock = std::make_shared<MockDatabaseAdminStub>();

  EXPECT_CALL(*mock, GetDatabase(_, _))
      .WillOnce(Return(Status(StatusCode::kPermissionDenied, "uh-oh")));

  auto client = CreateTestingClient(std::move(mock));
  auto response = client.GetDatabase(
      Database("test-project", "test-instance", "test-database"));
  EXPECT_EQ(StatusCode::kPermissionDenied, response.status().code());
}

/// @test Verify that too many transients errors are reported corrrectly.
TEST(DatabaseAdminClientTest, GetDatabase_TooManyTransients) {
  auto mock = std::make_shared<MockDatabaseAdminStub>();

  EXPECT_CALL(*mock, GetDatabase(_, _))
      .WillRepeatedly(Return(Status(StatusCode::kUnavailable, "try-again")));

  auto client = CreateTestingClient(std::move(mock));
  auto response = client.GetDatabase(
      Database("test-project", "test-instance", "test-database"));
  EXPECT_EQ(StatusCode::kUnavailable, response.status().code());
}

/// @test Verify that successful case works.
TEST(DatabaseAdminClientTest, UpdateDatabaseSuccess) {
  auto mock = std::make_shared<MockDatabaseAdminStub>();

  EXPECT_CALL(*mock, UpdateDatabase(_, _))
      .WillOnce(Invoke(
          [](grpc::ClientContext&, gcsa::UpdateDatabaseDdlRequest const&) {
            gcsa::UpdateDatabaseDdlMetadata metadata;
            metadata.set_database("test-db");
            google::longrunning::Operation op;
            op.set_name("test-operation-name");
            op.set_done(true);
            op.mutable_response()->PackFrom(metadata);
            return make_status_or(op);
          }));
  EXPECT_CALL(*mock, AwaitUpdateDatabase(_))
      .WillOnce(Invoke([](google::longrunning::Operation const& op) {
        EXPECT_EQ("test-operation-name", op.name());
        EXPECT_TRUE(op.done());
        EXPECT_TRUE(op.has_response());
        gcsa::UpdateDatabaseDdlMetadata metadata;
        op.response().UnpackTo(&metadata);
        return make_ready_future(make_status_or(metadata));
      }));

  DatabaseAdminClient client(mock);
  Database dbase("test-project", "test-instance", "test-db");
  auto fut = client.UpdateDatabase(
      dbase, {"ALTER TABLE Albums ADD COLUMN MarketingBudget INT64"});
  EXPECT_EQ(std::future_status::ready, fut.wait_for(std::chrono::seconds(0)));
  auto metadata = fut.get();
  EXPECT_STATUS_OK(metadata);

  EXPECT_EQ("test-db", metadata->database());
}

/// @test Verify that a permanent error in UpdateDatabase is immediately
/// reported.
TEST(DatabaseAdminClientTest, HandleUpdateDatabaseError) {
  auto mock = std::make_shared<MockDatabaseAdminStub>();

  EXPECT_CALL(*mock, UpdateDatabase(_, _))
      .WillOnce(Invoke(
          [](grpc::ClientContext&, gcsa::UpdateDatabaseDdlRequest const&) {
            return StatusOr<google::longrunning::Operation>(
                Status(StatusCode::kPermissionDenied, "uh-oh"));
          }));

  DatabaseAdminClient client(mock);
  Database dbase("test-project", "test-instance", "test-db");
  auto fut = client.UpdateDatabase(
      dbase, {"ALTER TABLE Albums ADD COLUMN MarketingBudget INT64"});
  EXPECT_EQ(std::future_status::ready, fut.wait_for(std::chrono::seconds(0)));
  auto db = fut.get();
  EXPECT_EQ(StatusCode::kPermissionDenied, db.status().code());
}

/// @test Verify that errors in the polling loop are reported.
TEST(DatabaseAdminClientTest, HandleAwaitCreateDatabaseError) {
  auto mock = std::make_shared<MockDatabaseAdminStub>();

  EXPECT_CALL(*mock, CreateDatabase(_, _))
      .WillOnce(
          Invoke([](grpc::ClientContext&, gcsa::CreateDatabaseRequest const&) {
            google::longrunning::Operation op;
            op.set_name("test-operation-name");
            op.set_done(false);
            return make_status_or(std::move(op));
          }));
  EXPECT_CALL(*mock, AwaitCreateDatabase(_))
      .WillOnce(Invoke([](google::longrunning::Operation const& op) {
        EXPECT_EQ("test-operation-name", op.name());
        return make_ready_future(
            StatusOr<gcsa::Database>(Status(StatusCode::kAborted, "oh noes")));
      }));

  DatabaseAdminClient client(mock);
  Database dbase("test-project", "test-instance", "test-db");
  auto db = client.CreateDatabase(dbase).get();
  EXPECT_EQ(StatusCode::kAborted, db.status().code());
}

/// @test Verify that errors in the polling loop are reported.
TEST(DatabaseAdminClientTest, HandleAwaitUpdateDatabaseError) {
  auto mock = std::make_shared<MockDatabaseAdminStub>();

  EXPECT_CALL(*mock, UpdateDatabase(_, _))
      .WillOnce(Invoke(
          [](grpc::ClientContext&, gcsa::UpdateDatabaseDdlRequest const&) {
            google::longrunning::Operation op;
            op.set_name("test-operation-name");
            op.set_done(false);
            return make_status_or(std::move(op));
          }));
  EXPECT_CALL(*mock, AwaitUpdateDatabase(_))
      .WillOnce(Invoke([](google::longrunning::Operation const& op) {
        EXPECT_EQ("test-operation-name", op.name());
        return make_ready_future(StatusOr<gcsa::UpdateDatabaseDdlMetadata>(
            Status(StatusCode::kAborted, "oh noes")));
      }));

  DatabaseAdminClient client(mock);
  Database dbase("test-project", "test-instance", "test-db");
  auto db =
      client
          .UpdateDatabase(
              dbase, {"ALTER TABLE Albums ADD COLUMN MarketingBudget INT64"})
          .get();
  EXPECT_EQ(StatusCode::kAborted, db.status().code());
}

}  // namespace
}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google
