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

#include "google/cloud/spanner/database.h"
#include "google/cloud/spanner/database_admin_client.h"

namespace spanner = google::cloud::spanner;

int main(int argc, char* argv[]) try {
  if (argc != 4) {
    std::string const cmd = argv[0];
    auto last_slash = std::string(argv[0]).find_last_of("/");
    std::cerr << "Usage: " << cmd.substr(last_slash + 1)
        << " <project-id> <instance-id> <database-id>\n";
    return 1;
  }

  google::cloud::spanner::Database const database(argv[1], argv[2], argv[3]);

  using google::cloud::future;
  using google::cloud::StatusOr;

  google::cloud::spanner::DatabaseAdminClient client;
  future<StatusOr<google::spanner::admin::database::v1::Database>>
      created_database =
          client.CreateDatabase(database, {R"""(
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
  StatusOr<google::spanner::admin::database::v1::Database> db =
      created_database.get();
  if (!db) {
      throw std::runtime_error(db.status().message());
  }
  std::cout << "Created database [" << database << "]\n";

  return 0;
} catch (std::exception const& ex) {
  std::cerr << "Standard exception raised: " << ex.what() << "\n";
  return 1;
}
