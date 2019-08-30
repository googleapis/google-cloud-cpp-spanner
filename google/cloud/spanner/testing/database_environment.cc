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

#include "google/cloud/spanner/testing/database_environment.h"
#include "google/cloud/spanner/database_admin_client.h"
#include "google/cloud/spanner/testing/random_database_name.h"
#include "google/cloud/internal/getenv.h"
#include "google/cloud/testing_util/assert_ok.h"

namespace google {
namespace cloud {
namespace spanner_testing {
inline namespace SPANNER_CLIENT_NS {

google::cloud::spanner::Database* DatabaseEnvironment::db_;
google::cloud::internal::DefaultPRNG* DatabaseEnvironment::generator_;

void DatabaseEnvironment::SetUp() {
  auto project_id =
      google::cloud::internal::GetEnv("GOOGLE_CLOUD_PROJECT").value_or("");
  ASSERT_FALSE(project_id.empty());
  auto instance_id =
      google::cloud::internal::GetEnv("GOOGLE_CLOUD_CPP_SPANNER_INSTANCE")
          .value_or("");
  ASSERT_FALSE(instance_id.empty());

  generator_ = new google::cloud::internal::DefaultPRNG(
      google::cloud::internal::MakeDefaultPRNG());
  auto database_id = spanner_testing::RandomDatabaseName(*generator_);

  db_ = new spanner::Database(project_id, instance_id, database_id);

  std::cout << "Creating database and table " << std::flush;
  spanner::DatabaseAdminClient admin_client;
  auto database_future =
      admin_client.CreateDatabase(*db_, {R"""(CREATE TABLE Singers (
                                SingerId   INT64 NOT NULL,
                                FirstName  STRING(1024),
                                LastName   STRING(1024)
                             ) PRIMARY KEY (SingerId))"""});
  int i = 0;
  int const timeout = 120;
  while (++i < timeout) {
    auto status = database_future.wait_for(std::chrono::seconds(1));
    if (status == std::future_status::ready) break;
    std::cout << '.' << std::flush;
  }
  if (i >= timeout) {
    std::cout << "TIMEOUT\n";
    FAIL();
  }
  auto database = database_future.get();
  ASSERT_STATUS_OK(database);
  std::cout << "DONE\n";
}

void DatabaseEnvironment::TearDown() {
  spanner::DatabaseAdminClient admin_client;
  auto drop_status = admin_client.DropDatabase(*db_);
  EXPECT_STATUS_OK(drop_status);
}

}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner_testing
}  // namespace cloud
}  // namespace google