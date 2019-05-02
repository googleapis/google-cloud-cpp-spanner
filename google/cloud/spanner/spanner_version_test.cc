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

#include "google/cloud/internal/build_info.h"
#include "google/cloud/spanner/version.h"
#include <gmock/gmock.h>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {
using ::testing::HasSubstr;
using ::testing::Not;
using ::testing::StartsWith;

/// @test A trivial test for the Google Cloud Spanner C++ Client
TEST(StorageVersionTest, Simple) {
  EXPECT_FALSE(spanner::version_string().empty());
  EXPECT_EQ(SPANNER_CLIENT_VERSION_MAJOR, spanner::version_major());
  EXPECT_EQ(SPANNER_CLIENT_VERSION_MINOR, spanner::version_minor());
  EXPECT_EQ(SPANNER_CLIENT_VERSION_PATCH, spanner::version_patch());
}

/// @test Verify the version string starts with the version numbers.
TEST(StorageVersionTest, Format) {
  std::ostringstream os;
  os << "v" << SPANNER_CLIENT_VERSION_MAJOR << "."
     << SPANNER_CLIENT_VERSION_MINOR << "." << SPANNER_CLIENT_VERSION_PATCH;
  EXPECT_THAT(version_string(), StartsWith(os.str()));
}

/// @test Verify the version does not contain build info for release builds.
TEST(StorageVersionTest, NoBuildInfoInRelease) {
  if (!google::cloud::internal::is_release()) {
    return;
  }
  EXPECT_THAT(version_string(),
              Not(HasSubstr("+" + google::cloud::internal::gitrev())));
}

/// @test Verify the version has the build info for development builds.
TEST(StorageVersionTest, HasBuildInfoInDevelopment) {
  if (google::cloud::internal::is_release()) {
    return;
  }
  EXPECT_THAT(version_string(),
              HasSubstr("+" + google::cloud::internal::gitrev()));
}

}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google
