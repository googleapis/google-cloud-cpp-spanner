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

#ifndef GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_MUTATION_H_
#define GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_MUTATION_H_

#include "google/cloud/spanner/key.h"
#include "google/cloud/spanner/row.h"
#include "google/cloud/spanner/value.h"
#include "google/cloud/spanner/version.h"
#include <string>
#include <vector>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {
/**
 * Represents an INSERT/UPDATE/REPLACE/DELETE mutation.
 */
class Mutation {
 public:
  static Mutation Insert(std::string table, std::vector<Row> rows) {
    return Mutation(Operation::INSERT, std::move(table), std::move(rows));
  }

  static Mutation Update(std::string table, std::vector<Row> rows) {
    return Mutation(Operation::UPDATE, std::move(table), std::move(rows));
  }

  // uses the alt implementation; columns + vector<row values>>
  // Only do this for Update() for now.
  static Mutation Update(std::string table, std::vector<std::string> columns,
                         std::vector<std::vector<Value>> values) {
    return Mutation(Operation::UPDATE, std::move(table), std::move(columns),
                    std::move(values));
  }

  static Mutation InsertOrUpdate(std::string table, std::vector<Row> rows) {
    return Mutation(Operation::INSERT_OR_UPDATE, std::move(table),
                    std::move(rows));
  }

  static Mutation Replace(std::string table, std::vector<Row> rows) {
    return Mutation(Operation::REPLACE, std::move(table), std::move(rows));
  }

  static Mutation Delete(std::string table, KeySet keys) {
    return Mutation(std::move(table), std::move(keys));
  }
  // value type, ==, !=

 private:
  enum class Operation { INSERT, UPDATE, INSERT_OR_UPDATE, REPLACE, DELETE };

  // all but Delete
  //
  // All Rows must have the same columns and value types.
  Mutation(Operation operation, std::string table, std::vector<Row> rows)
      : operation_(operation),
        table_(std::move(table)),
        write_rows_(std::move(rows)) {}

  // Alternative implementation - columns followed by values.
  Mutation(Operation operation, std::string table,
           std::vector<std::string> columns,
           std::vector<std::vector<Value>> values)
      : operation_(operation),
        table_(std::move(table)),
        write_columns_(std::move(columns)),
        write_values_(std::move(values)) {}

  // Delete
  Mutation(std::string table, KeySet keys)
      : operation_(Operation::DELETE),
        table_(std::move(table)),
        delete_keys_(std::move(keys)) {}

  // TODO(salty) these data members are just temporary - clean this up.
  // Ultimately, this may just wrap a google::spanner::v1::Mutation proto.

  // These are set for all mutations.
  Operation operation_;
  std::string table_;

  // and one of these 3 groups:
  std::vector<Row> write_rows_;

  std::vector<std::string> write_columns_;
  std::vector<std::vector<Value>> write_values_;

  KeySet delete_keys_;
};
}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_MUTATION_H_
