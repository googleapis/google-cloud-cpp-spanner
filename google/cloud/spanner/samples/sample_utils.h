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

#ifndef GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_SAMPLES_SAMPLE_UTILS_H_
#define GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_SAMPLES_SAMPLE_UTILS_H_

#include "google/cloud/spanner/database_admin_client.h"
#include "google/cloud/spanner/samples/sample_utils.h"
#include "google/cloud/internal/getenv.h"
#include "google/cloud/internal/random.h"
#include <sstream>

namespace google {
namespace cloud {
namespace spanner {
namespace samples {

std::string RandomDatabaseName(google::cloud::internal::DefaultPRNG& generator);

void CreateDatabase(std::vector<std::string> const& argv);

void DropDatabase(std::vector<std::string> const& argv);

bool AutoRun();

using CommandType = std::function<void(std::vector<std::string> const&)>;

using CommandPairType = std::pair<std::string, CommandType>;

class SampleSuite {
 public:
  void run_all();
  int run_one_command(std::vector<std::string> argv);
  SampleSuite(std::vector<CommandPairType>);

 private:
  std::map<std::string, CommandType> commands_;
  std::vector<std::string> command_order_;
};

}  // namespace samples
}  // namespace spanner
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_SAMPLES_SAMPLE_UTILS_H_
