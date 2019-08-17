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

#include "google/cloud/spanner/database.h"
#include <array>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {

Database::Database(std::string project_id, std::string instance_id,
                   std::string database_id)
    : project_id_(std::move(project_id)),
      instance_id_(std::move(instance_id)),
      database_id_(std::move(database_id)) {}

std::string Database::FullName() const {
  // "projects/<project-id>/instances/<instance-id>/databases/<database-id>"
  constexpr char kProjects[] = "projects/";
  constexpr char kInstances[] = "/instances/";
  constexpr char kDatabases[] = "/databases/";
  std::string name;
  name.reserve(sizeof(kProjects) + sizeof(kInstances) + sizeof(kDatabases) - 3 +
               project_id().size() + instance_id().size() +
               database_id().size());
  name += kProjects;
  name += project_id();
  name += kInstances;
  name += instance_id();
  name += kDatabases;
  name += database_id();
  return name;
}

bool operator==(Database const& a, Database const& b) {
  return std::tie(a.project_id(), a.instance_id(), a.database_id()) ==
         std::tie(b.project_id(), b.instance_id(), b.database_id());
}

bool operator!=(Database const& a, Database const& b) { return !(a == b); }

std::ostream& operator<<(std::ostream& os, Database const& dn) {
  return os << dn.FullName();
}

}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google
