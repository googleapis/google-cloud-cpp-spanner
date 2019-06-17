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

class SqlStatement {
 public:
  using param_type = std::unordered_map<std::string, Value>;
  explicit SqlStatement(std::string statement) : statement_(std::move(statement)) {}
  SqlStatement(std::string statement, param_type params) : 
    statement_(std::move(statement)), params_(std::move(params)) {}

  friend bool operator==(SqlStatement const& lhs, SqlStatement const& rhs);
  friend bool operator!=(SqlStatement const& lhs, SqlStatement const& rhs);

  std::string const& sql() const {return statement_;}
  param_type const& params() const {return params_;}

 private:
  std::string statement_;
  param_type params_;
};

}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_SQL_STATEMENT_H_
