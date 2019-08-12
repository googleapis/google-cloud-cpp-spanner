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

#include "google/cloud/spanner/internal/keys.h"
#include "google/cloud/spanner/testing/matchers.h"
#include <google/protobuf/text_format.h>
#include <google/spanner/v1/keys.pb.h>
#include <gmock/gmock.h>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {
namespace internal {
namespace {

TEST(InternalKeySetTest, ToProtoAll) {
  auto ks = KeySet::All();
  ::google::spanner::v1::KeySet expected;
  EXPECT_TRUE(::google::protobuf::TextFormat::ParseFromString(
      R"(
all: true
)",
      &expected));

  ::google::spanner::v1::KeySet result = internal::ToProto(ks);
  EXPECT_THAT(result, spanner_testing::IsProtoEqual(expected));
}

TEST(InternalKeySetTest, BuildToProtoTwoKeys) {
  auto ksb = KeySetBuilder<Row<std::string, std::string>>();
  ksb.Add(MakeRow("foo0", "bar0"));
  ksb.Add(MakeRow("foo1", "bar1"));

  KeySet ks = ksb.Build();

  ::google::spanner::v1::KeySet expected;
  EXPECT_TRUE(::google::protobuf::TextFormat::ParseFromString(
      R"(
keys {
  values {
    string_value: "foo0"
  }
  values {
    string_value: "bar0"
  }
}

keys {
  values {
    string_value: "foo1"
  }
  values {
    string_value: "bar1"
  }
}
all: false
)",
      &expected));
  ::google::spanner::v1::KeySet result = internal::ToProto(ks);

  EXPECT_THAT(result, spanner_testing::IsProtoEqual(expected));
}

TEST(InternalKeySetTest, BuildToProtoTwoRanges) {
  auto ksb = KeySetBuilder<Row<std::string, std::string>>(
      KeyRange<Row<std::string, std::string>>(MakeRow("start00", "start01"),
                                              MakeRow("end00", "end01")));
  auto range = KeyRange<Row<std::string, std::string>>(
      MakeBoundOpen(MakeRow("start10", "start11")),
      MakeBoundOpen(MakeRow("end10", "end11")));
  ksb.Add(range);

  KeySet ks = ksb.Build();

  ::google::spanner::v1::KeySet expected;
  EXPECT_TRUE(::google::protobuf::TextFormat::ParseFromString(
      R"(
ranges {
  start_closed {
    values {
      string_value: "start00"
    }
    values {
      string_value: "start01"
    }
  }

  end_closed {
    values {
      string_value: "end00"
    }
    values {
      string_value: "end01"
    }
  }
}

ranges {
  start_open {
    values {
      string_value: "start10"
    }
    values {
      string_value: "start11"
    }
  }

  end_open {
    values {
      string_value: "end10"
    }
    values {
      string_value: "end11"
    }
  }
}

all: false
)",
      &expected));
  ::google::spanner::v1::KeySet result = internal::ToProto(ks);

  EXPECT_THAT(result, spanner_testing::IsProtoEqual(expected));
}

}  // namespace
}  // namespace internal
}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google
