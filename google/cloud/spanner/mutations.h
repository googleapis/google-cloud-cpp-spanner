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

#ifndef GOOGLE_CLOUD_CPP_SPANNER_GOOGLE_CLOUD_SPANNER_MUTATIONS_H_
#define GOOGLE_CLOUD_CPP_SPANNER_GOOGLE_CLOUD_SPANNER_MUTATIONS_H_

#include "google/cloud/spanner/keys.h"
#include "google/cloud/spanner/value.h"
#include <google/spanner/v1/mutation.pb.h>
#include <vector>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {

namespace internal {
template <typename Op>
class WriteMutationBuilder;
class DeleteMutationBuilder;
}  // namespace internal

/**
 * A wrapper for Cloud Spanner mutations.
 *
 * In addition to the Data Manipulation Language (DML) based APIs, Cloud Spanner
 * supports the mutation API, where the application describes data modification
 * using a data structure instead of a SQL statement.
 *
 * This class serves as a wrapper for all mutations types. Use the builders,
 * such as `InsertMutationBuilder` or `UpdateMutationBuilder` to create objects
 * of this class.
 *
 * @see https://cloud.google.com/spanner/docs/modify-mutation-api
 *   for more information about the Cloud Spanner mutation API.
 */
class Mutation {
 public:
  /**
   * Creates an empty mutation.
   *
   * @note Empty mutations are not usable with the Cloud Spanner mutation API.
   */
  Mutation() = default;

  Mutation(Mutation&&) noexcept = default;
  Mutation& operator=(Mutation&&) noexcept = default;
  Mutation(Mutation const&) = default;
  Mutation& operator=(Mutation const&) = default;

  friend bool operator==(Mutation const& lhs, Mutation const& rhs);
  friend bool operator!=(Mutation const& lhs, Mutation const& rhs) {
    return !(lhs == rhs);
  }

  /// Convert the mutation to the underlying proto.
  google::spanner::v1::Mutation as_proto() && { return std::move(m_); }
  google::spanner::v1::Mutation as_proto() const& { return m_; }

  /**
   * Allows Google Test to print internal debugging information when test
   * assertions fail.
   *
   * @warning This is intended for debugging and human consumption only, not
   *   machine consumption as the output format may change without notice.
   */
  friend void PrintTo(Mutation const& m, std::ostream* os);

 private:
  template <typename Op>
  friend class internal::WriteMutationBuilder;
  friend class internal::DeleteMutationBuilder;
  explicit Mutation(google::spanner::v1::Mutation m) : m_(std::move(m)) {}

  google::spanner::v1::Mutation m_;
};

/**
 * An ordered sequence of mutations to pass to `Client::Commit()` or return
 * from the `Client::Commit()` mutator.
 */
using Mutations = std::vector<Mutation>;

// This namespace contains implementation details. It is not part of the public
// API, and subject to change without notice.
namespace internal {

template <typename Op>
class WriteMutationBuilder {
 public:
  WriteMutationBuilder(std::string table_name,
                       std::vector<std::string> column_names) {
    auto& field = Op::mutable_field(m_);
    field.set_table(std::move(table_name));
    field.mutable_columns()->Reserve(static_cast<int>(column_names.size()));
    for (auto& name : column_names) {
      *field.add_columns() = std::move(name);
    }
  }

  Mutation Build() const& { return Mutation(m_); }
  Mutation Build() && { return Mutation(std::move(m_)); }

  WriteMutationBuilder& AddRow(std::vector<Value> values) {
    auto& lv = *Op::mutable_field(m_).add_values();
    for (auto& v : values) {
      std::tie(std::ignore, *lv.add_values()) = internal::ToProto(std::move(v));
    }
    return *this;
  }

  template <typename... Ts>
  WriteMutationBuilder& EmplaceRow(Ts&&... values) {
    return AddRow({Value(std::forward<Ts>(values))...});
  }

 private:
  google::spanner::v1::Mutation m_;
};

struct InsertOp {
  static google::spanner::v1::Mutation::Write& mutable_field(
      google::spanner::v1::Mutation& m) {
    return *m.mutable_insert();
  }
};

struct UpdateOp {
  static google::spanner::v1::Mutation::Write& mutable_field(
      google::spanner::v1::Mutation& m) {
    return *m.mutable_update();
  }
};

struct InsertOrUpdateOp {
  static google::spanner::v1::Mutation::Write& mutable_field(
      google::spanner::v1::Mutation& m) {
    return *m.mutable_insert_or_update();
  }
};

struct ReplaceOp {
  static google::spanner::v1::Mutation::Write& mutable_field(
      google::spanner::v1::Mutation& m) {
    return *m.mutable_replace();
  }
};

class DeleteMutationBuilder {
 public:
  DeleteMutationBuilder(std::string table_name, KeySet keys) {
    auto& field = *m_.mutable_delete_();
    field.set_table(std::move(table_name));
    *field.mutable_key_set() = internal::ToProto(std::move(keys));
  }

