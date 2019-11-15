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

#include "google/cloud/spanner/internal/session_pool.h"
#include "google/cloud/spanner/internal/connection_impl.h"
#include "google/cloud/spanner/internal/retry_loop.h"
#include "google/cloud/spanner/internal/session.h"
#include "google/cloud/internal/make_unique.h"
#include "google/cloud/status.h"
#include <algorithm>
#include <random>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {
namespace internal {

namespace spanner_proto = ::google::spanner::v1;

namespace {

// Ensure the options have sensible values.
SessionPoolOptions SanitizeOptions(SessionPoolOptions options) {
  options.min_sessions = (std::max)(options.min_sessions, 0);
  options.max_sessions = (std::max)(options.max_sessions, 1);
  if (options.max_sessions < options.min_sessions) {
    options.max_sessions = options.min_sessions;
  }
  options.max_idle_sessions = (std::max)(options.max_idle_sessions, 0);
  options.write_sessions_fraction =
      (std::max)(options.write_sessions_fraction, 0.0);
  options.write_sessions_fraction =
      (std::min)(options.write_sessions_fraction, 1.0);
  return options;
}

}  // namespace

SessionPool::SessionPool(Database db,
                         std::vector<std::shared_ptr<SpannerStub>> stubs,
                         std::unique_ptr<RetryPolicy> retry_policy,
                         std::unique_ptr<BackoffPolicy> backoff_policy,
                         SessionPoolOptions options)
    : db_(std::move(db)),
      retry_policy_prototype_(std::move(retry_policy)),
      backoff_policy_prototype_(std::move(backoff_policy)),
      options_(SanitizeOptions(options)) {
  channels_.reserve(stubs.size());
  for (auto& stub : stubs) {
    channels_.emplace_back(std::move(stub));
  }
  least_loaded_channel_ = &channels_[0];

  if (options_.min_sessions == 0) {
    return;
  }

  // Eagerly initialize the pool with `min_sessions` sessions.
  std::unique_lock<std::mutex> lk(mu_);
  int num_channels = static_cast<int>(channels_.size());
  int sessions_per_channel = options_.min_sessions / num_channels;
  // if it doesn't divide evenly, add the remainder to the first channel.
  int extra_sessions = options_.min_sessions % num_channels;
  for (auto& channel : channels_) {
    // Just ignore failures; we'll try again when the caller requests a
    // session, and we'll be in a position to return an error at that time.
    (void)CreateSessions(lk, channel, sessions_per_channel + extra_sessions);
    extra_sessions = 0;
  }
  // Shuffle the pool so we distribute returned sessions across channels.
  std::shuffle(sessions_.begin(), sessions_.end(),
               std::mt19937(std::random_device()()));
}

StatusOr<SessionHolder> SessionPool::Allocate(bool dissociate_from_pool) {
  std::unique_lock<std::mutex> lk(mu_);
  for (;;) {
    if (!sessions_.empty()) {
      // return the most recently used session.
      auto session = std::move(sessions_.back());
      sessions_.pop_back();
      if (dissociate_from_pool) {
        --total_sessions_;
      }
      return {MakeSessionHolder(std::move(session), dissociate_from_pool)};
    }

    // If the pool is at its max size, fail or wait until someone returns a
    // session to the pool then try again.
    if (total_sessions_ >= options_.max_sessions) {
      if (options_.action_on_exhaustion == ActionOnExhaustion::FAIL) {
        return Status(StatusCode::kResourceExhausted, "session pool exhausted");
      }
      cond_.wait(lk, [this] {
        return !sessions_.empty() || total_sessions_ < options_.max_sessions;
      });
      continue;
    }

    // Create new sessions for the pool.
    //
    // TODO(#307) Currently we only allow one thread to do this at a time; a
    // possible enhancement is tracking the number of waiters and issuing more
    // simulaneous calls if additional sessions are needed. We can also use the
    // number of waiters in the `sessions_to_create` calculation below.
    if (create_in_progress_) {
      cond_.wait(lk,
                 [this] { return !sessions_.empty() || !create_in_progress_; });
      continue;
    }

    // Add `min_sessions` to the pool (plus the one we're going to return),
    // subject to the `max_sessions` cap.
    int sessions_to_create = (std::min)(
        options_.min_sessions + 1, options_.max_sessions - total_sessions_);
    ChannelInfo& channel = *least_loaded_channel_;
    auto create_status = CreateSessions(lk, channel, sessions_to_create);
    if (!create_status.ok()) {
      return create_status;
    }
    // Wake up everyone that was waiting for a session.
    cond_.notify_all();
  }
}

std::shared_ptr<SpannerStub> SessionPool::GetStub(Session const& session) {
  std::shared_ptr<SpannerStub> stub = session.stub();
  if (stub) return stub;

  // Sessions that were created for partitioned Reads/Queries do not have
  // their own stub, so return one to use.
  return least_loaded_channel_->stub;
}

void SessionPool::Release(Session* session) {
  std::unique_lock<std::mutex> lk(mu_);
  bool notify = sessions_.empty();
  sessions_.emplace_back(session);
  // If sessions_ was empty, wake up someone who was waiting for a session.
  if (notify) {
    lk.unlock();
    cond_.notify_one();
  }
}

// Creates `num_sessions` on `channel` and adds them to the pool.
//
// Requires `lk` has locked `mu_` prior to this call. `lk` will be dropped
// while the RPC is in progress and then reacquired.
Status SessionPool::CreateSessions(std::unique_lock<std::mutex>& lk,
                                   ChannelInfo& channel, int num_sessions) {
  spanner_proto::BatchCreateSessionsRequest request;
  request.set_database(db_.FullName());
  request.set_session_count(std::int32_t{num_sessions});
  const auto& stub = channel.stub;
  create_in_progress_ = true;
  lk.unlock();
  auto response = RetryLoop(
      retry_policy_prototype_->clone(), backoff_policy_prototype_->clone(),
      true,
      [&stub](grpc::ClientContext& context,
              spanner_proto::BatchCreateSessionsRequest const& request) {
        return stub->BatchCreateSessions(context, request);
      },
      request, __func__);
  lk.lock();
  create_in_progress_ = false;
  if (!response.ok()) {
    return response.status();
  }
  std::vector<std::unique_ptr<Session>> sessions;
  sessions.reserve(response->session_size());
  for (auto& session : *response->mutable_session()) {
    sessions.push_back(google::cloud::internal::make_unique<Session>(
        std::move(*session.mutable_name()), stub));
  }
  AddSessionsToPool(channel, std::move(sessions));
  return Status();
}

// adds `sessions` to the pool and updates counters for `channel` and the pool.
void SessionPool::AddSessionsToPool(
    ChannelInfo& channel, std::vector<std::unique_ptr<Session>> sessions) {
  int sessions_created = static_cast<int>(sessions.size());
  channel.session_count += sessions_created;
  total_sessions_ += sessions_created;
  // TODO(#307) instead of adding all of these to the end, we could insert
  // them in random locations to prevent "bunching" of sessions on the same
  // channel. Currently we do this for the initial allocation only.
  sessions_.insert(sessions_.end(), std::make_move_iterator(sessions.begin()),
                   std::make_move_iterator(sessions.end()));
  UpdateLeastLoadedChannel();
}

void SessionPool::UpdateLeastLoadedChannel() {
  least_loaded_channel_ = &channels_[0];
  for (auto& channel : channels_) {
    if (channel.session_count < least_loaded_channel_->session_count) {
      least_loaded_channel_ = &channel;
    }
  }
}

SessionHolder SessionPool::MakeSessionHolder(std::unique_ptr<Session> session,
                                             bool dissociate_from_pool) {
  if (dissociate_from_pool) {
    // Uses the default deleter; the `Session` is not returned to the pool.
    return {std::move(session)};
  }
  std::weak_ptr<SessionPool> pool = shared_from_this();
  return SessionHolder(session.release(), [pool](Session* session) {
    auto shared_pool = pool.lock();
    // If `pool` is still alive, release the `Session` to it; otherwise just
    // delete the `Session`.
    if (shared_pool) {
      shared_pool->Release(session);
    } else {
      delete session;
    }
  });
}

}  // namespace internal
}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google
