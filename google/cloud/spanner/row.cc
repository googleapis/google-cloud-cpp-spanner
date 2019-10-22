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

#include "google/cloud/spanner/row.h"
#include "google/cloud/log.h"
#include "google/cloud/status.h"
#include "google/cloud/status_or.h"
#include <algorithm>
#include <cassert>
#include <iterator>
#include <utility>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {

namespace internal {
Row MakeRow(std::vector<Value> vector,
            std::shared_ptr<const std::vector<std::string>> columns) {
  return Row(std::move(vector), std::move(columns));
}
}  // namespace internal

Row MakeRow(std::vector<std::pair<std::string, Value>> pairs) {
  auto values = std::vector<Value>{};
  auto columns = std::make_shared<std::vector<std::string>>();
  for (auto& p : pairs) {
    values.emplace_back(std::move(p.second));
    columns->emplace_back(std::move(p.first));
  }
  return internal::MakeRow(std::move(values), std::move(columns));
}

Row::Row() : Row({}, std::make_shared<std::vector<std::string>>()) {}

Row::Row(std::vector<Value> values,
         std::shared_ptr<const std::vector<std::string>> columns)
    : values_(std::move(values)), columns_(std::move(columns)) {
  if (values_.size() != columns_->size()) {
    GCP_LOG(FATAL) << "Row's value and column sizes do not match: "
                   << values_.size() << " vs " << columns_->size();
  }
}

// NOLINTNEXTLINE(readability-identifier-naming)
StatusOr<Value> Row::get(std::size_t pos) const {
  if (pos < values_.size()) return values_[pos];
  return Status(StatusCode::kInvalidArgument, "position out of range");
}

// NOLINTNEXTLINE(readability-identifier-naming)
StatusOr<Value> Row::get(std::string const& name) const {
  auto it = std::find(columns_->begin(), columns_->end(), name);
  if (it != columns_->end()) return get(std::distance(columns_->begin(), it));
  return Status(StatusCode::kInvalidArgument, "column name not found");
}

bool operator==(Row const& a, Row const& b) {
  return a.values_ == b.values_ && *a.columns_ == *b.columns_;
}

}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google
