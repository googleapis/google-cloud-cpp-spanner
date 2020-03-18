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

#include "google/cloud/spanner/internal/database_admin_logging.h"
#include "google/cloud/spanner/testing/mock_database_admin_stub.h"
#include "google/cloud/spanner/tracing_options.h"
#include "google/cloud/log.h"
#include "google/cloud/testing_util/assert_ok.h"
#include "google/cloud/testing_util/capture_log_lines_backend.h"
#include <gmock/gmock.h>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {
namespace internal {
namespace {

using ::testing::_;
using ::testing::Return;
namespace gcsa = ::google::spanner::admin::database::v1;

class DatabaseAdminLoggingTest : public ::testing::Test {
 protected:
  void SetUp() override {
    backend_ =
        std::make_shared<google::cloud::testing_util::CaptureLogLinesBackend>();
    logger_id_ = google::cloud::LogSink::Instance().AddBackend(backend_);
    mock_ = std::make_shared<spanner_testing::MockDatabaseAdminStub>();
  }

  void TearDown() override {
    google::cloud::LogSink::Instance().RemoveBackend(logger_id_);
    logger_id_ = 0;
  }

  static Status TransientError() {
    return Status(StatusCode::kUnavailable, "try-again");
  }

  void HasLogLineWith(std::string const& contents) {
    auto count =
        std::count_if(backend_->log_lines.begin(), backend_->log_lines.end(),
                      [&contents](std::string const& line) {
                        return std::string::npos != line.find(contents);
                      });
    EXPECT_NE(0, count) << "expected to find line with " << contents;
  }

  std::shared_ptr<spanner_testing::MockDatabaseAdminStub> mock_;

