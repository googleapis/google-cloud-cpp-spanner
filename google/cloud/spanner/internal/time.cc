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

#include "google/cloud/spanner/internal/time.h"
#include <chrono>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <ratio>
#include <sstream>
#include <utility>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {
namespace internal {

// All the civil-time code assumes the proleptic Gregorian calendar, and
// 24-hour days divided into 60-minute hours and 60-second minutes.

using nanoseconds = std::chrono::nanoseconds;
using seconds = std::chrono::seconds;
using system_clock = std::chrono::system_clock;
using time_point = system_clock::time_point;

//
// Duration
//

google::protobuf::Duration ToProto(nanoseconds ns) {
  auto nanos = ns.count();
  google::protobuf::Duration proto;
  proto.set_seconds(nanos / nanoseconds::period::den);  // rounds toward zero
  proto.set_nanos(nanos % nanoseconds::period::den);    // (a/b)*b + a%b == a
  return proto;
}

void ToProto(nanoseconds ns, google::protobuf::Duration* proto) {
  *proto = ToProto(ns);
}

nanoseconds FromProto(google::protobuf::Duration const& proto) {
  nanoseconds ns(proto.nanos());
  ns += seconds(proto.seconds());
  return ns;
}

//
// Timestamp
//

google::protobuf::Timestamp ToProto(time_point tp) {
  std::time_t t = system_clock::to_time_t(tp);
  time_point ttp = system_clock::from_time_t(t);
  auto ss = std::chrono::duration_cast<nanoseconds>(tp - ttp);
  if (ss.count() < 0) {
    t -= 1;
    ss += seconds(1);
  }
  google::protobuf::Timestamp proto;
  proto.set_seconds(t);
  proto.set_nanos(ss.count());
  return proto;
}

void ToProto(time_point tp, google::protobuf::Timestamp* proto) {
  *proto = ToProto(tp);
}

time_point FromProto(google::protobuf::Timestamp const& proto) {
  time_point tp = system_clock::from_time_t(proto.seconds());
  tp += nanoseconds(proto.nanos());
  return tp;
}

namespace {

// A duration capable of holding subsecond values at high precision.
using femtoseconds = std::chrono::duration<std::int64_t, std::femto>;

// Convert a std::time_t into a Zulu std::tm.
std::tm ZTime(std::time_t const t) {
  std::time_t sec = t % (24 * 60 * 60);
  std::time_t day = t / (24 * 60 * 60);
  if (sec < 0) {
    sec += 24 * 60 * 60;
    day -= 1;
  }
  day += 719468;  // since 1970-01-01
  std::time_t const era = (day >= 0 ? day : day - 146096) / 146097;
  std::time_t const doe = day - era * 146097;
  std::time_t const yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
  std::time_t const y = yoe + era * 400;
  std::time_t const doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
  std::time_t const mp = (5 * doy + 2) / 153;
  std::time_t const d = doy - (153 * mp + 2) / 5 + 1;
  std::time_t const m = mp + (mp < 10 ? 3 : -9);
  std::tm tm;
  tm.tm_year = static_cast<int>(y + (m <= 2 ? 1 : 0) - 1900);
  tm.tm_mon = m - 1;
  tm.tm_mday = d;
  tm.tm_hour = sec / (60 * 60);
  tm.tm_min = (sec / 60) % 60;
  tm.tm_sec = sec % 60;
  return tm;
}

// Convert a Zulu std::tm into a std::time_t.
std::time_t TimeZ(std::tm const& tm) {
  std::time_t const y = tm.tm_year + 1900L;
  std::time_t const m = tm.tm_mon + 1;
  std::time_t const d = tm.tm_mday;
  std::time_t const eyear = (m <= 2) ? y - 1 : y;
  std::time_t const era = (eyear >= 0 ? eyear : eyear - 399) / 400;
  std::time_t const yoe = eyear - era * 400;
  std::time_t const doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
  std::time_t const doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
  std::time_t const day = era * 146097 + doe - 719468;  // since 1970-01-01
  return (((day * 24) + tm.tm_hour) * 60 + tm.tm_min) * 60 + tm.tm_sec;
}

// Split a time_point into a Zulu std::tm and a (>=0) femto subsecond.
std::pair<std::tm, femtoseconds> SplitTime(time_point tp) {
  std::time_t t = system_clock::to_time_t(tp);
  time_point ttp = system_clock::from_time_t(t);
  auto ss = std::chrono::duration_cast<femtoseconds>(tp - ttp);
  if (ss.count() < 0) {
    t -= 1;
    ss += seconds(1);
  }
  return {ZTime(t), ss};
}

// Combine a Zulu std::tm and a femto subsecond into a time_point.
time_point CombineTime(std::tm const& tm, femtoseconds ss) {
  auto sub = std::chrono::duration_cast<system_clock::duration>(ss);
  return system_clock::from_time_t(TimeZ(tm)) + sub;
}

// RFC3339 "date-time" prefix (no "time-secfrac" or "time-offset").
char const kTimeFormat[] = "%Y-%m-%dT%H:%M:%S";

// Returned by TimestampFromString() when the string fails to parse.
time_point const kBadTimestamp = time_point::min();

}  // namespace

std::string TimestampToString(time_point tp) {
  auto bd = SplitTime(tp);
  std::ostringstream output;
  output << std::put_time(&bd.first, kTimeFormat);
  if (auto ss = bd.second.count()) {  // femtoseconds
    int width = 15;                   // log10(std::femto::den)
    while (ss % 10 == 0) {
      ss /= 10;
      width -= 1;
    }
    output << '.' << std::setfill('0') << std::setw(width) << ss;
  }
  output << 'Z';  // Zulu
  return output.str();
}

time_point TimestampFromString(std::string const& s) {
  std::tm tm{};
  std::istringstream input(s);
  input >> std::get_time(&tm, kTimeFormat);
  if (!input || input.tellg() < 0) {
    // Failed to match kTimeFormat, or consumed the entire string.
    return kBadTimestamp;
  }
  std::string::size_type pos = input.tellg();
  auto const len = s.size();

  femtoseconds ss(0);
  if (s[pos] == '.') {
    femtoseconds::rep v = 0;
    auto scale = std::femto::den;
    auto fpos = pos + 1;  // start of fractional part
    while (++pos != len) {
      static char const k_digits[] = "0123456789";
      char const* dp = std::strchr(k_digits, s[pos]);
      if (dp == nullptr || *dp == '\0') break;  // non-digit
      if (scale == 1) continue;                 // drop insignificant digits
      scale /= 10;
      v *= 10;
      v += dp - k_digits;
    }
    if (pos == fpos) return kBadTimestamp;  // one digit is required
    ss = femtoseconds(v * scale);
  }

  if (pos == len || s[pos] != 'Z') return kBadTimestamp;  // no Zulu
  if (++pos != len) return kBadTimestamp;                 // trailing garbage

  return CombineTime(tm, ss);
}

}  // namespace internal
}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google
