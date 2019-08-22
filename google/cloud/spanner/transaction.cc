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

#include "google/cloud/spanner/transaction.h"
#include "google/cloud/spanner/internal/time.h"
#include "google/cloud/spanner/internal/transaction_impl.h"

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {

namespace {

google::spanner::v1::TransactionOptions MakeOpts(
    google::spanner::v1::TransactionOptions_ReadOnly ro_opts) {
  google::spanner::v1::TransactionOptions opts;
  *opts.mutable_read_only() = std::move(ro_opts);
  return opts;
}

google::spanner::v1::TransactionOptions MakeOpts(
    google::spanner::v1::TransactionOptions_ReadWrite rw_opts) {
  google::spanner::v1::TransactionOptions opts;
  *opts.mutable_read_write() = std::move(rw_opts);
  return opts;
}

}  // namespace

Transaction::ReadOnlyOptions::ReadOnlyOptions() {
  ro_opts_.set_strong(true);  // only presence matters, not value
  ro_opts_.set_return_read_timestamp(true);
}

Transaction::ReadOnlyOptions::ReadOnlyOptions(Timestamp read_timestamp) {
  *ro_opts_.mutable_read_timestamp() = internal::ToProto(read_timestamp);
  ro_opts_.set_return_read_timestamp(true);
}

Transaction::ReadOnlyOptions::ReadOnlyOptions(
    std::chrono::nanoseconds exact_staleness) {
  *ro_opts_.mutable_exact_staleness() = internal::ToProto(exact_staleness);
  ro_opts_.set_return_read_timestamp(true);
}

Transaction::ReadWriteOptions::ReadWriteOptions() = default;  // currently none

Transaction::SingleUseOptions::SingleUseOptions(ReadOnlyOptions opts) {
  ro_opts_ = std::move(opts.ro_opts_);
}

Transaction::SingleUseOptions::SingleUseOptions(Timestamp min_read_timestamp) {
  *ro_opts_.mutable_min_read_timestamp() =
      internal::ToProto(min_read_timestamp);
  ro_opts_.set_return_read_timestamp(true);
}

Transaction::SingleUseOptions::SingleUseOptions(
    std::chrono::nanoseconds max_staleness) {
  *ro_opts_.mutable_max_staleness() = internal::ToProto(max_staleness);
  ro_opts_.set_return_read_timestamp(true);
}

Transaction::Transaction(ReadOnlyOptions opts) {
  google::spanner::v1::TransactionSelector selector;
  *selector.mutable_begin() = MakeOpts(std::move(opts.ro_opts_));
  impl_ = std::make_shared<internal::TransactionImpl>(std::move(selector));
}

Transaction::Transaction(ReadWriteOptions opts) {
  google::spanner::v1::TransactionSelector selector;
  *selector.mutable_begin() = MakeOpts(std::move(opts.rw_opts_));
  impl_ = std::make_shared<internal::TransactionImpl>(std::move(selector));
}

Transaction::Transaction(SingleUseOptions opts) {
  google::spanner::v1::TransactionSelector selector;
  *selector.mutable_single_use() = MakeOpts(std::move(opts.ro_opts_));
  impl_ = std::make_shared<internal::TransactionImpl>(std::move(selector));
}

Transaction::Transaction(std::string transaction_id) {
  google::spanner::v1::TransactionSelector selector;
  selector.set_id(std::move(transaction_id));
  impl_ = std::make_shared<internal::TransactionImpl>(std::move(selector));
}

Transaction::~Transaction() = default;

namespace internal {

Transaction MakeTransactionFromId(std::string transaction_id) {
  return Transaction(std::move(transaction_id));
}

}  // namespace internal
}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google
