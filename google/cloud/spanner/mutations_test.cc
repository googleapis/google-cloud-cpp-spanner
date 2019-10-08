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

#include "google/cloud/spanner/mutations.h"
#include "google/cloud/spanner/keys.h"
#include "google/cloud/spanner/testing/matchers.h"
#include <google/protobuf/text_format.h>
#include <google/protobuf/util/message_differencer.h>
#include <gmock/gmock.h>
#include <cstdint>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {
namespace {

namespace spanner_proto = ::google::spanner::v1;

using ::google::cloud::spanner_testing::IsProtoEqual;
using ::google::protobuf::TextFormat;
using ::testing::HasSubstr;

TEST(MutationsTest, Default) {
  Mutation actual;
  EXPECT_EQ(actual, actual);
}

TEST(MutationsTest, PrintTo) {
  Mutation insert =
      MakeInsertMutation("table-name", {}, std::string("test-string"));
  std::ostringstream os;
  PrintTo(insert, &os);
  auto actual = std::move(os).str();
  EXPECT_THAT(actual, HasSubstr("test-string"));
  EXPECT_THAT(actual, HasSubstr("Mutation={"));
}

TEST(MutationsTest, InsertSimple) {
  Mutation empty;
  Mutation insert =
      MakeInsertMutation("table-name", {"col_a", "col_b", "col_c"},
                         std::string("foo"), std::string("bar"), true);
  EXPECT_EQ(insert, insert);
  EXPECT_NE(insert, empty);

  auto actual = std::move(insert).as_proto();
  spanner_proto::Mutation expected;
  ASSERT_TRUE(TextFormat::ParseFromString(
      R"pb(
        insert: {
          columns: "col_a"
          columns: "col_b"
          columns: "col_c"
          table: "table-name"
          values: {
            values: { string_value: "foo" }
            values: { string_value: "bar" }
            values: { bool_value: true }
          }
        }
      )pb",
      &expected));
  EXPECT_THAT(actual, IsProtoEqual(expected));
}

TEST(MutationsTest, InsertComplex) {
  Mutation empty;
  auto builder =
      InsertMutationBuilder("table-name", {"col1", "col2", "col3"})
          .AddRow({Value(42), Value("foo"), Value(false)})
          .EmplaceRow(optional<std::int64_t>(), "bar", optional<bool>{});
  Mutation insert = builder.Build();
  Mutation moved = std::move(builder).Build();
  EXPECT_EQ(insert, moved);

  auto actual = std::move(insert).as_proto();
  spanner_proto::Mutation expected;
  ASSERT_TRUE(TextFormat::ParseFromString(
      R"pb(
        insert: {
          table: "table-name"
          columns: "col1"
          columns: "col2"
          columns: "col3"
          values: {
            values: { string_value: "42" }
            values: { string_value: "foo" }
            values: { bool_value: false }
          }
          values: {
            values: { null_value: NULL_VALUE }
            values: { string_value: "bar" }
            values: { null_value: NULL_VALUE }
          }
        }
      )pb",
      &expected));
  EXPECT_THAT(actual, IsProtoEqual(expected));
}

TEST(MutationsTest, UpdateSimple) {
  Mutation empty;
  Mutation update =
      MakeUpdateMutation("table-name", {"col_a", "col_b", "col_c"},
                         std::string("foo"), std::string("bar"), true);
  EXPECT_EQ(update, update);
  EXPECT_NE(update, empty);

  auto actual = std::move(update).as_proto();
  spanner_proto::Mutation expected;
  ASSERT_TRUE(TextFormat::ParseFromString(
      R"pb(
        update: {
          table: "table-name"
          columns: "col_a"
          columns: "col_b"
          columns: "col_c"
          values: {
            values: { string_value: "foo" }
            values: { string_value: "bar" }
            values: { bool_value: true }
          }
        }
      )pb",
      &expected));
  EXPECT_THAT(actual, IsProtoEqual(expected));
}

TEST(MutationsTest, UpdateComplex) {
  Mutation empty;
  auto builder =
      UpdateMutationBuilder("table-name", {"col_a", "col_b"})
          .AddRow({Value(std::vector<std::string>{}), Value(7.0)})
          .EmplaceRow(std::vector<std::string>{"a", "b"}, optional<double>{});
  Mutation update = builder.Build();
  Mutation moved = std::move(builder).Build();
  EXPECT_EQ(update, moved);

  auto actual = std::move(update).as_proto();
  spanner_proto::Mutation expected;
  ASSERT_TRUE(TextFormat::ParseFromString(
      R"pb(
        update: {
          table: "table-name"
          columns: "col_a"
          columns: "col_b"
          values: {
            values: { list_value: {} }
            values: { number_value: 7.0 }
          }
          values: {
            values: {
              list_value: {
                values: { string_value: "a" }
                values: { string_value: "b" }
              }
            }
            values: { null_value: NULL_VALUE }
          }
        }
      )pb",
      &expected));
  EXPECT_THAT(actual, IsProtoEqual(expected));
}

