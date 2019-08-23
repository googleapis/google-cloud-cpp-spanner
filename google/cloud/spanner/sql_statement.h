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

#ifndef GOOGLE_CLOUD_CPP_SPANNER_GOOGLE_CLOUD_SPANNER_SQL_STATEMENT_H_
#define GOOGLE_CLOUD_CPP_SPANNER_GOOGLE_CLOUD_SPANNER_SQL_STATEMENT_H_

#include "google/cloud/spanner/value.h"
#include "google/cloud/status_or.h"
#include <google/spanner/v1/spanner.pb.h>
#include <string>
#include <unordered_map>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {

class SqlStatement;  // Defined later in this file.
// Internal implementation details that callers should not use.
namespace internal {
// Use this proto type because it conveniently wraps all three attributes
// required to represent a SQL statement.
using SqlStatementProto =
    google::spanner::v1::ExecuteBatchDmlRequest::Statement;
SqlStatementProto ToProto(SqlStatement s);
}  // namespace internal

/**
 * @brief Represents a potentially parameterized SQL statement.
 *
 * Details on case sensitivity for SQL statements and string values can be
 * found here: <a
 * href="https://cloud.google.com/spanner/docs/lexical#case-sensitivity">Case
 * Sensitivity</a>.
 *
 * @note `SqlStatement` supports case-sensitive equality comparisons.
 *
 * Parameter placeholders are specified by `@<param name>` in the SQL string.
 * Values for parameters are a collection of `std::pair<std::string const,
 * google::cloud:spanner::Value>`.
 *
 * @warning SqlStatement requires compile-time string literals for the
 *     `statement` in order to protect against SQL injection attacks. If you
 *     need to compute the SQL query string dynamically at runtime, you may use
 *     the `MakeUntrustedSqlStatement()` function defined below. It is the
 *     caller's responsibility to ensure the dynamically generated string is
 *     safe.
 *
 * @par Example
 *
 *     using google::cloud::spanner::SqlStatement;
 *     using google::cloud::spanner::Value;
 *     SqlStatement stmt("select * from scmods where last_name = @param1",
 *       {{"param1", Value("Blues")}});
 *     SqlStatement copy = stmt;
 *     assert(copy == stmt);
 */
class SqlStatement {
 public:
  /// Type alias for parameter collection.
  using ParamType = std::unordered_map<std::string, Value>;

  SqlStatement() = default;

  /// Constructs an SqlStatement from the given string literal.
  template <std::size_t N>
  explicit SqlStatement(char const (&arr)[N]) : SqlStatement(arr, {}) {}

  /// Constructs an SqlStatement from the given string literal and @p params.
  template <std::size_t N>
  SqlStatement(char const (&arr)[N], ParamType params)
      : statement_(arr, N - 1), params_(std::move(params)) {
    // TODO(XXX): Look into compiler-specific extensions/attributes that will
    // allow us better enforce that these are compile-time literals.
    static_assert(N > 0, "String literal required");
  }

  /// Copy and move.
  SqlStatement(SqlStatement const&) = default;
  SqlStatement(SqlStatement&&) = default;
  SqlStatement& operator=(SqlStatement const&) = default;
  SqlStatement& operator=(SqlStatement&&) = default;

  std::string const& sql() const { return statement_; }
  ParamType const& params() const { return params_; }
  std::vector<std::string> ParameterNames() const;
  google::cloud::StatusOr<Value> GetParameter(
      std::string const& parameter_name) const;

  friend bool operator==(SqlStatement const& a, SqlStatement const& b) {
    return a.statement_ == b.statement_ && a.params_ == b.params_;
  }
  friend bool operator!=(SqlStatement const& a, SqlStatement const& b) {
    return !(a == b);
  }

 private:
  // Constructs an SqlStatement with specified parameters.
  SqlStatement(std::string statement, ParamType params)
      : statement_(std::move(statement)), params_(std::move(params)) {}

  friend std::ostream& operator<<(std::ostream& os, SqlStatement const& stmt);
  friend internal::SqlStatementProto internal::ToProto(SqlStatement s);
  friend SqlStatement MakeUntrustedSqlStatement(std::string statement,
                                                SqlStatement::ParamType params);

  std::string statement_;
  ParamType params_;
};

/**
 * Constructs an SqlStatement form the given string and optional params.
 *
 * @warning It is the caller's responsibility to ensure that the `statement`
 *     argument is safe and free from SQL injection attacks. Whenver possible,
 *     it is safer to use the `SqlStatement` constructors which require (as
 *     much as possible) that the SQL string was given at compile-time and is
 *     therefore safe from user influence.
 *
 * @par Example
 *
 *   // Preferred
 *   SqlStatement sql1("select ....");  // Safe compile-time string
 *
 *   // When necessary
 *   std::string s;
 *   if (UserCodeToVerifyStringIsSafe(s)) {
 *     SqlStatement sql2 = MakeUntrusted(s);
 *   }
 */
inline SqlStatement MakeUntrustedSqlStatement(
    std::string statement, SqlStatement::ParamType params = {}) {
  return SqlStatement(std::move(statement), std::move(params));
}

}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_SPANNER_GOOGLE_CLOUD_SPANNER_SQL_STATEMENT_H_
