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

#ifndef GOOGLE_CLOUD_CPP_SPANNER_GOOGLE_CLOUD_SPANNER_DATABASE_H_
#define GOOGLE_CLOUD_CPP_SPANNER_GOOGLE_CLOUD_SPANNER_DATABASE_H_

#include "google/cloud/spanner/version.h"
#include <ostream>
#include <string>
#include <tuple>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {

/**
 * This class identifies a Spanner Database 
 *
 * A Spanner database is identified by its `project_id`, `instance_id`, and
 * `database_id`.
 */
class Database {
 public:
  /// Constructs a Spanner Database identified by the given IDs.
  Database(std::string project_id, std::string instance_id,
           std::string database_id);

  /// @name Copy and move
  //@{
  Database(Database const&) = default;
  Database& operator=(Database const&) = default;
  Database(Database&&) = default;
  Database& operator=(Database&&) = default;
  //@}

  /// @name Accessors
  //@{
  std::string const& project_id() const { return project_id_; }
  std::string const& instance_id() const { return instance_id_; }
  std::string const& database_id() const { return database_id_; }
  //@}

  // Returns the fully qualified database name as a string of the form:
  // "projects/<project-id>/instances/<instance-id>/databases/<database-id>"
  std::string FullName() const;

 private:
  std::string project_id_;
  std::string instance_id_;
  std::string database_id_;
};

/// @name Equality operators
//@{
bool operator==(Database const& a, Database const& b);
bool operator!=(Database const& a, Database const& b);
//@}

/// Output the `FullName()` format.
std::ostream& operator<<(std::ostream& os, Database const& dn);

}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_SPANNER_GOOGLE_CLOUD_SPANNER_DATABASE_H_
