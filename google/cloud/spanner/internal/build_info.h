// Copyright 2017 Google Inc.
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

#ifndef GOOGLE_CLOUD_CPP_SPANNER_GOOGLE_CLOUD_SPANNER_INTERNAL_BUILD_INFO_H_
#define GOOGLE_CLOUD_CPP_SPANNER_GOOGLE_CLOUD_SPANNER_INTERNAL_BUILD_INFO_H_

#include "google/cloud/spanner/version.h"

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {
namespace internal {

/**
 * Returns the compiler.
 *
 * This is usually the basename of the compile command that was used. For
 * example, "clang" or "gcc". In the case of "g++" or "clang++", they will be
 * rewritten to "gxx" and "clangxx" since '+' is an illegal character for the
 * server-side fields that look at these values.
 */
std::string CompilerName();

/**
 * Returns the compiler version.
 *
 * This string may be something simple like "9.1.1" or it may be something more
 * verbose like "clang-version-8.0.0-Fedora-8.0.0-1.fc30".
 */
std::string CompilerVersion();

/**
 * Returns the compiler flags.
 *
 * Examples include "-c fastbuild" or "-O2 -DNDEBUG".
 */
std::string CompilerFlags();

/**
 * Returns the 4-digit year of the C++ language standard along with an
 * exception indicator.
 *
 * The returned string is suffixed with "-ex" or "-noex" to indicate whether or
 * not the code was compiled with exceptions enabled.
 *
 * Example return values: "2011-ex", "2017-noex" "unknown-ex"
 */
std::string LanguageVersion();

/**
 * Returns true if this is a release branch.
 */
bool IsRelease();

/**
 * Returns the metadata injected by the build system.
 *
 * The returned info is the string "sha." followed by a short commit hash. See
 * https://semver.org/#spec-item-10 for more details about the use and format
 * of build metadata.
 */
std::string BuildMetadata();

}  // namespace internal
}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_SPANNER_GOOGLE_CLOUD_SPANNER_INTERNAL_BUILD_INFO_H_
