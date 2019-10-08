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

#include "google/cloud/spanner/bytes.h"
#include "google/cloud/testing_util/assert_ok.h"
#include <gmock/gmock.h>
#include <cstdint>
#include <deque>
#include <limits>
#include <string>
#include <utility>
#include <vector>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {
namespace {

using ::testing::HasSubstr;

TEST(Bytes, RoundTrip) {
  char c = std::numeric_limits<char>::min();
  std::string chars(1, c);
  while (c != std::numeric_limits<char>::max()) {
    chars.push_back(++c);
  }

  // Empty string.
  std::string data;
  Bytes bytes(data);
  EXPECT_EQ("", internal::BytesToBase64(bytes));
  EXPECT_EQ(data, bytes.get<std::string>());

  // All 1-char strings.
  data.resize(1);
  for (auto c : chars) {
    data[0] = c;
    Bytes bytes(data);
    EXPECT_EQ(4, internal::BytesToBase64(bytes).size());
    EXPECT_EQ(data, bytes.get<std::string>());
  }

  // All 2-char strings.
  data.resize(2);
  for (auto c0 : chars) {
    data[0] = c0;
    for (auto c1 : chars) {
      data[1] = c1;
      Bytes bytes(data);
      EXPECT_EQ(4, internal::BytesToBase64(bytes).size());
      EXPECT_EQ(data, bytes.get<std::string>());
    }
  }

  // Some 3-char strings.
  data.resize(3);
  for (auto c0 : {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j'}) {
    data[0] = c0;
    for (auto c1 : chars) {
      data[1] = c1;
      for (auto c2 : chars) {
        data[2] = c2;
        Bytes bytes(data);
        EXPECT_EQ(4, internal::BytesToBase64(bytes).size());
        EXPECT_EQ(data, bytes.get<std::string>());
      }
    }
  }
}

TEST(Bytes, LongerRoundTrip) {
  std::vector<std::pair<std::string, std::string>> test_cases = {
      {"abcd", "YWJjZA=="},
      {"abcde", "YWJjZGU="},
      {"abcdef", "YWJjZGVm"},
      {"abcdefg", "YWJjZGVmZw=="},
      {"abcdefgh", "YWJjZGVmZ2g="},
      {"abcdefghi", "YWJjZGVmZ2hp"},
      {"abcdefghij", "YWJjZGVmZ2hpag=="},
      {"abcdefghijk", "YWJjZGVmZ2hpams="},
      {"abcdefghijkl", "YWJjZGVmZ2hpamts"},
      {"abcdefghijklm", "YWJjZGVmZ2hpamtsbQ=="},
      {"abcdefghijklmn", "YWJjZGVmZ2hpamtsbW4="},
      {"abcdefghijklmno", "YWJjZGVmZ2hpamtsbW5v"},
      {"abcdefghijklmnop", "YWJjZGVmZ2hpamtsbW5vcA=="},
      {"abcdefghijklmnopq", "YWJjZGVmZ2hpamtsbW5vcHE="},
      {"abcdefghijklmnopqr", "YWJjZGVmZ2hpamtsbW5vcHFy"},
      {"abcdefghijklmnopqrs", "YWJjZGVmZ2hpamtsbW5vcHFycw=="},
      {"abcdefghijklmnopqrst", "YWJjZGVmZ2hpamtsbW5vcHFyc3Q="},
      {"abcdefghijklmnopqrstu", "YWJjZGVmZ2hpamtsbW5vcHFyc3R1"},
      {"abcdefghijklmnopqrstuv", "YWJjZGVmZ2hpamtsbW5vcHFyc3R1dg=="},
      {"abcdefghijklmnopqrstuvw", "YWJjZGVmZ2hpamtsbW5vcHFyc3R1dnc="},
      {"abcdefghijklmnopqrstuvwx", "YWJjZGVmZ2hpamtsbW5vcHFyc3R1dnd4"},
      {"abcdefghijklmnopqrstuvwxy", "YWJjZGVmZ2hpamtsbW5vcHFyc3R1dnd4eQ=="},
      {"abcdefghijklmnopqrstuvwxyz", "YWJjZGVmZ2hpamtsbW5vcHFyc3R1dnd4eXo="},
  };
  for (auto const& test_case : test_cases) {
    Bytes bytes(test_case.first);
    EXPECT_EQ(test_case.second, internal::BytesToBase64(bytes));
    EXPECT_EQ(test_case.first, bytes.get<std::string>());
    auto decoded = internal::BytesFromBase64(test_case.second);
    EXPECT_STATUS_OK(decoded) << test_case.first;
    EXPECT_EQ(test_case.first, decoded->get<std::string>());
    EXPECT_EQ(bytes, *decoded);
  }
}

TEST(Bytes, RFC4648TestVectors) {
  // https://tools.ietf.org/html/rfc4648#section-10
  std::vector<std::pair<std::string, std::string>> test_cases = {
      {"", ""},
      {"f", "Zg=="},
      {"fo", "Zm8="},
      {"foo", "Zm9v"},
      {"foob", "Zm9vYg=="},
      {"fooba", "Zm9vYmE="},
      {"foobar", "Zm9vYmFy"},
  };
  for (auto const& test_case : test_cases) {
    Bytes bytes(test_case.first);
    EXPECT_EQ(test_case.second, internal::BytesToBase64(bytes));
    EXPECT_EQ(test_case.first, bytes.get<std::string>());
    auto decoded = internal::BytesFromBase64(test_case.second);
    EXPECT_STATUS_OK(decoded) << test_case.first;
    EXPECT_EQ(test_case.first, decoded->get<std::string>());
    EXPECT_EQ(bytes, *decoded);
  }
}

TEST(Bytes, WikiExample) {
  // https://en.wikipedia.org/wiki/Base64#Examples
  std::string const plain =
      "Man is distinguished, not only by his reason, but by this singular "
      "passion from other animals, which is a lust of the mind, that by a "
      "perseverance of delight in the continued and indefatigable generation "
      "of knowledge, exceeds the short vehemence of any carnal pleasure.";
  std::string const coded =
      "TWFuIGlzIGRpc3Rpbmd1aXNoZWQsIG5vdCBvbmx5IGJ5IGhpcyByZWFzb24sIGJ1dCBieSB0"
      "aGlzIHNpbmd1bGFyIHBhc3Npb24gZnJvbSBvdGhlciBhbmltYWxzLCB3aGljaCBpcyBhIGx1"
      "c3Qgb2YgdGhlIG1pbmQsIHRoYXQgYnkgYSBwZXJzZXZlcmFuY2Ugb2YgZGVsaWdodCBpbiB0"
      "aGUgY29udGludWVkIGFuZCBpbmRlZmF0aWdhYmxlIGdlbmVyYXRpb24gb2Yga25vd2xlZGdl"
      "LCBleGNlZWRzIHRoZSBzaG9ydCB2ZWhlbWVuY2Ugb2YgYW55IGNhcm5hbCBwbGVhc3VyZS4"
      "=";
  Bytes bytes(plain);
  EXPECT_EQ(coded, internal::BytesToBase64(bytes));
  EXPECT_EQ(plain, bytes.get<std::string>());
  auto decoded = internal::BytesFromBase64(coded);
  EXPECT_STATUS_OK(decoded) << coded;
  EXPECT_EQ(plain, decoded->get<std::string>());
  EXPECT_EQ(bytes, *decoded);
}

TEST(Bytes, FromBase64Failures) {
  // Bad lengths.
  for (std::string const base64 : {"x", "xx", "xxx"}) {
    auto decoded = internal::BytesFromBase64(base64);
    EXPECT_FALSE(decoded.ok());
    if (!decoded) {
      EXPECT_THAT(decoded.status().message(), HasSubstr("Invalid base64"));
      EXPECT_THAT(decoded.status().message(), HasSubstr("at offset 0"));
    }
  }
  for (std::string const base64 : {"xxxxx", "xxxxxx", "xxxxxxx"}) {
    auto decoded = internal::BytesFromBase64(base64);
    EXPECT_FALSE(decoded.ok());
    if (!decoded) {
      EXPECT_THAT(decoded.status().message(), HasSubstr("Invalid base64"));
      EXPECT_THAT(decoded.status().message(), HasSubstr("at offset 4"));
    }
  }

  // Chars outside base64 alphabet.
  for (std::string const base64 : {".xxx", "x.xx", "xx.x", "xxx.", "xx.="}) {
    auto decoded = internal::BytesFromBase64(base64);
    EXPECT_FALSE(decoded.ok());
    if (!decoded) {
      EXPECT_THAT(decoded.status().message(), HasSubstr("Invalid base64"));
      EXPECT_THAT(decoded.status().message(), HasSubstr("at offset 0"));
    }
  }

  // Non-zero padding bits.
  for (std::string const base64 : {"xx==", "xxx="}) {
    auto decoded = internal::BytesFromBase64(base64);
    EXPECT_FALSE(decoded.ok());
    if (!decoded) {
      EXPECT_THAT(decoded.status().message(), HasSubstr("Invalid base64"));
      EXPECT_THAT(decoded.status().message(), HasSubstr("at offset 0"));
    }
  }
}

TEST(Bytes, Conversions) {
  std::string const s_coded = "Zm9vYmFy";
  std::string const s_plain = "foobar";
  std::deque<char> const d_plain(s_plain.begin(), s_plain.end());
  std::vector<std::uint8_t> const v_plain(s_plain.begin(), s_plain.end());

  auto bytes = internal::BytesFromBase64(s_coded);
  EXPECT_STATUS_OK(bytes) << s_coded;
  EXPECT_EQ(s_coded, internal::BytesToBase64(*bytes));
  EXPECT_EQ(s_plain, bytes->get<std::string>());
  EXPECT_EQ(d_plain, bytes->get<std::deque<char>>());
  EXPECT_EQ(v_plain, bytes->get<std::vector<std::uint8_t>>());

  bytes = Bytes(s_plain);
  EXPECT_EQ(s_coded, internal::BytesToBase64(*bytes));
  EXPECT_EQ(s_plain, bytes->get<std::string>());
  EXPECT_EQ(d_plain, bytes->get<std::deque<char>>());
  EXPECT_EQ(v_plain, bytes->get<std::vector<std::uint8_t>>());

  bytes = Bytes(d_plain);
  EXPECT_EQ(s_coded, internal::BytesToBase64(*bytes));
  EXPECT_EQ(s_plain, bytes->get<std::string>());
  EXPECT_EQ(d_plain, bytes->get<std::deque<char>>());
  EXPECT_EQ(v_plain, bytes->get<std::vector<std::uint8_t>>());

  bytes = Bytes(v_plain);
  EXPECT_EQ(s_coded, internal::BytesToBase64(*bytes));
  EXPECT_EQ(s_plain, bytes->get<std::string>());
  EXPECT_EQ(d_plain, bytes->get<std::deque<char>>());
  EXPECT_EQ(v_plain, bytes->get<std::vector<std::uint8_t>>());
}

TEST(Bytes, RelationalOperators) {
  std::string const s_plain = "The quick brown fox jumps over the lazy dog.";
  std::deque<char> const d_plain(s_plain.begin(), s_plain.end());
  std::vector<std::uint8_t> const v_plain(s_plain.begin(), s_plain.end());

  auto s_bytes = Bytes(s_plain.begin(), s_plain.end());
  auto d_bytes = Bytes(d_plain.begin(), d_plain.end());
  auto v_bytes = Bytes(v_plain.begin(), v_plain.end());
  EXPECT_EQ(s_bytes, d_bytes);
  EXPECT_EQ(d_bytes, v_bytes);
  EXPECT_EQ(v_bytes, s_bytes);

  auto x_bytes = Bytes(s_plain + " How vexingly quick daft zebras jump!");
  EXPECT_NE(x_bytes, s_bytes);
  EXPECT_NE(x_bytes, d_bytes);
  EXPECT_NE(x_bytes, v_bytes);
}

}  // namespace
}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google
