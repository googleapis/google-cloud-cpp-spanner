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
#include <gmock/gmock.h>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {

// Pretend to be google::cloud::spanner::Client so that we can create a
// transaction with SingleUseOptions.
class Client {
 public:
  static Transaction MakeSingleUseTransaction(
      Transaction::SingleUseOptions opts) {
    return Transaction(std::move(opts));
  }
};

namespace {

TEST(Transaction, RegularSemantics) {
  Transaction::ReadOnlyOptions strong;
  Transaction a(strong);
  Transaction b(strong);
  EXPECT_NE(a, b);

  Transaction c = b;
  EXPECT_EQ(c, b);
  EXPECT_NE(c, a);

  Transaction d(c);
  EXPECT_EQ(d, c);
  EXPECT_EQ(d, b);

  Transaction::ReadWriteOptions none;
  Transaction e(none);
  Transaction f(none);
  EXPECT_NE(e, f);

  Transaction g = f;
  EXPECT_EQ(g, f);
  EXPECT_NE(g, e);

  Transaction h = Client::MakeSingleUseTransaction(strong);
  Transaction i = Client::MakeSingleUseTransaction(strong);
  EXPECT_NE(h, i);

  Transaction j = i;
  EXPECT_EQ(j, i);
  EXPECT_NE(j, h);
}

}  // namespace
}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google
