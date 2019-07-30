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
#include "google/cloud/spanner/samples/sample_utils.h"
#include "google/cloud/internal/getenv.h"
#include "google/cloud/internal/random.h"
#include <sstream>
#include <tuple>
#include <utility>

namespace google {
namespace cloud {
namespace spanner {
namespace samples {

void AddColumn(std::vector<std::string> const& argv) {
  if (argv.size() != 3) {
    throw std::runtime_error(
        "add-column <project-id> <instance-id> <database-id>");
  }

  //! [update-database] [START spanner_add_column]
  using google::cloud::future;
  using google::cloud::StatusOr;
  [](std::string const& project_id, std::string const& instance_id,
     std::string const& database_id) {
    google::cloud::spanner::DatabaseAdminClient client;
    future<StatusOr<
        google::spanner::admin::database::v1::UpdateDatabaseDdlMetadata>>
        future = client.UpdateDatabase(
            project_id, instance_id, database_id,
            {"ALTER TABLE Albums ADD COLUMN MarketingBudget INT64"});
    StatusOr<google::spanner::admin::database::v1::UpdateDatabaseDdlMetadata>
        metadata = future.get();
    if (!metadata) {
      throw std::runtime_error(metadata.status().message());
    }
    std::cout << "Added MarketingBudget column\n";
  }
  //! [update-database] [END spanner_add_column]
  (argv[0], argv[1], argv[2]);
}

}  // namespace samples
}  // namespace spanner
}  // namespace cloud
}  // namespace google

namespace samples = google::cloud::spanner::samples;

int main(int ac, char* av[]) try {
  auto sample_suite = samples::SampleSuite({{
      "add-column",
      &samples::AddColumn,
  }});
  if (samples::AutoRun()) {
    sample_suite.run_all();
    return 0;
  }

  return sample_suite.run_one_command({av, av + ac});
} catch (std::exception const& ex) {
  std::cerr << ex.what() << "\n";
  return 1;
}
