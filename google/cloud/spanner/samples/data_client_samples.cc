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

void QueryWithStruct(std::vector<std::string> const&) {
  // TODO(#188): Add querying part once data client is ready.
  // [START spanner_create_struct_with_data]
  auto singer_info = std::make_tuple(std::make_pair("FirstName", "Elena"),
                                     std::make_pair("LastName", "Campbell"));
  // [END spanner_create_struct_with_data]
  std::cout << "Struct created with the following data:\n"
            << std::get<0>(singer_info).first << ":"
            << std::get<0>(singer_info).second << "\n"
            << std::get<1>(singer_info).first << ":"
            << std::get<1>(singer_info).second << "\n";
}

}  // namespace samples
}  // namespace spanner
}  // namespace cloud
}  // namespace google

namespace samples = google::cloud::spanner::samples;

int main(int ac, char* av[]) try {
  auto sample_suite = samples::SampleSuite({{
      "query-with-struct",
      &samples::QueryWithStruct,
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