  Mutation Build() const& { return Mutation(m_); }
  Mutation Build() && { return Mutation(std::move(m_)); }

 private:
  google::spanner::v1::Mutation m_;
};

}  // namespace internal

/**
 * A helper class to construct "insert" mutations.
 *
 * @see The Mutation class documentation for an overview of the Cloud Spanner
 *   mutation API
 *
 * @see https://cloud.google.com/spanner/docs/modify-mutation-api
 *   for more information about the Cloud Spanner mutation API.
 */
using InsertMutationBuilder =
    internal::WriteMutationBuilder<internal::InsertOp>;

/// Creates a simple insert mutation for the values in @p values.
template <typename... Ts>
Mutation MakeInsertMutation(std::string table_name,
                            std::vector<std::string> columns, Ts&&... values) {
  return InsertMutationBuilder(std::move(table_name), std::move(columns))
      .EmplaceRow(std::forward<Ts>(values)...)
      .Build();
}

/**
 * A helper class to construct "update" mutations.
 *
 * @see The Mutation class documentation for an overview of the Cloud Spanner
 *   mutation API
 *
 * @see https://cloud.google.com/spanner/docs/modify-mutation-api
 *   for more information about the Cloud Spanner mutation API.
 */
using UpdateMutationBuilder =
    internal::WriteMutationBuilder<internal::UpdateOp>;

/// Creates a simple update mutation for the values in @p values.
template <typename... Ts>
Mutation MakeUpdateMutation(std::string table_name,
                            std::vector<std::string> columns, Ts&&... values) {
  return UpdateMutationBuilder(std::move(table_name), std::move(columns))
      .EmplaceRow(std::forward<Ts>(values)...)
      .Build();
}

/**
 * A helper class to construct "insert_or_update" mutations.
 *
 * @see The Mutation class documentation for an overview of the Cloud Spanner
 *   mutation API
 *
 * @see https://cloud.google.com/spanner/docs/modify-mutation-api
 *   for more information about the Cloud Spanner mutation API.
 */
using InsertOrUpdateMutationBuilder =
    internal::WriteMutationBuilder<internal::InsertOrUpdateOp>;

/// Creates a simple "insert or update" mutation for the values in @p values.
template <typename... Ts>
Mutation MakeInsertOrUpdateMutation(std::string table_name,
                                    std::vector<std::string> columns,
                                    Ts&&... values) {
  return InsertOrUpdateMutationBuilder(std::move(table_name),
                                       std::move(columns))
      .EmplaceRow(std::forward<Ts>(values)...)
      .Build();
}

/**
 * A helper class to construct "insert_or_update" mutations.
 *
 * @see The Mutation class documentation for an overview of the Cloud Spanner
 *   mutation API
 *
 * @see https://cloud.google.com/spanner/docs/modify-mutation-api
 *   for more information about the Cloud Spanner mutation API.
 */
using ReplaceMutationBuilder =
    internal::WriteMutationBuilder<internal::ReplaceOp>;

/// Creates a simple "replace" mutation for the values in @p values.
template <typename... Ts>
Mutation MakeReplaceMutation(std::string table_name,
                             std::vector<std::string> columns, Ts&&... values) {
  return ReplaceMutationBuilder(std::move(table_name), std::move(columns))
      .EmplaceRow(std::forward<Ts>(values)...)
      .Build();
}

/**
 * A helper class to construct "delete" mutations.
 *
 * @see The Mutation class documentation for an overview of the Cloud Spanner
 *   mutation API
 *
 * @see https://cloud.google.com/spanner/docs/modify-mutation-api
 *   for more information about the Cloud Spanner mutation API.
 */
using DeleteMutationBuilder = internal::DeleteMutationBuilder;

/// Creates a simple "delete" mutation for the values in @p keys.
inline Mutation MakeDeleteMutation(std::string table_name, KeySet keys) {
  return DeleteMutationBuilder(std::move(table_name), std::move(keys)).Build();
}

}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_SPANNER_GOOGLE_CLOUD_SPANNER_MUTATIONS_H_