TEST(MutationsTest, InsertOrUpdateSimple) {
  Mutation empty;
  Mutation update =
      MakeInsertOrUpdateMutation("table-name", {"col_a", "col_b", "col_c"},
                                 std::string("foo"), std::string("bar"), true);
  EXPECT_EQ(update, update);
  EXPECT_NE(update, empty);

  auto actual = std::move(update).as_proto();
  spanner_proto::Mutation expected;
  ASSERT_TRUE(TextFormat::ParseFromString(
      R"pb(
        insert_or_update: {
          table: "table-name"
          columns: "col_a"
          columns: "col_b"
          columns: "col_c"
          values: {
            values: { string_value: "foo" }
            values: { string_value: "bar" }
            values: { bool_value: true }
          }
        }
      )pb",
      &expected));
  EXPECT_THAT(actual, IsProtoEqual(expected));
}

TEST(MutationsTest, InsertOrUpdateComplex) {
  Mutation empty;
  auto builder = InsertOrUpdateMutationBuilder("table-name", {"col_a", "col_b"})
                     .AddRow({Value(std::make_tuple("a", 7.0))})
                     .EmplaceRow(std::make_tuple("b", 8.0));
  Mutation update = builder.Build();
  Mutation moved = std::move(builder).Build();
  EXPECT_EQ(update, moved);

  auto actual = std::move(update).as_proto();
  spanner_proto::Mutation expected;
  ASSERT_TRUE(TextFormat::ParseFromString(
      R"pb(
        insert_or_update: {
          table: "table-name"
          columns: "col_a"
          columns: "col_b"
          values: {
            values: {
              list_value: {
                values: { string_value: "a" }
                values: { number_value: 7.0 }
              }
            }
          }
          values: {
            values: {
              list_value: {
                values: { string_value: "b" }
                values: { number_value: 8.0 }
              }
            }
          }
        }
      )pb",
      &expected));
  EXPECT_THAT(actual, IsProtoEqual(expected));
}

TEST(MutationsTest, ReplaceSimple) {
  Mutation empty;
  Mutation replace =
      MakeReplaceMutation("table-name", {"col_a", "col_b", "col_c"},
                          std::string("foo"), std::string("bar"), true);
  EXPECT_EQ(replace, replace);
  EXPECT_NE(replace, empty);

  auto actual = std::move(replace).as_proto();
  spanner_proto::Mutation expected;
  ASSERT_TRUE(TextFormat::ParseFromString(
      R"pb(
        replace: {
          table: "table-name"
          columns: "col_a"
          columns: "col_b"
          columns: "col_c"
          values: {
            values: { string_value: "foo" }
            values: { string_value: "bar" }
            values: { bool_value: true }
          }
        }
      )pb",
      &expected));
  EXPECT_THAT(actual, IsProtoEqual(expected));
}

TEST(MutationsTest, ReplaceComplex) {
  Mutation empty;
  auto builder = ReplaceMutationBuilder("table-name", {"col_a", "col_b"})
                     .EmplaceRow("a", 7.0)
                     .AddRow({Value("b"), Value(8.0)});
  Mutation update = builder.Build();
  Mutation moved = std::move(builder).Build();
  EXPECT_EQ(update, moved);

  auto actual = std::move(update).as_proto();
  spanner_proto::Mutation expected;
  ASSERT_TRUE(TextFormat::ParseFromString(
      R"pb(
        replace: {
          table: "table-name"
          columns: "col_a"
          columns: "col_b"
          values: {
            values: { string_value: "a" }
            values: { number_value: 7.0 }
          }
          values: {
            values: { string_value: "b" }
            values: { number_value: 8.0 }
          }
        }
      )pb",
      &expected));
  EXPECT_THAT(actual, IsProtoEqual(expected));
}

TEST(MutationsTest, DeleteSimple) {
  auto ks = KeySet().AddKey("key-to-delete");
  Mutation dele = MakeDeleteMutation("table-name", std::move(ks));
  EXPECT_EQ(dele, dele);

  Mutation empty;
  EXPECT_NE(dele, empty);

  auto actual = std::move(dele).as_proto();
  spanner_proto::Mutation expected;
  ASSERT_TRUE(TextFormat::ParseFromString(
      R"pb(
        delete: {
          table: "table-name"
          key_set: { keys: { values { string_value: "key-to-delete" } } }
        }
      )pb",
      &expected));
  EXPECT_THAT(actual, IsProtoEqual(expected));
}

}  // namespace
}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google
