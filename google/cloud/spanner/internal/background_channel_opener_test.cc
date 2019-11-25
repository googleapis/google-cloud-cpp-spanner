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

#include "google/cloud/spanner/internal/background_channel_opener.h"
#include "google/spanner/v1/spanner.grpc.pb.h"
#include <gmock/gmock.h>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {
namespace internal {
namespace {

namespace gsv1 = google::spanner::v1;
/**
 * Dummy service
 */
class DummySpanner final : public gsv1::Spanner::Service {
 public:
  DummySpanner() = default;
  grpc::Status CreateSession(grpc::ServerContext*,
                             const gsv1::CreateSessionRequest*,
                             gsv1::Session*) override {
    return grpc::Status::OK;
  }
};

/**
 * This test prepares the server (not starting it immediately), and the client
 * side grpc::Channel that connecs to the same port.
 */
class BackgroundChannelOpenerTestWithoutServer : public ::testing::Test {
 protected:
  void SetUp() override {
    int port;
    std::string server_address("[::]:0");
    builder_.AddListeningPort(server_address, grpc::InsecureServerCredentials(),
                              &port);
    builder_.RegisterService(&impl_);
    channel_ = grpc::CreateChannel("localhost:" + std::to_string(port),
                                   grpc::InsecureChannelCredentials());
    cq_thread_ = std::thread([this] { cq_.Run(); });
  }

  void TearDown() override {
    cq_.Shutdown();
    if (cq_thread_.joinable()) {
      cq_thread_.join();
    }
    WaitForServerShutdown();
  }

  void WaitForServerShutdown() {
    if (server_) {
      server_->Shutdown();
    }
    if (server_thread_.joinable()) {
      server_thread_.join();
    }
  }

  DummySpanner impl_;
  grpc::ServerBuilder builder_;
  std::unique_ptr<grpc::Server> server_;
  std::shared_ptr<grpc::Channel> channel_;
  std::thread server_thread_;

  grpc_utils::CompletionQueue cq_;
  std::thread cq_thread_;
};

/**
 * This test prepares the server and the client side grpc::Channel that connecs
 * to the same port.
 */
class BackgroundChannelOpenerTest
    : public BackgroundChannelOpenerTestWithoutServer {
 protected:
  void SetUp() override {
    int port;
    std::string server_address("[::]:0");
    builder_.AddListeningPort(server_address, grpc::InsecureServerCredentials(),
                              &port);
    builder_.RegisterService(&impl_);
    server_ = builder_.BuildAndStart();
    server_thread_ = std::thread([this]() { server_->Wait(); });
    channel_ = grpc::CreateChannel("localhost:" + std::to_string(port),
                                   grpc::InsecureChannelCredentials());
    cq_thread_ = std::thread([this] { cq_.Run(); });
  }
};

TEST_F(BackgroundChannelOpenerTestWithoutServer, NoServer) {
  // Make sure it doesn't block forever.
  BackgroundChannelOpener(cq_, channel_);
}

TEST_F(BackgroundChannelOpenerTest, WithServer) {
  BackgroundChannelOpener(cq_, channel_);
  grpc_connectivity_state state = GRPC_CHANNEL_IDLE;
  int retry = 10;
  while (--retry > 0) {
    state = channel_->GetState(false);
    if (state == GRPC_CHANNEL_READY || state == GRPC_CHANNEL_CONNECTING) {
      break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }
  EXPECT_TRUE(state == GRPC_CHANNEL_READY || state == GRPC_CHANNEL_CONNECTING)
      << "Where state: " << state
      << " not equal neither: " << GRPC_CHANNEL_READY
      << " nor: " << GRPC_CHANNEL_CONNECTING << ".";
}

}  // namespace
}  // namespace internal
}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google
