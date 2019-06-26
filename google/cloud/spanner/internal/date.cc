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

#include "google/cloud/spanner/internal/date.h"
#include <ctime>
#include <iomanip>
#include <sstream>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {
namespace internal {

namespace {

// RFC3339 "full-date".
constexpr auto kDateFormat = "%Y-%m-%d";

}  // namespace

std::string DateToString(Date d) {
  std::tm tm;
  tm.tm_year = static_cast<int>(d.year() - 1900);
  tm.tm_mon = d.month() - 1;
  tm.tm_mday = d.day();
  std::ostringstream output;
  output << std::put_time(&tm, kDateFormat);
  return output.str();
}

StatusOr<Date> DateFromString(std::string const& s) {
  std::tm tm{};
  std::istringstream input(s);
  input >> std::get_time(&tm, kDateFormat);
  if (!input) {
    return Status(StatusCode::kInvalidArgument,
                  s + ": Failed to match RFC3339 full-date");
  }
  if (input.tellg() >= 0) {
    return Status(StatusCode::kInvalidArgument,
                  s + ": Extra data after RFC3339 full-date");
  }
  return Date(tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
}

}  // namespace internal
}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google
