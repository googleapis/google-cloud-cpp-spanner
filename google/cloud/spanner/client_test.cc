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
#include "google/cloud/spanner/internal/spanner_stub.h"
#include <gmock/gmock.h>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {
namespace {

using ::testing::_;
using ::testing::HasSubstr;
namespace spanner_proto = ::google::spanner::v1;

// NOLINTNEXTLINE(clang-analyzer-optin.cplusplus.VirtualCall)
class MockConnection : public Connection {
 public:
  virtual ~MockConnection() override = default;

  // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
  MOCK_METHOD1(Commit, StatusOr<CommitResult>(CommitParams));
};

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

TEST(ClientTest, Commit) {
  auto conn = std::make_shared<MockConnection>();

  Client client(conn);
  EXPECT_CALL(*conn, Commit(_))
      .WillOnce(
          ::testing::Return(Status(StatusCode::kPermissionDenied, "blah")));

  auto txn = MakeReadWriteTransaction();
  auto commit = client.Commit(txn, {});
  EXPECT_EQ(StatusCode::kPermissionDenied, commit.status().code());
  EXPECT_THAT(commit.status().message(), HasSubstr("blah"));
}

TEST(ClientTest, MakeDatabaseName) {
  EXPECT_EQ(
      "projects/dummy_project/instances/dummy_instance/databases/"
      "dummy_database_id",
      MakeDatabaseName("dummy_project", "dummy_instance", "dummy_database_id"));
}

TEST(ClientTest, MakeConnectionOptionalArguments) {
  auto conn = MakeConnection("foo");
  EXPECT_NE(conn, nullptr);

  conn = MakeConnection("foo", grpc::GoogleDefaultCredentials());
  EXPECT_NE(conn, nullptr);

  conn = MakeConnection("foo", grpc::GoogleDefaultCredentials(), "localhost");
  EXPECT_NE(conn, nullptr);
}

}  // namespace
}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google
