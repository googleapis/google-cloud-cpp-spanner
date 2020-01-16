#!/usr/bin/env bash
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

if [[ $# != 2 ]]; then
  # The arguments are ignored, but required for compatibility with
  # build-in-docker-cmake.sh
  echo "Usage: $(basename "$0") <source-directory> <binary-directory>"
  exit 1
fi

readonly SOURCE_DIR="$1"
readonly BINARY_DIR="$2"

# This script is supposed to run inside a Docker container, see
# ci/kokoro/build.sh for the expected setup.  The /v directory is a volume
# pointing to a (clean-ish) checkout of google-cloud-cpp:
if [[ -z "${PROJECT_ROOT+x}" ]]; then
  readonly PROJECT_ROOT="/v"
fi
source "${PROJECT_ROOT}/ci/colors.sh"

# Run the "bazel build"/"bazel test" cycle inside a Docker image.
# This script is designed to work in the context created by the
# ci/Dockerfile.* build scripts.

echo
echo "${COLOR_YELLOW}Starting docker build $(date) with ${NCPU} cores${COLOR_RESET}"
echo

echo "================================================================"
echo "Verify formatting $(date)"
(cd "${PROJECT_ROOT}" ; ./ci/check-style.sh)
echo "================================================================"

readonly BAZEL_BIN="/usr/local/bin/bazel"
echo "Using Bazel in ${BAZEL_BIN}"

bazel_args=("--test_output=errors" "--verbose_failures=true" "--keep_going")
if [[ -n "${BAZEL_CONFIG}" ]]; then
    bazel_args+=(--config "${BAZEL_CONFIG}")
fi

echo "================================================================"
echo "Fetching dependencies $(date)"
echo "================================================================"
"${PROJECT_ROOT}/ci/retry-command.sh" \
    "${BAZEL_BIN}" fetch -- //google/cloud/...:all

echo "================================================================"
echo "Compiling and running unit tests $(date)"
echo "================================================================"
# Note that we exclude the integration tests from this run.
"${BAZEL_BIN}" test \
    "${bazel_args[@]}" "--test_tag_filters=-integration-tests" \
    -- //google/cloud/...:all

echo "================================================================"
echo "Compiling all the code, including integration tests $(date)"
echo "================================================================"
# Then build everything else (integration tests, examples, etc). So we can run
# them next.
"${BAZEL_BIN}" build \
    "${bazel_args[@]}" \
    -- //google/cloud/...:all

readonly INTEGRATION_TESTS_CONFIG="/c/spanner-integration-tests-config.sh"
# yes: always try to run integration tests
# auto: only try to run integration tests if the config file is executable.
if [[ "${RUN_INTEGRATION_TESTS}" == "yes" || \
      ( "${RUN_INTEGRATION_TESTS}" == "auto" && \
        -r "${INTEGRATION_TESTS_CONFIG}" ) ]]; then
  echo "================================================================"
  echo "Running the integration tests $(date)"
  echo "================================================================"
  # shellcheck disable=SC1091
  source "${INTEGRATION_TESTS_CONFIG}"

  # Run the integration tests using Bazel to drive them.
  "${BAZEL_BIN}" test \
      "${bazel_args[@]}" \
      "--spawn_strategy=local" \
      "--test_env=GOOGLE_APPLICATION_CREDENTIALS=/c/spanner-credentials.json" \
      "--test_env=GOOGLE_CLOUD_PROJECT=${GOOGLE_CLOUD_PROJECT}" \
      "--test_env=GOOGLE_CLOUD_CPP_SPANNER_INSTANCE=${GOOGLE_CLOUD_CPP_SPANNER_INSTANCE}" \
      "--test_env=GOOGLE_CLOUD_CPP_SPANNER_IAM_TEST_SA=${GOOGLE_CLOUD_CPP_SPANNER_IAM_TEST_SA}" \
      "--test_env=GOOGLE_CLOUD_CPP_AUTO_RUN_EXAMPLES=yes" \
      "--test_tag_filters=integration-tests" \
      -- //google/cloud/...:all
fi

echo "================================================================"
echo "Build finished at $(date)"
echo "================================================================"

exit 0
