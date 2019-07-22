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

#include "google/cloud/spanner/internal/merge_chunk.h"
#include "google/cloud/spanner/value.h"
#include "google/cloud/testing_util/assert_ok.h"
#include <google/protobuf/struct.pb.h>
#include <google/protobuf/text_format.h>
#include <google/protobuf/util/message_differencer.h>
#include <gmock/gmock.h>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {
namespace internal {
namespace {

//
// MakeProtoValue() is an overloaded helper function for creating
// google::protobuf::Value protos from convenient user-supplied arguments.
//

google::protobuf::Value MakeProtoValue(Value v) {
  google::protobuf::Value value;
  std::tie(std::ignore, value) = internal::ToProto(std::move(v));
  return value;
}

google::protobuf::Value MakeProtoValue(std::string s) {
  google::protobuf::Value value;
  value.set_string_value(std::move(s));
  return value;
}

google::protobuf::Value MakeProtoValue(double d) {
  google::protobuf::Value value;
  value.set_number_value(d);
  return value;
}

template <typename T>
google::protobuf::Value MakeProtoValue(std::vector<T> v) {
  google::protobuf::Value value;
  for (auto& e : v) {
    *value.mutable_list_value()->add_values() = MakeProtoValue(std::move(e));
  }
  return value;
}

// Example from
// https://github.com/googleapis/googleapis/blob/master/google/spanner/v1/result_set.proto
//
// "foo", "bar" => "foobar"
TEST(MergeChunk, ExampleStrings) {
  auto a = MakeProtoValue("foo");
  auto b = MakeProtoValue("bar");
  ASSERT_STATUS_OK(MergeChunk(a, std::move(b)));

  auto expected = MakeProtoValue("foobar");
  std::string delta;
  google::protobuf::util::MessageDifferencer differencer;
  differencer.ReportDifferencesToString(&delta);
  EXPECT_TRUE(differencer.Compare(a, expected)) << delta;
}

// Example from
// https://github.com/googleapis/googleapis/blob/master/google/spanner/v1/result_set.proto
//
// [2, 3], [4] => [2, 3, 4]
TEST(MergeChunk, ExampleListOfInts) {
  auto a = MakeProtoValue(std::vector<double>{2, 3});
  auto b = MakeProtoValue(std::vector<double>{4});
  ASSERT_STATUS_OK(MergeChunk(a, std::move(b)));

  auto expected = MakeProtoValue(std::vector<double>{2, 3, 4});
  std::string delta;
  google::protobuf::util::MessageDifferencer differencer;
  differencer.ReportDifferencesToString(&delta);
  EXPECT_TRUE(differencer.Compare(a, expected)) << delta;
}

// Example from
// https://github.com/googleapis/googleapis/blob/master/google/spanner/v1/result_set.proto
//
// ["a", "b"], ["c", "d"] => ["a", "bc", "d"]
TEST(MergeChunk, ExampleListOfStrings) {
  auto a = MakeProtoValue(std::vector<std::string>{"a", "b"});
  auto b = MakeProtoValue(std::vector<std::string>{"c", "d"});
  ASSERT_STATUS_OK(MergeChunk(a, std::move(b)));

  auto expected = MakeProtoValue(std::vector<std::string>{"a", "bc", "d"});
  std::string delta;
  google::protobuf::util::MessageDifferencer differencer;
  differencer.ReportDifferencesToString(&delta);
  EXPECT_TRUE(differencer.Compare(a, expected)) << delta;
}

// Example from
// https://github.com/googleapis/googleapis/blob/master/google/spanner/v1/result_set.proto
//
// ["a", ["b", "c"]], [["d"], "e"] => ["a", ["b", "cd"], "e"]
TEST(MergeChunk, ExampleListsOfListOfString) {
  auto a = MakeProtoValue(std::vector<Value>{
      Value("a"), Value(std::vector<std::string>{"b", "c"})});
  auto b = MakeProtoValue(
      std::vector<Value>{Value(std::vector<std::string>{"d"}), Value("e")});
  ASSERT_STATUS_OK(MergeChunk(a, std::move(b)));

  auto expected = MakeProtoValue(std::vector<Value>{
      Value("a"), Value(std::vector<std::string>{"b", "cd"}), Value("e")});

  std::string delta;
  google::protobuf::util::MessageDifferencer differencer;
  differencer.ReportDifferencesToString(&delta);
  EXPECT_TRUE(differencer.Compare(a, expected)) << delta;
}

//
// Tests some edge cases that we think should probably work.
//

TEST(MergeChunk, EmptyString) {
  auto value = MakeProtoValue("foo");
  ASSERT_STATUS_OK(MergeChunk(value, MakeProtoValue("")));

  std::string delta;
  google::protobuf::util::MessageDifferencer differencer;
  differencer.ReportDifferencesToString(&delta);
  EXPECT_TRUE(differencer.Compare(value, MakeProtoValue("foo"))) << delta;

  auto empty = MakeProtoValue("");
  ASSERT_STATUS_OK(MergeChunk(empty, MakeProtoValue("bar")));

  delta.clear();
  EXPECT_TRUE(differencer.Compare(empty, MakeProtoValue("bar"))) << delta;
}

//
// Error cases
//

TEST(MergeChunk, ErrorMismatchedTypes) {
  auto value = MakeProtoValue(std::vector<std::string>{"hello"});
  auto status = MergeChunk(value, MakeProtoValue("world"));

  EXPECT_FALSE(status.ok());
  EXPECT_THAT(status.message(), testing::HasSubstr("mismatched types"));
}

//
// Tests the unsupported cases
//

TEST(MergeChunk, CannotMergeBools) {
  google::protobuf::Value bool1;
  bool1.set_bool_value(true);

  google::protobuf::Value bool2;
  bool2.set_bool_value(true);

  auto status = MergeChunk(bool1, std::move(bool2));
  EXPECT_FALSE(status.ok());
  EXPECT_THAT(status.message(), testing::HasSubstr("invalid type"));
}

TEST(MergeChunk, CannotMergeNumbers) {
  google::protobuf::Value number1;
  number1.set_number_value(1.0);

  google::protobuf::Value number2;
  number2.set_number_value(2.0);

  auto status = MergeChunk(number1, std::move(number2));
  EXPECT_FALSE(status.ok());
  EXPECT_THAT(status.message(), testing::HasSubstr("invalid type"));
}

TEST(MergeChunk, CannotMergeNull) {
  google::protobuf::Value null1;
  null1.set_null_value(google::protobuf::NullValue::NULL_VALUE);

  google::protobuf::Value null2;
  null2.set_null_value(google::protobuf::NullValue::NULL_VALUE);

  auto status = MergeChunk(null1, std::move(null2));
  EXPECT_FALSE(status.ok());
  EXPECT_THAT(status.message(), testing::HasSubstr("invalid type"));
}

TEST(MergeChunk, CannotMergeStruct) {
  google::protobuf::Value struct1;
  struct1.mutable_struct_value();

  google::protobuf::Value struct2;
  struct2.mutable_struct_value();

  auto status = MergeChunk(struct1, std::move(struct2));
  EXPECT_FALSE(status.ok());
  EXPECT_THAT(status.message(), testing::HasSubstr("invalid type"));
}

}  // namespace
}  // namespace internal
}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google
