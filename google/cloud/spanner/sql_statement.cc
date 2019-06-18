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
namespace {
bool CaseInsensitiveStringCompare(std::string const& lhs,
                                  std::string const& rhs) {
  if (lhs.size() == rhs.size()) {
    for (int i = 0; i < static_cast<int>(lhs.size()); ++i) {
      if (std::tolower(lhs[i]) != std::tolower(rhs[i])) {
        return false;
      }
    }
    return true;
  }
  return false;
}
}  // namespace

bool operator==(SqlStatement const& lhs, SqlStatement const& rhs) {
  return CaseInsensitiveStringCompare(lhs.statement_, rhs.statement_) &&
         lhs.params_ == rhs.params_;
}

bool operator!=(SqlStatement const& lhs, SqlStatement const& rhs) {
  return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& os, SqlStatement const& stmt) {
  os << stmt.statement_;
  for (auto const& param : stmt.params_) {
    os << "{" << param.first << ", "
       << "Parameter Value stringification not yet implemented"
       << "}";
  }
  return os;
}
}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google
