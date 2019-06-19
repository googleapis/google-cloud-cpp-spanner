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

#ifndef GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_SQL_STATEMENT_H_
#define GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_SQL_STATEMENT_H_

#include "google/cloud/spanner/value.h"
#include <string>
#include <unordered_map>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {
/**
 * The SqlStatement class represents an SQL statement, that may or may not be
 * parameterized.
 *
 * Parameter placeholders are specified by `@<param name>` in the SQL string.
 * Values for parameters are a collection of `std::pair<std::string const,
 * google::cloud:spanner::Value>`.
 * @par Example:
 *
 *     google::cloud::spanner::SqlStatement stmt("select * from scmods where
 *     last_name = @param1", {{"param1", Value("Blues")}});
 */
class SqlStatement {
 public:
  /// Type alias for parameter collection.
  using ParamType = std::unordered_map<std::string, Value>;

  /// Copy and move.
  SqlStatement(SqlStatement const&) = default;
  SqlStatement(SqlStatement&&) = default;
  SqlStatement& operator=(SqlStatement const&) = default;
  SqlStatement& operator=(SqlStatement&&) = default;

  SqlStatement() = default;
  /// Constructs a SqlStatement without parameters.
  explicit SqlStatement(std::string statement)
      : statement_(std::move(statement)) {}
  /// Constructs a SqlStatement with specified parameters.
  SqlStatement(std::string statement, ParamType params)
      : statement_(std::move(statement)), params_(std::move(params)) {}

  std::string const& sql() const { return statement_; }
  ParamType const& params() const { return params_; }

  friend std::ostream& operator<<(std::ostream& os, SqlStatement const& stmt);

 private:
  std::string statement_;
  ParamType params_;
};

}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_SQL_STATEMENT_H_
