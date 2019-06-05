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

#ifndef GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_TRANSACTION_H_
#define GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_TRANSACTION_H_

#include "google/cloud/spanner/version.h"
#include <memory>
#include <string>
#include <utility>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {
/**
 * Spanner transactions are represented by an instance of the
 * spanner::Transaction class. Caller's cannot directly construct Transaction
 * objects, rather Transaction objects are created by the Client class and
 * passed as arguments to caller-provided function objects that are passed to a
 * spanner::Client::RunTransaction method.
 *
 * A spanner::Client::RunTransaction method (aka a "transaction runner") takes a
 * caller-provided function object, and runs it passing in the
 * spanner::Transaction to be used. Based on the caller-provided options, either
 * a read-only or read-write transaction will be created. Read-only transactions
 * cannot fail and will not be retried. Read-write transactions may fail with
 * retryable errors and the transaction-runner method will handle creating a new
 * spanner::Transaction object (associated with the same session) and retrying
 * the caller-provided functor. It will stop when the caller-provided functor
 * says to stop (via a function return enum), or the commit succeeds.
 */
class Transaction {
 public:
  struct ReadOnlyOptions {
    // … like strong, time bounds, etc.
  };
  struct ReadWriteOptions {
    // … currently none
  };

  // Copy and move support. No default constructor.
  Transaction(Transaction&&) = default;
  Transaction& operator=(Transaction&&) = default;
  Transaction(Transaction const&) = default;
  Transaction& operator=(Transaction const&) = default;

  // Equality comparable.
  friend bool operator==(Transaction const& a, Transaction const& b);
  friend bool operator!=(Transaction const& a, Transaction const& b);

  // No other public API.
  // No accessors.
  // A Transaction is a fairly opaque handle.

 private:
  friend class Client;
  Transaction(std::string id) : id_(std::move(id)) {}

  std::string id_;
};

bool operator==(Transaction const& a, Transaction const& b) {
  return a.id_ == b.id_;
}
bool operator!=(Transaction const& a, Transaction const& b) {
  return std::rel_ops::operator!=(a, b);
}

}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_TRANSACTION_H_
