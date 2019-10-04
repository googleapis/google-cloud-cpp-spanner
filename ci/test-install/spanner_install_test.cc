// Copyright 2019 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "google/cloud/spanner/client.h"
#include "google/cloud/spanner/database.h"
#include "google/cloud/spanner/database_admin_client.h"
#include "google/cloud/internal/getenv.h"
#include "google/cloud/internal/random.h"
#include <functional>

namespace spanner = google::cloud::spanner;

std::function<void()> drop_database = []{};

int main(int argc, char* argv[]) try {
  if (argc != 1) {
    std::string const cmd = argv[0];
    auto last_slash = std::string(argv[0]).find_last_of("/");
    std::cerr << "Usage: " << cmd.substr(last_slash + 1) << "\n";
    return 1;
  }

  auto project_id =
      google::cloud::internal::GetEnv("GOOGLE_CLOUD_PROJECT").value_or("");
  if (project_id.empty()) {
    throw std::runtime_error(
        "The GOOGLE_CLOUD_PROJECT environment variable should be set to a "
        "non-empty value");
  }
  auto instance_id =
      google::cloud::internal::GetEnv("GOOGLE_CLOUD_CPP_SPANNER_INSTANCE")
          .value_or("");
  if (project_id.empty()) {
    throw std::runtime_error(
        "The GOOGLE_CLOUD_CPP_SPANNER_INSTANCE environment variable should be "
        "set to a non-empty value");
  }

  auto generator = google::cloud::internal::MakeDefaultPRNG();
  auto database_id =
      "db-" + google::cloud::internal::Sample(
                  generator, 20, "abcdefghijlkmnopqrstuvwxyz0123456789");

  namespace spanner = google::cloud::spanner;
  spanner::Database const database(project_id, instance_id, database_id);

  using google::cloud::future;
  using google::cloud::StatusOr;

  std::cout << "Creating database [" << database_id << "] " << std::flush;
  spanner::DatabaseAdminClient admin_client;
  future<StatusOr<google::spanner::admin::database::v1::Database>>
      created_database =
          admin_client.CreateDatabase(database, {R"""(
                        CREATE TABLE Singers (
                                SingerId   INT64 NOT NULL,
                                FirstName  STRING(1024),
                                LastName   STRING(1024),
                                SingerInfo BYTES(MAX)
                        ) PRIMARY KEY (SingerId))""",
                                                 R"""(CREATE TABLE Albums (
                                SingerId     INT64 NOT NULL,
                                AlbumId      INT64 NOT NULL,
                                AlbumTitle   STRING(MAX)
                        ) PRIMARY KEY (SingerId, AlbumId),
                        INTERLEAVE IN PARENT Singers ON DELETE CASCADE)"""});

  int i = 0;
  int const timeout = 120;
  while (++i < timeout) {
    auto status = created_database.wait_for(std::chrono::seconds(1));
    if (status == std::future_status::ready) break;
    std::cout << '.' << std::flush;
  }
  if (i >= timeout) {
    std::cout << " TIMEOUT\n";
    throw std::runtime_error("Timeout while creating database");
  }
  std::cout << " DONE\n";

  StatusOr<google::spanner::admin::database::v1::Database> db =
      created_database.get();
  if (!db) throw std::runtime_error(db.status().message());

  drop_database = [admin_client, database]() mutable {
    auto drop = admin_client.DropDatabase(database);
    if (!drop.ok()) throw std::runtime_error(drop.message());
    std::cout << "Database dropped\n";
  };

  spanner::Client client(spanner::MakeConnection(database));

  auto reader =
      client.ExecuteQuery(spanner::SqlStatement("SELECT 'Hello World'"));

  for (auto&& row : reader->Rows<spanner::Row<std::string>>()) {
    if (!row) throw std::runtime_error(row.status().message());
    std::cout << row->get<0>() << "\n";
  }

  drop_database();
  return 0;
} catch (std::exception const& ex) {
  std::cerr << "Standard exception raised: " << ex.what() << "\n";
  drop_database();
  return 1;
}