 private:
  std::shared_ptr<google::cloud::testing_util::CaptureLogLinesBackend> backend_;
  long logger_id_ = 0;  // NOLINT
};

TEST_F(DatabaseAdminLoggingTest, CreateDatabase) {
  EXPECT_CALL(*mock_, CreateDatabase(_, _)).WillOnce(Return(TransientError()));

  DatabaseAdminLogging stub(mock_, TracingOptions{});

  grpc::ClientContext context;
  auto status = stub.CreateDatabase(context, gcsa::CreateDatabaseRequest{});
  EXPECT_EQ(TransientError(), status.status());

  HasLogLineWith("CreateDatabase");
  HasLogLineWith(TransientError().message());
}

TEST_F(DatabaseAdminLoggingTest, GetDatabase) {
  EXPECT_CALL(*mock_, GetDatabase(_, _)).WillOnce(Return(TransientError()));

  DatabaseAdminLogging stub(mock_, TracingOptions{});

  grpc::ClientContext context;
  auto response = stub.GetDatabase(context, gcsa::GetDatabaseRequest{});
  EXPECT_EQ(TransientError(), response.status());

  HasLogLineWith("GetDatabase");
  HasLogLineWith(TransientError().message());
}

TEST_F(DatabaseAdminLoggingTest, GetDatabaseDdl) {
  EXPECT_CALL(*mock_, GetDatabaseDdl(_, _)).WillOnce(Return(TransientError()));

  DatabaseAdminLogging stub(mock_, TracingOptions{});

  grpc::ClientContext context;
  auto response = stub.GetDatabaseDdl(context, gcsa::GetDatabaseDdlRequest{});
  EXPECT_EQ(TransientError(), response.status());

  HasLogLineWith("GetDatabaseDdl");
  HasLogLineWith(TransientError().message());
}

TEST_F(DatabaseAdminLoggingTest, UpdateDatabase) {
  EXPECT_CALL(*mock_, UpdateDatabase(_, _)).WillOnce(Return(TransientError()));

  DatabaseAdminLogging stub(mock_, TracingOptions{});

  grpc::ClientContext context;
  auto status = stub.UpdateDatabase(context, gcsa::UpdateDatabaseDdlRequest{});
  EXPECT_EQ(TransientError(), status.status());

  HasLogLineWith("UpdateDatabase");
  HasLogLineWith(TransientError().message());
}

TEST_F(DatabaseAdminLoggingTest, DropDatabase) {
  EXPECT_CALL(*mock_, DropDatabase(_, _)).WillOnce(Return(TransientError()));

  DatabaseAdminLogging stub(mock_, TracingOptions{});

  grpc::ClientContext context;
  auto status = stub.DropDatabase(context, gcsa::DropDatabaseRequest{});
  EXPECT_EQ(TransientError(), status);

  HasLogLineWith("DropDatabase");
  HasLogLineWith(TransientError().message());
}

TEST_F(DatabaseAdminLoggingTest, ListDatabases) {
  EXPECT_CALL(*mock_, ListDatabases(_, _)).WillOnce(Return(TransientError()));

  DatabaseAdminLogging stub(mock_, TracingOptions{});

  grpc::ClientContext context;
  auto response = stub.ListDatabases(context, gcsa::ListDatabasesRequest{});
  EXPECT_EQ(TransientError(), response.status());

  HasLogLineWith("ListDatabases");
  HasLogLineWith(TransientError().message());
}

TEST_F(DatabaseAdminLoggingTest, RestoreDatabase) {
  EXPECT_CALL(*mock_, RestoreDatabase(_, _)).WillOnce(Return(TransientError()));

  DatabaseAdminLogging stub(mock_, TracingOptions{});

  grpc::ClientContext context;
  auto status = stub.RestoreDatabase(context, gcsa::RestoreDatabaseRequest{});
  EXPECT_EQ(TransientError(), status.status());

  HasLogLineWith("RestoreDatabase");
  HasLogLineWith(TransientError().message());
}

TEST_F(DatabaseAdminLoggingTest, GetIamPolicy) {
  EXPECT_CALL(*mock_, GetIamPolicy(_, _)).WillOnce(Return(TransientError()));

  DatabaseAdminLogging stub(mock_, TracingOptions{});

  grpc::ClientContext context;
  auto response =
      stub.GetIamPolicy(context, google::iam::v1::GetIamPolicyRequest{});
  EXPECT_EQ(TransientError(), response.status());

  HasLogLineWith("GetIamPolicy");
  HasLogLineWith(TransientError().message());
}

TEST_F(DatabaseAdminLoggingTest, SetIamPolicy) {
  EXPECT_CALL(*mock_, SetIamPolicy(_, _)).WillOnce(Return(TransientError()));

  DatabaseAdminLogging stub(mock_, TracingOptions{});

  grpc::ClientContext context;
  auto response =
      stub.SetIamPolicy(context, google::iam::v1::SetIamPolicyRequest{});
  EXPECT_EQ(TransientError(), response.status());

  HasLogLineWith("SetIamPolicy");
  HasLogLineWith(TransientError().message());
}

TEST_F(DatabaseAdminLoggingTest, TestIamPermissions) {
  EXPECT_CALL(*mock_, TestIamPermissions(_, _))
      .WillOnce(Return(TransientError()));

  DatabaseAdminLogging stub(mock_, TracingOptions{});

  grpc::ClientContext context;
  auto response = stub.TestIamPermissions(
      context, google::iam::v1::TestIamPermissionsRequest{});
  EXPECT_EQ(TransientError(), response.status());

  HasLogLineWith("TestIamPermissions");
  HasLogLineWith(TransientError().message());
}

TEST_F(DatabaseAdminLoggingTest, CreateBackup) {
  EXPECT_CALL(*mock_, CreateBackup(_, _)).WillOnce(Return(TransientError()));

  DatabaseAdminLogging stub(mock_, TracingOptions{});

  grpc::ClientContext context;
  auto status = stub.CreateBackup(context, gcsa::CreateBackupRequest{});
  EXPECT_EQ(TransientError(), status.status());

  HasLogLineWith("CreateBackup");
  HasLogLineWith(TransientError().message());
}

TEST_F(DatabaseAdminLoggingTest, GetBackup) {
  EXPECT_CALL(*mock_, GetBackup(_, _)).WillOnce(Return(TransientError()));

  DatabaseAdminLogging stub(mock_, TracingOptions{});

  grpc::ClientContext context;
  auto status = stub.GetBackup(context, gcsa::GetBackupRequest{});
  EXPECT_EQ(TransientError(), status.status());

  HasLogLineWith("GetBackup");
  HasLogLineWith(TransientError().message());
}

TEST_F(DatabaseAdminLoggingTest, DeleteBackup) {
  EXPECT_CALL(*mock_, DeleteBackup(_, _)).WillOnce(Return(TransientError()));

  DatabaseAdminLogging stub(mock_, TracingOptions{});

  grpc::ClientContext context;
  auto status = stub.DeleteBackup(context, gcsa::DeleteBackupRequest{});
  EXPECT_EQ(TransientError(), status);

  HasLogLineWith("DeleteBackup");
  HasLogLineWith(TransientError().message());
}

TEST_F(DatabaseAdminLoggingTest, ListBackups) {
  EXPECT_CALL(*mock_, ListBackups(_, _)).WillOnce(Return(TransientError()));

  DatabaseAdminLogging stub(mock_, TracingOptions{});

  grpc::ClientContext context;
  auto response = stub.ListBackups(context, gcsa::ListBackupsRequest{});
  EXPECT_EQ(TransientError(), response.status());

  HasLogLineWith("ListBackups");
  HasLogLineWith(TransientError().message());
}

TEST_F(DatabaseAdminLoggingTest, UpdateBackup) {
  EXPECT_CALL(*mock_, UpdateBackup(_, _)).WillOnce(Return(TransientError()));

  DatabaseAdminLogging stub(mock_, TracingOptions{});

  grpc::ClientContext context;
  auto status = stub.UpdateBackup(context, gcsa::UpdateBackupRequest{});
  EXPECT_EQ(TransientError(), status.status());

  HasLogLineWith("UpdateBackup");
  HasLogLineWith(TransientError().message());
}

TEST_F(DatabaseAdminLoggingTest, ListBackupOperations) {
  EXPECT_CALL(*mock_, ListBackupOperations(_, _))
      .WillOnce(Return(TransientError()));

  DatabaseAdminLogging stub(mock_, TracingOptions{});

  grpc::ClientContext context;
  auto response =
      stub.ListBackupOperations(context, gcsa::ListBackupOperationsRequest{});
  EXPECT_EQ(TransientError(), response.status());

  HasLogLineWith("ListBackupOperations");
  HasLogLineWith(TransientError().message());
}

TEST_F(DatabaseAdminLoggingTest, ListDatabaseOperations) {
  EXPECT_CALL(*mock_, ListDatabaseOperations(_, _))
      .WillOnce(Return(TransientError()));

  DatabaseAdminLogging stub(mock_, TracingOptions{});

  grpc::ClientContext context;
  auto response = stub.ListDatabaseOperations(
      context, gcsa::ListDatabaseOperationsRequest{});
  EXPECT_EQ(TransientError(), response.status());

  HasLogLineWith("ListDatabaseOperations");
  HasLogLineWith(TransientError().message());
}

TEST_F(DatabaseAdminLoggingTest, GetOperation) {
  EXPECT_CALL(*mock_, GetOperation(_, _)).WillOnce(Return(TransientError()));

  DatabaseAdminLogging stub(mock_, TracingOptions{});

  grpc::ClientContext context;
  auto status =
      stub.GetOperation(context, google::longrunning::GetOperationRequest{});
  EXPECT_EQ(TransientError(), status.status());

  HasLogLineWith("GetOperation");
  HasLogLineWith(TransientError().message());
}

TEST_F(DatabaseAdminLoggingTest, CancelOperation) {
  EXPECT_CALL(*mock_, CancelOperation(_, _)).WillOnce(Return(TransientError()));

  DatabaseAdminLogging stub(mock_, TracingOptions{});

  grpc::ClientContext context;
  auto status = stub.CancelOperation(
      context, google::longrunning::CancelOperationRequest{});
  EXPECT_EQ(TransientError(), status);

  HasLogLineWith("CancelOperation");
  HasLogLineWith(TransientError().message());
}

}  // namespace
}  // namespace internal
}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google
