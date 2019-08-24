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
#include "google/cloud/spanner/testing/random_database_name.h"
#include "google/cloud/internal/getenv.h"
#include "google/cloud/internal/make_unique.h"
#include "google/cloud/internal/random.h"
#include "google/cloud/testing_util/assert_ok.h"
#include <gmock/gmock.h>
#include <cmath>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {
namespace {

class RpcFailureThresholdTest : public ::testing::Test {
 public:
  void SetUp() override {
    auto project_id =
        google::cloud::internal::GetEnv("GOOGLE_CLOUD_PROJECT").value_or("");
    ASSERT_FALSE(project_id.empty());
    auto instance_id =
        google::cloud::internal::GetEnv("GOOGLE_CLOUD_CPP_SPANNER_INSTANCE")
            .value_or("");
    ASSERT_FALSE(instance_id.empty());

    generator_ = google::cloud::internal::MakeDefaultPRNG();
    auto database_id = spanner_testing::RandomDatabaseName(generator_);

    db_ = google::cloud::internal::make_unique<Database>(
        project_id, instance_id, database_id);

    std::cout << "Creating database [" << database_id << "] and table "
              << std::flush;
    DatabaseAdminClient admin_client;
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
      std::cout << " TIMEOUT\n";
      FAIL();
    }
    auto database = database_future.get();
    ASSERT_STATUS_OK(database);
    std::cout << " DONE\n";

    client_ =
        google::cloud::internal::make_unique<Client>(MakeConnection(*db_));
  }

  void TearDown() override {
    if (!client_) {
      return;
    }
    std::cout << "Dropping database " << db_->DatabaseId() << std::flush;
    DatabaseAdminClient admin_client;
    auto drop_status = admin_client.DropDatabase(*db_);
    EXPECT_STATUS_OK(drop_status);
    std::cout << " DONE\n";
  }

 protected:
  google::cloud::internal::DefaultPRNG generator_;
  std::unique_ptr<Database> db_;
  std::unique_ptr<Client> client_;
};

/**
 * @test Verify that the error rate for commit operations is within bounds.
 *
 * This program runs a series of commits
 */
TEST_F(RpcFailureThresholdTest, CommitErrors) {
  ASSERT_TRUE(client_);

  int number_of_failures = 0;
  int number_of_successes = 0;
  // probit(0.975): probit is the percentile function for the normal
  // distribution:
  //   https://en.wikipedia.org/wiki/Probit
  // We will need it to estimate the confidence interval for the error rate.
  double const z = 1.96;

  // We are willing to tolerate one failure in 10,000 requests.
  double const kThreshold = 1 / 10000.0;

  auto update_trials = [&number_of_failures,
                        &number_of_successes](Status const& status) {
    if (status.ok()) {
      ++number_of_successes;
    } else {
      ++number_of_failures;
      std::cout << status << "\n";
    }
  };

  int const iterations = 200;
  int const report = iterations / 40;
  std::cout << "Running test " << std::flush;
  for (int i = 0; i != iterations; ++i) {
    if (i % report == 0) std::cout << '.' << std::flush;
    auto s = std::to_string(i);
    auto commit_result = client_->Commit(
        MakeReadWriteTransaction(),
        {InsertMutationBuilder("Singers", {"SingerId", "FirstName", "LastName"})
             .EmplaceRow(i, "fname-" + s, "lname-" + s)
             .Build()});
    update_trials(commit_result.status());
    // Read the first 5 rows.
    ReadOptions max_rows;
    max_rows.limit = 5;
    auto reader =
        client_->Read("Singers", KeySet::All(),
                      {"SingerId", "FirstName", "LastName"}, max_rows);
    update_trials(reader.status());
    if (!reader) continue;
    for (auto row : reader->Rows<std::int64_t, std::string, std::string>()) {
      update_trials(row.status());
      if (!row) break;
    }
    auto delete_status = RunTransaction(
        *client_, Transaction::ReadWriteOptions{},
        [&](Client client, Transaction const& txn) -> StatusOr<Mutations> {
          auto status = client.ExecuteSql(txn,
                            SqlStatement("DELETE FROM Singers WHERE true"));
          if (!status) return std::move(status).status();
          return Mutations{};
        });
    update_trials(delete_status.status());
  }
  std::cout << " DONE\n";

  double const number_of_trials = number_of_failures + number_of_successes;
  double const mid = number_of_successes / number_of_trials;
  double const r =
      z / number_of_trials *
      std::sqrt(number_of_failures / number_of_trials * number_of_successes);
  std::cout << "Estimated 95% confidence interval for success rate is ["
            << (mid - r) << "," << (mid + r) << "]\n";

  EXPECT_GT(mid - r, 1.0 - kThreshold)
      << " number_of_failures=" << number_of_failures
      << ", number_of_successes=" << number_of_successes
      << ", number_of_trials=" << number_of_trials << ", mid=" << mid
      << ", r=" << r << ", range=[ " << (mid - r) << " , " << (mid + r) << "]"
      << ", kTheshold=" << kThreshold;
}

}  // namespace
}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google
