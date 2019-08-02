// Copyright 2019 Google Inc.
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

#include "google/cloud/spanner/internal/build_info.h"
#include "google/cloud/internal/port_platform.h"
#include <algorithm>
#include <cctype>
#include <iterator>
#include <sstream>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {
namespace internal {

/**
 * The macros for determining the compiler ID are taken from:
 * https://gitlab.kitware.com/cmake/cmake/tree/v3.5.0/Modules/Compiler/\*-DetermineCompiler.cmake
 * We do not care to detect every single compiler possible and only target the
 * most popular ones.
 */
std::string CompilerId() {
#if defined(__GNUC__)
  return "GNU";
#endif

#if defined(__clang__)
  return "Clang";
#endif

#if defined(_MSC_VER)
  return "MSVC";
#endif

  return "Unknown";
}

std::string CompilerVersion() {
  std::ostringstream os;

#if defined(__GNUC__)
  os << __GNUC__ << "." << __GNUC_MINOR__ << "." << __GNUC_PATCHLEVEL__;
  return std::move(os).str();
#endif

#if defined(__clang__)
  os << __clang_major__ << "." << __clang_minor__ << "."
     << __clang_patchlevel__;
  return std::move(os).str();
#endif

#if defined(_MSC_VER)
  os << _MSC_VER / 100 << ".";
  os << _MSC_VER % 100;
#if defined(_MSC_FULL_VER)
#if _MSC_VER >= 1400
  out << "." << _MSC_FULL_VER % 100000;
#else
  out << "." << _MSC_FULL_VER % 10000;
#endif
#endif
  return std::move(os).str();
#endif

  return "Unknown";
}

}  // namespace internal
}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google
