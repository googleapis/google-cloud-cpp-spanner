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

#include "google/cloud/spanner/sql_statement.h"
#include <algorithm>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {

bool operator==(SqlStatement const& lhs, SqlStatement const& rhs) {
  std::string s1 = lhs.statement_;
  std::string s2 = rhs.statement_;
  std::transform(std::begin(s1), std::end(s1), std::begin(s1),
                 [](unsigned char c) { return std::tolower(c); });
  std::transform(std::begin(s2), std::end(s2), std::begin(s2),
                 [](unsigned char c) { return std::tolower(c); });
  return s1 == s2 && lhs.params_ == rhs.params_;
}

bool operator!=(SqlStatement const& lhs, SqlStatement const& rhs) {
  return !(lhs == rhs);
}

}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google
