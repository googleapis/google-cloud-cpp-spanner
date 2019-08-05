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

if [[ $# -ne 3 ]]; then
  echo "Usage: $(basename "$0") <project-root> <source-directory> <binary-directory>"
  exit 1
fi

readonly PROJECT_ROOT="$1"
readonly SOURCE_DIR="$2"
readonly BINARY_DIR="$3"

NCPU="$(sysctl -n hw.logicalcpu)"
readonly NCPU

source "${PROJECT_ROOT}/ci/colors.sh"

echo "================================================================"
echo "Compiling on $(date) with ${NCPU} cpus"
echo "================================================================"
cd "${PROJECT_ROOT}"
cmake_flags=("-DCMAKE_INSTALL_PREFIX=$HOME/staging")

cmake "-H${SOURCE_DIR}" "-B${BINARY_DIR}" "${cmake_flags[@]}"
cmake --build "${BINARY_DIR}" -- -j "${NCPU}"

# When user a super-build the tests are hidden in a subdirectory. We can tell
# that ${BINARY_DIR} does not have the tests by checking for this file:
if [[ -r "${BINARY_DIR}/CTestTestfile.cmake" ]]; then
  echo "================================================================"
  # It is Okay to skip the tests in this case because the super build
  # automatically runs them.
  echo "Running the unit tests $(date)"
  env -C "${BINARY_DIR}" ctest \
      -LE integration-tests \
      --output-on-failure -j "${NCPU}"
  echo "================================================================"
fi

if [[ ${RUN_INTEGRATION_TESTS} == "yes" ]]; then
  echo "================================================================"
  echo "Running the integration tests $(date)"
  echo "================================================================"
  source "${KOKORO_GFILE_DIR}/spanner-integration-tests-config.sh"
  export GOOGLE_APPLICATION_CREDENTIALS="${KOKORO_GFILE_DIR}/spanner-credentials.json"
  export GOOGLE_CLOUD_CPP_AUTO_RUN_EXAMPLES=yes

  # Run the integration tests too.
  env -C "${BINARY_DIR}" ctest \
      -L integration-tests \
      --output-on-failure -j "${NCPU}"
  echo "================================================================"
fi

echo "================================================================"
echo "Build finished at $(date)"
echo "================================================================"

exit 0
