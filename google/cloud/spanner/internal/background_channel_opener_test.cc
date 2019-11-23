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
class BackgroundChannelOpenerTest : public ::testing::Test {
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

  void StartServer() {
    server_ = builder_.BuildAndStart();
    server_thread_ = std::thread([this]() { server_->Wait(); });
  }

  void TearDown() override {
    cq_.Shutdown();
    if (cq_thread_.joinable()) {
      cq_thread_.join();
    }
    if (server_) {
      WaitForServerShutdown();
    }
  }

  void WaitForServerShutdown() {
    server_->Shutdown();
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

TEST_F(BackgroundChannelOpenerTest, NoServer) {
  // Make sure it doesn't block forever.
  BackgroundChannelOpener(cq_, channel_);
  ASSERT_EQ(GRPC_CHANNEL_IDLE, channel_->GetState(false));
}

TEST_F(BackgroundChannelOpenerTest, WithServer) {
  ASSERT_EQ(GRPC_CHANNEL_IDLE, channel_->GetState(false));
  StartServer();
  using ms = std::chrono::milliseconds;
  cq_.MakeRelativeTimer(ms(200)).then(
      [this](future<std::chrono::system_clock::time_point>) {
       BackgroundChannelOpener(cq_, channel_);
      });
  grpc_connectivity_state state = GRPC_CHANNEL_IDLE;
  int retry = 10;
  while (--retry > 0) {
    channel_->WaitForStateChange(
        state,
        std::chrono::system_clock::now() + ms(1000));
    state = channel_->GetState(false);
    if (state == GRPC_CHANNEL_READY || state == GRPC_CHANNEL_CONNECTING) {
      break;
    }
  }
  ASSERT_TRUE(state == GRPC_CHANNEL_READY || state == GRPC_CHANNEL_CONNECTING)
            << "Where state: "   << state
            << " not equal neither: " << GRPC_CHANNEL_READY
            << " nor: "               << GRPC_CHANNEL_CONNECTING << ".";
}

}  // namespace
}  // namespace internal
}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google
