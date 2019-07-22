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

#include "google/cloud/spanner/internal/transaction.h"
#include "google/cloud/spanner/internal/time.h"
#include "google/cloud/spanner/timestamp.h"
#include "google/cloud/spanner/transaction.h"
#include <gmock/gmock.h>
#include <chrono>
#include <ctime>
#include <future>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {

using TransactionSelector = google::spanner::v1::TransactionSelector;

class KeySet {};
class ResultSet {};

// Fake `google::cloud::spanner::Client` that supports a single `Read()`
// operation that does nothing but track the expected transaction callbacks.
class Client {
 public:
  // Set the `read_timestamp` we expect to see, and the `txn_id` we want to
  // use during the upcoming `Read()` calls.
  void Reset(Timestamp read_timestamp, std::string txn_id) {
    read_timestamp_ = read_timestamp;
    txn_id_ = std::move(txn_id);
    std::unique_lock<std::mutex> lock(mu_);
    valid_visits_ = 0;
  }

  // Return the number of valid visitations made to the transaction during a
  // completed set of `Read()` calls.
  int ValidVisits() {
    std::unique_lock<std::mutex> lock(mu_);
    return valid_visits_;
  }

  // User-visible read operation.
  ResultSet Read(Transaction txn, std::string const& table, KeySet const& keys,
                 std::vector<std::string> const& columns) {
    std::function<ResultSet(TransactionSelector&)> read =
        [this, &table, &keys, &columns](TransactionSelector& selector) {
          return this->Read(selector, table, keys, columns);
        };
    return txn->Visit(std::move(read));
  }

 private:
  ResultSet Read(TransactionSelector& selector, std::string const& table,
                 KeySet const& keys, std::vector<std::string> const& columns);

  Timestamp read_timestamp_;
  std::string txn_id_;
  std::mutex mu_;
  int valid_visits_;  // GUARDED_BY(mu_);
};

// Transaction callback.  Normally we would use the TransactionSelector
// to make a StreamingRead() RPC, and then, if the selector was a `begin`,
// switch the selector to use the allocated transaction ID.  Here we use
// the pre-assigned transaction ID after checking the read timestamp.
ResultSet Client::Read(TransactionSelector& selector, std::string const&,
                       KeySet const&, std::vector<std::string> const&) {
  if (selector.has_begin()) {
    if (selector.begin().has_read_only()) {
      if (selector.begin().read_only().has_read_timestamp()) {
        auto const& proto = selector.begin().read_only().read_timestamp();
        if (internal::FromProto(proto) == read_timestamp_) {
          std::unique_lock<std::mutex> lock(mu_);
          if (valid_visits_ == 0) ++valid_visits_;  // first visit only
        }
      }
    }
    selector.set_id(txn_id_);  // begin -> id
  } else {
    if (selector.id() == txn_id_) {
      std::unique_lock<std::mutex> lock(mu_);
      if (valid_visits_ != 0) ++valid_visits_;  // subsequent visits only
    }
  }
  return {};
}

namespace {

// Call `client->Read()` from multiple threads in the context of a single,
// read-only transaction with an exact-staleness timestamp, and return the
// number of valid visitations to that transaction (should be `n_threads`).
int MultiThreadedRead(int n_threads, Client* client, std::time_t read_time,
                      std::string const& txn_id) {
  Timestamp read_timestamp = std::chrono::time_point_cast<Timestamp::duration>(
      std::chrono::system_clock::from_time_t(read_time));
  client->Reset(read_timestamp, txn_id);

  Transaction::ReadOnlyOptions opts(read_timestamp);
  Transaction txn(opts);

  // Unused Read() parameeters.
  std::string const table{};
  KeySet const keys{};
  std::vector<std::string> const columns{};

  std::promise<void> ready_promise;
  std::shared_future<void> ready_future(ready_promise.get_future());
  auto read = [&](std::promise<void>* started) {
    started->set_value();
    ready_future.wait();                      // wait for go signal
    client->Read(txn, table, keys, columns);  // ignore ResultSet
  };

  std::vector<std::thread> threads;
  for (int i = 0; i < n_threads; ++i) {
    std::promise<void> started;
    threads.emplace_back(read, &started);
    started.get_future().wait();  // wait until thread running
  }
  ready_promise.set_value();  // go!
  for (auto& thread : threads) {
    thread.join();
  }

  return client->ValidVisits();  // should be n_threads
}

TEST(InternalTransaction, MultiThreadedRead) {
  Client client;
  EXPECT_EQ(1, MultiThreadedRead(1, &client, 1562359982, "id-0"));
  EXPECT_EQ(64, MultiThreadedRead(64, &client, 1562360571, "id-1"));
  EXPECT_EQ(128, MultiThreadedRead(128, &client, 1562361252, "id-2"));
}

}  // namespace
}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google
