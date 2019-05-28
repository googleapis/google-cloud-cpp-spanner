#ifndef GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_TRANSACTION_H_
#define GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_TRANSACTION_H_

#include "google/cloud/spanner/version.h"
#include "google/cloud/status_or.h"
#include <memory>
#include <string>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {

class Transaction {
 public:
  struct ReadOnlyOptions {
    // ...
  };
  struct ReadWriteOptions {
    // ...
  };
  // Value type.
  // No default c'tor, but defaulted copy, assign, move, etc.

  friend bool operator==(Transaction const& a, Transaction const& b) {
    return a.impl_ == b.impl_;
  }
  friend bool operator!=(Transaction const& a, Transaction const& b) {
    return !(a == b);
  }

 private:
  friend class Client;
  friend google::cloud::StatusOr<Transaction> DeserializeTransaction(
      std::string);
  friend std::string SerializeTransaction(Transaction);
  friend Transaction MakeReadOnlyTransaction(Transaction::ReadOnlyOptions);
  friend Transaction MakeReadWriteTransaction(Transaction::ReadWriteOptions);

  struct PartitionedDml {};
  explicit Transaction(PartitionedDml) {}
  static Transaction MakePartitionedDmlTransaction() {
    return Transaction(PartitionedDml{});
  }

  struct SingleUse {};
  explicit Transaction(SingleUse){};
  static Transaction MakeSingleUseTransaction() {
    return Transaction(SingleUse{});
  }

  explicit Transaction(ReadOnlyOptions opts) {}
  explicit Transaction(ReadWriteOptions opts) {}

  class Impl;
  std::shared_ptr<Impl> impl_;
};

Transaction MakeReadOnlyTransaction(Transaction::ReadOnlyOptions opts = {}) {
  return Transaction(opts);
}

Transaction MakeReadWriteTransaction(Transaction::ReadWriteOptions opts = {}) {
  return Transaction(opts);
}

std::string SerializeTransaction(Transaction tx) {
  // TODO: Use proto or something better to serialize
  return {};
}

google::cloud::StatusOr<Transaction> DeserializeTransaction(std::string s) {
  // TODO: Properly deserialize.
  return Transaction(Transaction::ReadOnlyOptions{});
}

}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_SPANNER_TRANSACTION_H_
