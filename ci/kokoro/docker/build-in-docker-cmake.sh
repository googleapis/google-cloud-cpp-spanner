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
  echo "Usage: $(basename "$0") <source-directory> <binary-directory>"
  exit 1
fi

readonly SOURCE_DIR="$1"
readonly BINARY_DIR="$2"

# This script is supposed to run inside a Docker container, see
# ci/kokoro/cmake/installed-dependencies/build.sh for the expected setup.  The
# /v directory is a volume pointing to a (clean-ish) checkout of the project:
if [[ -z "${PROJECT_ROOT+x}" ]]; then
  readonly PROJECT_ROOT="/v"
fi
source "${PROJECT_ROOT}/ci/colors.sh"

echo
echo "${COLOR_YELLOW}Starting docker build $(date) with $(nproc) cores${COLOR_RESET}"
echo

echo "================================================================"
echo "Verify formatting $(date)"
(cd "${PROJECT_ROOT}" ; ./ci/check-style.sh)
echo "================================================================"

echo "================================================================"
echo "Compiling on $(date)"
echo "================================================================"
cd "${PROJECT_ROOT}"
cmake_flags=()
if [[ "${CLANG_TIDY:-}" = "yes" ]]; then
  cmake_flags+=("-DGOOGLE_CLOUD_CPP_CLANG_TIDY=yes")
fi
if [[ "${GOOGLE_CLOUD_CPP_CXX_STANDARD:-}" != "" ]]; then
  cmake_flags+=(
    "-DGOOGLE_CLOUD_CPP_CXX_STANDARD=${GOOGLE_CLOUD_CPP_CXX_STANDARD}")
fi

# We disable the shellcheck warning because we want ${CMAKE_FLAGS} to expand as
# separate arguments.
# shellcheck disable=SC2086
cmake "-DCMAKE_INSTALL_PREFIX=$HOME/staging" \
      -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
      ${CMAKE_FLAGS:-} \
      "-H${SOURCE_DIR}" "-B${BINARY_DIR}" "${cmake_flags[@]}"
cmake --build "${BINARY_DIR}" -- -j "$(nproc)"

TEST_JOB_COUNT="$(nproc)"
if [[ "${BUILD_TYPE}" == "Coverage" ]]; then
  # The code coverage build cannot run the tests in parallel. Some of
  # the files where the code coverage is recorded are shared and not
  # protected by locks of any kind.
  TEST_JOB_COUNT=1
fi
readonly TEST_JOB_COUNT

# When user a super-build the tests are hidden in a subdirectory. We can tell
# that ${BINARY_DIR} does not have the tests by checking for this file:
if [[ -r "${BINARY_DIR}/CTestTestfile.cmake" ]]; then
  echo "================================================================"
  # It is Okay to skip the tests in this case because the super build
  # automatically runs them.
  echo "Running the unit tests $(date)"
  env -C "${BINARY_DIR}" ctest \
      -LE integration-tests --output-on-failure -j "${TEST_JOB_COUNT}"
  echo "================================================================"
fi

if [[ ${RUN_INTEGRATION_TESTS} == "yes" ]]; then
  echo "================================================================"
  echo "Running the integration tests $(date)"
  echo "================================================================"
  # shellcheck disable=SC1091
  source /c/spanner-integration-tests-config.sh
  export GOOGLE_APPLICATION_CREDENTIALS=/c/spanner-credentials.json
  export GOOGLE_CLOUD_CPP_AUTO_RUN_EXAMPLES=yes

  # Run the integration tests too.
  env -C "${BINARY_DIR}" ctest \
      -L integration-tests --output-on-failure -j "${TEST_JOB_COUNT}"
  echo "================================================================"
fi

if [[ "${GENERATE_DOCS:-}" = "yes" ]]; then
  echo "================================================================"
  echo "Validate Doxygen documentation $(date)"
  cmake --build "${BINARY_DIR}" --target doxygen-docs
  echo "================================================================"
fi

if [[ "${TEST_INSTALL:-}" = "yes" ]]; then
    echo "================================================================"
    echo "Running the test install $(date)"
    echo "================================================================"
    cmake --build "${BINARY_DIR}" --target install || echo "FAILED"
fi

echo "================================================================"
echo "Build finished at $(date)"
echo "================================================================"

exit 0
