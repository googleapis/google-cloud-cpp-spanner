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

#ifndef GOOGLE_CLOUD_CPP_SPANNER_GOOGLE_CLOUD_SPANNER_INTERNAL_SESSION_HOLDER_H_
#define GOOGLE_CLOUD_CPP_SPANNER_GOOGLE_CLOUD_SPANNER_INTERNAL_SESSION_HOLDER_H_

#include "google/cloud/spanner/internal/connection_impl.h"
#include "google/cloud/spanner/version.h"
#include <string>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {
namespace internal {

/**
 * A RAII class that releases a session to the pool when destroyed.
 */
class SessionHolder {
 public:
  SessionHolder(std::string session, ConnectionImpl* conn) noexcept
      : session_(std::move(session)), conn_(conn) {}

  // This class is move-only because we want only one destructor returning the
  // session back to `conn_`.
  SessionHolder(SessionHolder const&) = delete;
  SessionHolder& operator=(SessionHolder const&) = delete;

  // Need explicit move constructor and assignment to clear the `conn_` from
  // the source.
  SessionHolder(SessionHolder&& rhs) noexcept
      : session_(std::move(rhs.session_)), conn_(rhs.conn_) {
    rhs.conn_ = nullptr;
  }

  SessionHolder& operator=(SessionHolder&& rhs) noexcept {
    session_ = std::move(rhs.session_);
    conn_ = rhs.conn_;
    rhs.conn_ = nullptr;
    return *this;
  }

  ~SessionHolder() {
    if (conn_) {
      conn_->ReleaseSession(std::move(session_));
    }
  }

  std::string const& session_name() const { return session_; }

 private:
  std::string session_;
  ConnectionImpl* conn_;
};

}  // namespace internal
}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_SPANNER_GOOGLE_CLOUD_SPANNER_INTERNAL_SESSION_HOLDER_H_
