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

#ifndef GOOGLE_CLOUD_CPP_SPANNER_GOOGLE_CLOUD_SPANNER_BYTES_H_
#define GOOGLE_CLOUD_CPP_SPANNER_GOOGLE_CLOUD_SPANNER_BYTES_H_

#include "google/cloud/spanner/version.h"
#include "google/cloud/status_or.h"
#include <array>
#include <cstddef>
#include <cstring>
#include <iterator>
#include <string>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {

/**
 * A representation of the Spanner BYTES type ... variable-length binary data.
 *
 * A `Bytes` value can be constructed from, and converted to (1) a sequence of
 * octets, or (2) a base64-encoded US-ASCII `std::string`. The base64 alphabet
 * is "A-Za-z+/", with "=" padding characters. Construction from a base64
 * string can fail if the input string is unparsable.
 *
 * `Bytes` values can be compared for equality.
 *
 * All functions/operators should interpret the data as raw bytes (in contrast
 * to the Spanner STRING type, where the data should be interpreted as Unicode
 * characters).
 */
class Bytes {
 public:
  /// An empty sequence.
  Bytes() {}

  /// Construction from a sequence of octets.
  ///@{
  explicit Bytes(char const* p) : Bytes(p, p + std::strlen(p)) {}
  template <typename InputIt>
  Bytes(InputIt first, InputIt last) {
    Encoder encoder(base64_rep_);
    while (first != last) {
      encoder.buf_[encoder.len_++] = *first++;
      if (encoder.len_ == encoder.buf_.size()) encoder.Flush();
    }
    if (encoder.len_ != 0) encoder.FlushAndPad();
  }
  template <typename Container>
  explicit Bytes(Container const& c) : Bytes(c.begin(), c.end()) {}
  ///@}

  /// Conversion to a sequence of octets.
  template <typename Container>
  Container get() const {
    Decoder decoder(base64_rep_);
    return Container(decoder.begin(), decoder.end());
  }

  /// Construction from a base64-encoded US-ASCII `std::string`.
  static StatusOr<Bytes> FromBase64(std::string input);

  /// Conversion to a base64-encoded US-ASCII `std::string`.
  std::string ToBase64() const { return base64_rep_; }

  /// @name Relational operators
  ///@{
  friend bool operator==(Bytes const& a, Bytes const& b) {
    return a.base64_rep_ == b.base64_rep_;
  }
  friend bool operator!=(Bytes const& a, Bytes const& b) { return !(a == b); }
  ///@}

 private:
  struct Encoder {
    Encoder(std::string& rep) : rep_(rep), len_(0) {}
    void Flush();
    void FlushAndPad();

    std::string& rep_;  // encoded
    std::size_t len_;   // buf_[0 .. len_-1] pending encode
    std::array<unsigned char, 3> buf_;
  };

  struct Decoder {
    class iterator {
     public:
      using iterator_category = std::input_iterator_tag;
      using value_type = unsigned char;
      using difference_type = std::ptrdiff_t;
      using pointer = value_type*;
      using reference = value_type&;

      iterator(std::string::const_iterator begin,
               std::string::const_iterator end)
          : pos_(begin), end_(end), len_(0) {
        Fill();
      }

      void Fill();

      reference operator*() { return buf_[len_]; }
      pointer operator->() { return &buf_[len_]; }

      iterator& operator++() {
        if (--len_ == 0) Fill();
        return *this;
      }
      iterator operator++(int) {
        auto const old = *this;
        operator++();
        return old;
      }

      friend bool operator==(iterator const& a, iterator const& b) {
        return a.pos_ == b.pos_ && a.len_ == b.len_;
      }
      friend bool operator!=(iterator const& a, iterator const& b) {
        return !(a == b);
      }

     private:
      std::string::const_iterator pos_;  // [pos_ .. end_) pending decode
      std::string::const_iterator end_;
      std::size_t len_;  // buf_[len_ .. 1] decoded
      std::array<value_type, 1 + 3> buf_;
    };

    Decoder(std::string const& rep) : rep_(rep) {}
    iterator begin() { return iterator(rep_.begin(), rep_.end()); }
    iterator end() { return iterator(rep_.end(), rep_.end()); }

    std::string const& rep_;  // encoded
  };

  std::string base64_rep_;  // valid base64 representation
};

}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_SPANNER_GOOGLE_CLOUD_SPANNER_BYTES_H_
