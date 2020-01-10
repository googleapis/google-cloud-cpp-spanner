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

#include "google/cloud/spanner/timestamp.h"
#include <google/protobuf/timestamp.pb.h>
#include <gmock/gmock.h>
#include <cstdint>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {
namespace {

using ::testing::HasSubstr;

google::protobuf::Timestamp MakeProtoTimestamp(std::int64_t seconds,
                                               std::int32_t nanos) {
  google::protobuf::Timestamp proto;
  proto.set_seconds(seconds);
  proto.set_nanos(nanos);
  return proto;
}

TEST(Timestamp, RegularSemantics) {
  Timestamp const ts = internal::TimestampFromProto(MakeProtoTimestamp(0, 0));

  Timestamp const copy1(ts);
  EXPECT_EQ(copy1, ts);

  Timestamp const copy2 = ts;
  EXPECT_EQ(copy2, ts);

  Timestamp assign;
  assign = ts;
  EXPECT_EQ(assign, ts);
}

TEST(Timestamp, RelationalOperators) {
  EXPECT_EQ(
      internal::TimestampFromProto(MakeProtoTimestamp(1576030524, 611422667)),
      internal::TimestampFromProto(MakeProtoTimestamp(1576030524, 611422667)));
  EXPECT_LE(
      internal::TimestampFromProto(MakeProtoTimestamp(1576030524, 611422667)),
      internal::TimestampFromProto(MakeProtoTimestamp(1576030524, 611422667)));
  EXPECT_GE(
      internal::TimestampFromProto(MakeProtoTimestamp(1576030524, 611422667)),
      internal::TimestampFromProto(MakeProtoTimestamp(1576030524, 611422667)));

  EXPECT_NE(
      internal::TimestampFromProto(MakeProtoTimestamp(1576030524, 611422667)),
      internal::TimestampFromProto(MakeProtoTimestamp(1576030524, 611422668)));
  EXPECT_LT(
      internal::TimestampFromProto(MakeProtoTimestamp(1576030524, 611422667)),
      internal::TimestampFromProto(MakeProtoTimestamp(1576030524, 611422668)));
  EXPECT_NE(
      internal::TimestampFromProto(MakeProtoTimestamp(1576030524, 611422667)),
      internal::TimestampFromProto(MakeProtoTimestamp(1576030525, 611422667)));
  EXPECT_LT(
      internal::TimestampFromProto(MakeProtoTimestamp(1576030524, 611422667)),
      internal::TimestampFromProto(MakeProtoTimestamp(1576030525, 611422667)));

  EXPECT_NE(
      internal::TimestampFromProto(MakeProtoTimestamp(1576030524, 611422668)),
      internal::TimestampFromProto(MakeProtoTimestamp(1576030524, 611422667)));
  EXPECT_GT(
      internal::TimestampFromProto(MakeProtoTimestamp(1576030524, 611422668)),
      internal::TimestampFromProto(MakeProtoTimestamp(1576030524, 611422667)));
  EXPECT_NE(
      internal::TimestampFromProto(MakeProtoTimestamp(1576030525, 611422667)),
      internal::TimestampFromProto(MakeProtoTimestamp(1576030524, 611422667)));
  EXPECT_GT(
      internal::TimestampFromProto(MakeProtoTimestamp(1576030525, 611422667)),
      internal::TimestampFromProto(MakeProtoTimestamp(1576030524, 611422667)));
}

TEST(Timestamp, FromRFC3339) {
  EXPECT_EQ(internal::TimestampFromProto(MakeProtoTimestamp(1561135942, 0)),
            internal::TimestampFromRFC3339("2019-06-21T16:52:22Z").value());
  EXPECT_EQ(
      internal::TimestampFromProto(MakeProtoTimestamp(1561135942, 9)),
      internal::TimestampFromRFC3339("2019-06-21T16:52:22.000000009Z").value());
  EXPECT_EQ(
      internal::TimestampFromProto(MakeProtoTimestamp(1561135942, 89)),
      internal::TimestampFromRFC3339("2019-06-21T16:52:22.000000089Z").value());
  EXPECT_EQ(
      internal::TimestampFromProto(MakeProtoTimestamp(1561135942, 789)),
      internal::TimestampFromRFC3339("2019-06-21T16:52:22.000000789Z").value());
  EXPECT_EQ(
      internal::TimestampFromProto(MakeProtoTimestamp(1561135942, 6789)),
      internal::TimestampFromRFC3339("2019-06-21T16:52:22.000006789Z").value());
  EXPECT_EQ(
      internal::TimestampFromProto(MakeProtoTimestamp(1561135942, 56789)),
      internal::TimestampFromRFC3339("2019-06-21T16:52:22.000056789Z").value());
  EXPECT_EQ(
      internal::TimestampFromProto(MakeProtoTimestamp(1561135942, 456789)),
      internal::TimestampFromRFC3339("2019-06-21T16:52:22.000456789Z").value());
  EXPECT_EQ(
      internal::TimestampFromProto(MakeProtoTimestamp(1561135942, 3456789)),
      internal::TimestampFromRFC3339("2019-06-21T16:52:22.003456789Z").value());
  EXPECT_EQ(
      internal::TimestampFromProto(MakeProtoTimestamp(1561135942, 23456789)),
      internal::TimestampFromRFC3339("2019-06-21T16:52:22.023456789Z").value());
  EXPECT_EQ(
      internal::TimestampFromProto(MakeProtoTimestamp(1561135942, 123456789)),
      internal::TimestampFromRFC3339("2019-06-21T16:52:22.123456789Z").value());
  EXPECT_EQ(
      internal::TimestampFromProto(MakeProtoTimestamp(1561135942, 123456780)),
      internal::TimestampFromRFC3339("2019-06-21T16:52:22.12345678Z").value());
  EXPECT_EQ(
      internal::TimestampFromProto(MakeProtoTimestamp(1561135942, 123456700)),
      internal::TimestampFromRFC3339("2019-06-21T16:52:22.1234567Z").value());
  EXPECT_EQ(
      internal::TimestampFromProto(MakeProtoTimestamp(1561135942, 123456000)),
      internal::TimestampFromRFC3339("2019-06-21T16:52:22.123456Z").value());
  EXPECT_EQ(
      internal::TimestampFromProto(MakeProtoTimestamp(1561135942, 123450000)),
      internal::TimestampFromRFC3339("2019-06-21T16:52:22.12345Z").value());
  EXPECT_EQ(
      internal::TimestampFromProto(MakeProtoTimestamp(1561135942, 123400000)),
      internal::TimestampFromRFC3339("2019-06-21T16:52:22.1234Z").value());
  EXPECT_EQ(
      internal::TimestampFromProto(MakeProtoTimestamp(1561135942, 123000000)),
      internal::TimestampFromRFC3339("2019-06-21T16:52:22.123Z").value());
  EXPECT_EQ(
      internal::TimestampFromProto(MakeProtoTimestamp(1561135942, 120000000)),
      internal::TimestampFromRFC3339("2019-06-21T16:52:22.12Z").value());
  EXPECT_EQ(
      internal::TimestampFromProto(MakeProtoTimestamp(1561135942, 100000000)),
      internal::TimestampFromRFC3339("2019-06-21T16:52:22.1Z").value());
}

TEST(Timestamp, FromRFC3339Offset) {
  EXPECT_EQ(
      internal::TimestampFromProto(MakeProtoTimestamp(1546398245, 0)),
      internal::TimestampFromRFC3339("2019-01-02T03:04:05+00:00").value());
  EXPECT_EQ(
      internal::TimestampFromProto(
          MakeProtoTimestamp(1546398245 + 3600 + 120, 0)),
      internal::TimestampFromRFC3339("2019-01-02T03:04:05+01:02").value());
  EXPECT_EQ(
      internal::TimestampFromProto(
          MakeProtoTimestamp(1546398245 - 3600 - 120, 0)),
      internal::TimestampFromRFC3339("2019-01-02T03:04:05-01:02").value());
}

TEST(Timestamp, FromRFC3339Failure) {
  EXPECT_FALSE(internal::TimestampFromRFC3339(""));
  EXPECT_FALSE(internal::TimestampFromRFC3339("garbage in"));
  EXPECT_FALSE(internal::TimestampFromRFC3339("2019-06-21T16:52:22"));
  EXPECT_FALSE(internal::TimestampFromRFC3339("2019-06-21T16:52:22.9"));
  EXPECT_FALSE(internal::TimestampFromRFC3339("2019-06-21T16:52:22.Z"));
  EXPECT_FALSE(internal::TimestampFromRFC3339("2019-06-21T16:52:22ZX"));
  EXPECT_FALSE(internal::TimestampFromRFC3339("2019-06-21T16:52:-22Z"));

  EXPECT_FALSE(internal::TimestampFromRFC3339("2019-06-21T16:52:22+0:"));
  EXPECT_FALSE(internal::TimestampFromRFC3339("2019-06-21T16:52:22+:0"));
  EXPECT_FALSE(internal::TimestampFromRFC3339("2019-06-21T16:52:22+0:-0"));
  EXPECT_FALSE(internal::TimestampFromRFC3339("2019-06-21T16:52:22x00:00"));
  EXPECT_FALSE(internal::TimestampFromRFC3339("2019-06-21T16:52:22+ab:cd"));
  EXPECT_FALSE(internal::TimestampFromRFC3339("2019-06-21T16:52:22-24:60"));
  EXPECT_FALSE(internal::TimestampFromRFC3339("2019-06-21T16:52:22+00:00:00"));
}

TEST(Timestamp, FromRFC3339Limit) {
  // Verify Spanner range requirements.
  EXPECT_EQ(
      internal::TimestampFromProto(MakeProtoTimestamp(-62135596800, 0)),
      internal::TimestampFromRFC3339("0001-01-01T00:00:00.000000000Z").value());
  EXPECT_EQ(
      internal::TimestampFromProto(MakeProtoTimestamp(253402300799, 999999999)),
      internal::TimestampFromRFC3339("9999-12-31T23:59:59.999999999Z").value());

  // std::tm range limits (including -1900 tm_year bias).
  EXPECT_EQ(
      internal::TimestampFromProto(MakeProtoTimestamp(-67768040609740800, 0)),
      internal::TimestampFromRFC3339("-2147481748-01-01T00:00:00.000000000Z")
          .value());
  EXPECT_EQ(
      internal::TimestampFromProto(
          MakeProtoTimestamp(67768036191676799, 999999999)),
      internal::TimestampFromRFC3339("2147485547-12-31T23:59:59.999999999Z")
          .value());

  // One nanosecond beyond std::tm range limits fails to parse.
  EXPECT_FALSE(
      internal::TimestampFromRFC3339("-2147481749-12-31T23:59:59.999999999Z"));
  EXPECT_FALSE(
      internal::TimestampFromRFC3339("2147485548-01-01T00:00:00.000000000Z"));
}

TEST(Timestamp, ToRFC3339) {
  EXPECT_EQ("2019-06-21T16:52:22Z",
            internal::TimestampToRFC3339(internal::TimestampFromProto(
                MakeProtoTimestamp(1561135942, 0))));
  EXPECT_EQ("2019-06-21T16:52:22.000000009Z",
            internal::TimestampToRFC3339(internal::TimestampFromProto(
                MakeProtoTimestamp(1561135942, 9))));
  EXPECT_EQ("2019-06-21T16:52:22.000000089Z",
            internal::TimestampToRFC3339(internal::TimestampFromProto(
                MakeProtoTimestamp(1561135942, 89))));
  EXPECT_EQ("2019-06-21T16:52:22.000000789Z",
            internal::TimestampToRFC3339(internal::TimestampFromProto(
                MakeProtoTimestamp(1561135942, 789))));
  EXPECT_EQ("2019-06-21T16:52:22.000006789Z",
            internal::TimestampToRFC3339(internal::TimestampFromProto(
                MakeProtoTimestamp(1561135942, 6789))));
  EXPECT_EQ("2019-06-21T16:52:22.000056789Z",
            internal::TimestampToRFC3339(internal::TimestampFromProto(
                MakeProtoTimestamp(1561135942, 56789))));
  EXPECT_EQ("2019-06-21T16:52:22.000456789Z",
            internal::TimestampToRFC3339(internal::TimestampFromProto(
                MakeProtoTimestamp(1561135942, 456789))));
  EXPECT_EQ("2019-06-21T16:52:22.003456789Z",
            internal::TimestampToRFC3339(internal::TimestampFromProto(
                MakeProtoTimestamp(1561135942, 3456789))));
  EXPECT_EQ("2019-06-21T16:52:22.023456789Z",
            internal::TimestampToRFC3339(internal::TimestampFromProto(
                MakeProtoTimestamp(1561135942, 23456789))));
  EXPECT_EQ("2019-06-21T16:52:22.123456789Z",
            internal::TimestampToRFC3339(internal::TimestampFromProto(
                MakeProtoTimestamp(1561135942, 123456789))));
  EXPECT_EQ("2019-06-21T16:52:22.12345678Z",
            internal::TimestampToRFC3339(internal::TimestampFromProto(
                MakeProtoTimestamp(1561135942, 123456780))));
  EXPECT_EQ("2019-06-21T16:52:22.1234567Z",
            internal::TimestampToRFC3339(internal::TimestampFromProto(
                MakeProtoTimestamp(1561135942, 123456700))));
  EXPECT_EQ("2019-06-21T16:52:22.123456Z",
            internal::TimestampToRFC3339(internal::TimestampFromProto(
                MakeProtoTimestamp(1561135942, 123456000))));
  EXPECT_EQ("2019-06-21T16:52:22.12345Z",
            internal::TimestampToRFC3339(internal::TimestampFromProto(
                MakeProtoTimestamp(1561135942, 123450000))));
  EXPECT_EQ("2019-06-21T16:52:22.1234Z",
            internal::TimestampToRFC3339(internal::TimestampFromProto(
                MakeProtoTimestamp(1561135942, 123400000))));
  EXPECT_EQ("2019-06-21T16:52:22.123Z",
            internal::TimestampToRFC3339(internal::TimestampFromProto(
                MakeProtoTimestamp(1561135942, 123000000))));
  EXPECT_EQ("2019-06-21T16:52:22.12Z",
            internal::TimestampToRFC3339(internal::TimestampFromProto(
                MakeProtoTimestamp(1561135942, 120000000))));
  EXPECT_EQ("2019-06-21T16:52:22.1Z",
            internal::TimestampToRFC3339(internal::TimestampFromProto(
                MakeProtoTimestamp(1561135942, 100000000))));
}

TEST(Timestamp, ToRFC3339Limit) {
  // Spanner range requirements.
  EXPECT_EQ("0001-01-01T00:00:00Z",
            internal::TimestampToRFC3339(internal::TimestampFromProto(
                MakeProtoTimestamp(-62135596800, 0))));
  EXPECT_EQ("9999-12-31T23:59:59.999999999Z",
            internal::TimestampToRFC3339(internal::TimestampFromProto(
                MakeProtoTimestamp(253402300799, 999999999))));

  // std::tm range limits (including -1900 tm_year bias).
  EXPECT_EQ("-2147481748-01-01T00:00:00Z",
            internal::TimestampToRFC3339(internal::TimestampFromProto(
                MakeProtoTimestamp(-67768040609740800, 0))));
  EXPECT_EQ("2147485547-12-31T23:59:59.999999999Z",
            internal::TimestampToRFC3339(internal::TimestampFromProto(
                MakeProtoTimestamp(67768036191676799, 999999999))));

  // One nanosecond beyond std::tm range limits gives unspecified behavior.
  // Here we expect that we *do not* observe the "right" outputs.
  EXPECT_NE("-2147481749-12-31T23:59:59.999999999Z",
            internal::TimestampToRFC3339(internal::TimestampFromProto(
                MakeProtoTimestamp(-67768040609740801, 999999999))));
  EXPECT_NE("2147485548-01-01T00:00:00",
            internal::TimestampToRFC3339(internal::TimestampFromProto(
                MakeProtoTimestamp(67768036191676800, 0))));
}

TEST(Timestamp, FromProto) {
  google::protobuf::Timestamp proto = MakeProtoTimestamp(0, 0);
  EXPECT_EQ("1970-01-01T00:00:00Z",
            internal::TimestampToRFC3339(internal::TimestampFromProto(proto)));

  proto = MakeProtoTimestamp(1576030524, 611422667);
  EXPECT_EQ("2019-12-11T02:15:24.611422667Z",
            internal::TimestampToRFC3339(internal::TimestampFromProto(proto)));

  proto = MakeProtoTimestamp(-62135596800, 0);
  EXPECT_EQ("0001-01-01T00:00:00Z",
            internal::TimestampToRFC3339(internal::TimestampFromProto(proto)));

  proto = MakeProtoTimestamp(253402300799, 999999999);
  EXPECT_EQ("9999-12-31T23:59:59.999999999Z",
            internal::TimestampToRFC3339(internal::TimestampFromProto(proto)));
}

TEST(Timestamp, ToProto) {
  auto proto = internal::TimestampToProto(
      internal::TimestampFromRFC3339("1970-01-01T00:00:00.000000000Z").value());
  EXPECT_EQ(0, proto.seconds());
  EXPECT_EQ(0, proto.nanos());

  proto = internal::TimestampToProto(
      internal::TimestampFromRFC3339("2019-12-11T02:15:24.611422667Z").value());
  EXPECT_EQ(1576030524, proto.seconds());
  EXPECT_EQ(611422667, proto.nanos());

  proto = internal::TimestampToProto(
      internal::TimestampFromRFC3339("0001-01-01T00:00:00.000000000Z").value());
  EXPECT_EQ(-62135596800, proto.seconds());
  EXPECT_EQ(0, proto.nanos());

  proto = internal::TimestampToProto(
      internal::TimestampFromRFC3339("9999-12-31T23:59:59.999999999Z").value());
  EXPECT_EQ(253402300799, proto.seconds());
  EXPECT_EQ(999999999, proto.nanos());
}

TEST(Timestamp, FromChrono) {  // i.e., MakeTimestamp(sys_time<Duration>)
  // A chrono duration that can hold values beyond the range of Timestamp.
  using big_minutes = std::chrono::duration<std::int64_t, std::ratio<60>>;

  // Assumes the system_clock epoch is hour-aligned with the Unix epoch.
  auto const unix_epoch = std::chrono::time_point_cast<std::chrono::hours>(
      std::chrono::system_clock::from_time_t(0));

  auto const tp1 = unix_epoch + std::chrono::seconds(2123456789) +
                   std::chrono::nanoseconds(123456789);
  EXPECT_EQ(
      internal::TimestampFromProto(MakeProtoTimestamp(2123456789, 123456789)),
      MakeTimestamp(tp1).value());

  auto const tp2 = unix_epoch + std::chrono::seconds(2123456789) +
                   std::chrono::microseconds(123456);
  EXPECT_EQ(
      internal::TimestampFromProto(MakeProtoTimestamp(2123456789, 123456000)),
      MakeTimestamp(tp2).value());

  auto const tp3 = unix_epoch + std::chrono::seconds(2123456789) +
                   std::chrono::milliseconds(123);
  EXPECT_EQ(
      internal::TimestampFromProto(MakeProtoTimestamp(2123456789, 123000000)),
      MakeTimestamp(tp3).value());

  auto const tp4 = unix_epoch + std::chrono::minutes(2123456789);
  EXPECT_EQ(
      internal::TimestampFromProto(MakeProtoTimestamp(2123456789LL * 60, 0)),
      MakeTimestamp(tp4).value());

  auto const tp5 = unix_epoch + std::chrono::hours(2123456789);
  EXPECT_EQ(internal::TimestampFromProto(
                MakeProtoTimestamp(2123456789LL * 60 * 60, 0)),
            MakeTimestamp(tp5).value());

  auto const tp_big_pos = unix_epoch + big_minutes(153722867280912931LL);
  auto const ts_big_pos = MakeTimestamp(tp_big_pos);
  EXPECT_FALSE(ts_big_pos.ok());
  EXPECT_THAT(ts_big_pos.status().message(), HasSubstr("positive overflow"));

  auto const tp6 = unix_epoch - std::chrono::seconds(2123456789) +
                   std::chrono::nanoseconds(123456789);
  EXPECT_EQ(
      internal::TimestampFromProto(MakeProtoTimestamp(-2123456789, 123456789)),
      MakeTimestamp(tp6).value());

  auto const tp7 = unix_epoch - std::chrono::seconds(2123456789) +
                   std::chrono::microseconds(123456);
  EXPECT_EQ(
      internal::TimestampFromProto(MakeProtoTimestamp(-2123456789, 123456000)),
      MakeTimestamp(tp7).value());

  auto const tp8 = unix_epoch - std::chrono::seconds(2123456789) +
                   std::chrono::milliseconds(123);
  EXPECT_EQ(
      internal::TimestampFromProto(MakeProtoTimestamp(-2123456789, 123000000)),
      MakeTimestamp(tp8).value());

  auto const tp9 = unix_epoch - std::chrono::minutes(2123456789);
  EXPECT_EQ(
      internal::TimestampFromProto(MakeProtoTimestamp(-2123456789LL * 60, 0)),
      MakeTimestamp(tp9).value());

  auto const tp10 = unix_epoch - std::chrono::hours(2123456789);
  EXPECT_EQ(internal::TimestampFromProto(
                MakeProtoTimestamp(-2123456789LL * 60 * 60, 0)),
            MakeTimestamp(tp10).value());

  auto const tp_big_neg = unix_epoch - big_minutes(153722867280912931LL);
  auto const ts_big_neg = MakeTimestamp(tp_big_neg);
  EXPECT_FALSE(ts_big_neg.ok());
  EXPECT_THAT(ts_big_neg.status().message(), HasSubstr("negative overflow"));
}

TEST(Timestamp, ToChrono) {  // i.e., Timestamp::get<sys_time<Duration>>()
  // Assumes the system_clock epoch is hour-aligned with the Unix epoch.
  auto const epoch = std::chrono::time_point_cast<std::chrono::seconds>(
      std::chrono::system_clock::from_time_t(0));

  auto const ts_pos =
      internal::TimestampFromProto(MakeProtoTimestamp(2123456789, 123456789));

  auto const tp1 = epoch + std::chrono::seconds(2123456789) +
                   std::chrono::nanoseconds(123456789);
  EXPECT_EQ(tp1, ts_pos.get<sys_time<std::chrono::nanoseconds>>().value());

  auto const tp2 = epoch + std::chrono::seconds(2123456789) +
                   std::chrono::microseconds(123456);
  EXPECT_EQ(tp2, ts_pos.get<sys_time<std::chrono::microseconds>>().value());

  auto const tp3 =
      epoch + std::chrono::seconds(2123456789) + std::chrono::milliseconds(123);
  EXPECT_EQ(tp3, ts_pos.get<sys_time<std::chrono::milliseconds>>().value());

  auto const tp4 = epoch + std::chrono::seconds(2123456789);
  EXPECT_EQ(tp4, ts_pos.get<sys_time<std::chrono::seconds>>().value());

  auto const tp5 = epoch + std::chrono::hours(2123456789 / 60 / 60);
  EXPECT_EQ(tp5, ts_pos.get<sys_time<std::chrono::hours>>().value());

  auto const ts_big_pos =
      internal::TimestampFromProto(MakeProtoTimestamp(20000000000, 0));
  auto const tp_big_pos = ts_big_pos.get<sys_time<std::chrono::nanoseconds>>();
  EXPECT_FALSE(tp_big_pos.ok());
  EXPECT_THAT(tp_big_pos.status().message(), HasSubstr("positive overflow"));

  auto const ts_neg =
      internal::TimestampFromProto(MakeProtoTimestamp(-2123456789, 123456789));

  auto const tp6 = epoch - std::chrono::seconds(2123456789) +
                   std::chrono::nanoseconds(123456789);
  EXPECT_EQ(tp6, ts_neg.get<sys_time<std::chrono::nanoseconds>>().value());

  auto const tp7 = epoch - std::chrono::seconds(2123456789) +
                   std::chrono::microseconds(123456);
  EXPECT_EQ(tp7, ts_neg.get<sys_time<std::chrono::microseconds>>().value());

  auto const tp8 =
      epoch - std::chrono::seconds(2123456789) + std::chrono::milliseconds(123);
  EXPECT_EQ(tp8, ts_neg.get<sys_time<std::chrono::milliseconds>>().value());

  auto const tp9 = epoch - std::chrono::seconds(2123456789);
  EXPECT_EQ(tp9, ts_neg.get<sys_time<std::chrono::seconds>>().value());

  auto const tp10 = epoch - std::chrono::hours(2123456789 / 60 / 60);
  EXPECT_EQ(tp10, ts_neg.get<sys_time<std::chrono::hours>>().value());

  auto const ts_big_neg =
      internal::TimestampFromProto(MakeProtoTimestamp(-20000000000, 0));
  auto const tp_big_neg = ts_big_neg.get<sys_time<std::chrono::nanoseconds>>();
  EXPECT_FALSE(tp_big_neg.ok());
  EXPECT_THAT(tp_big_neg.status().message(), HasSubstr("negative overflow"));
}

}  // namespace
}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google
