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
#include <google/protobuf/text_format.h>
#include <google/protobuf/util/message_differencer.h>
#include <gmock/gmock.h>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {
namespace {

TEST(MutationsTest, Default) {
  Mutation actual;
  EXPECT_EQ(actual, actual);
}

TEST(MutationsTest, Insert) {
  Mutation empty;
  Mutation insert =
      MakeInsertMutation(std::string("foo"), std::string("bar"), true);
  EXPECT_EQ(insert, insert);
  EXPECT_NE(insert, empty);

  auto actual = std::move(insert).as_proto();
  google::spanner::v1::Mutation expected;
  ASSERT_TRUE(google::protobuf::TextFormat::ParseFromString(R"""(
              insert: {
                values {
                  values {
                    string_value: "foo"
                  }
                  values {
                    string_value: "bar"
                  }
                  values {
                    bool_value: true
                  }
                }
              }
              )""",
                                                            &expected));
  std::string delta;
  google::protobuf::util::MessageDifferencer differencer;
  differencer.ReportDifferencesToString(&delta);
  EXPECT_TRUE(differencer.Compare(actual, expected)) << delta;
}

}  // namespace
}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google
