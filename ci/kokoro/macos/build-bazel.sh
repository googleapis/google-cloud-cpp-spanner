#!/usr/bin/env bash
#
# Copyright 2019 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set -eu

if [[ $# -ne 1 ]]; then
  echo "Usage: $(basename "$0") <project-root>"
  exit 1
fi

readonly PROJECT_ROOT="$1"

echo
echo "================================================================"
echo "Update or Install Bazel at $(date)."
echo

# macOS does not have sha256sum by default, but `shasum -a 256` does the same
# thing:
function sha256sum() { shasum -a 256 "$@" ; } && export -f sha256sum

"${PROJECT_ROOT}/ci/install-bazel.sh"

echo
echo "================================================================"
readonly BAZEL_BIN="$HOME/bin/bazel"
echo "Using Bazel in ${BAZEL_BIN}"

bazel_args=(
    # On macOS gRPC does not compile correctly unless one defines this:
    "--copt=-DGRPC_BAZEL_BUILD"
    # We need this environment variable because on macOS gRPC crashes if it
    # cannot find the credentials, even if you do not use them. Some of the
    # unit tests do exactly that.
    "--action_env=GOOGLE_APPLICATION_CREDENTIALS=${GOOGLE_APPLICATION_CREDENTIALS}"
    "--test_output=errors"
    "--verbose_failures=true"
    "--keep_going")
if [[ -n "${BAZEL_CONFIG}" ]]; then
    bazel_args+=("--config" "${BAZEL_CONFIG}")
fi

echo "================================================================"
echo "Fetching dependencies $(date)"
"${PROJECT_ROOT}/ci/retry-command.sh" \
    "${BAZEL_BIN}" fetch -- //google/cloud/...:all

echo "================================================================"
echo "Build and run unit tests at $(date)."
"${BAZEL_BIN}" test \
    "${bazel_args[@]}" "--test_tag_filters=-integration-tests" \
    -- //google/cloud/...:all

echo
echo "================================================================"
echo "Build and all targets at $(date)."
"${BAZEL_BIN}" build \
    "${bazel_args[@]}" -- //google/cloud/...:all

if [[ ${RUN_INTEGRATION_TESTS} == "yes" ]]; then
  echo "================================================================"
  echo "Running the integration tests $(date)"
  echo "================================================================"

  # Run the integration tests using Bazel to drive them.
  "${BAZEL_BIN}" test \
      "${bazel_args[@]}" \
      "--spawn_strategy=local" \
      "--test_env=GOOGLE_APPLICATION_CREDENTIALS=${GOOGLE_APPLICATION_CREDENTIALS}" \
      "--test_env=RUN_SLOW_INTEGRATION_TESTS=${RUN_SLOW_INTEGRATION_TESTS}" \
      "--test_env=GRPC_DEFAULT_SSL_ROOTS_FILE_PATH=${GRPC_DEFAULT_SSL_ROOTS_FILE_PATH}" \
      "--test_env=GOOGLE_CLOUD_PROJECT=${GOOGLE_CLOUD_PROJECT}" \
      "--test_env=GOOGLE_CLOUD_CPP_SPANNER_INSTANCE=${GOOGLE_CLOUD_CPP_SPANNER_INSTANCE}" \
      "--test_env=GOOGLE_CLOUD_CPP_SPANNER_IAM_TEST_SA=${GOOGLE_CLOUD_CPP_SPANNER_IAM_TEST_SA}" \
      "--test_env=GOOGLE_CLOUD_CPP_AUTO_RUN_EXAMPLES=yes" \
      "--test_tag_filters=integration-tests" \
      -- //google/cloud/...:all
fi
