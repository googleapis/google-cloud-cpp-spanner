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

# This script is supposed to run inside a Docker container, see
# ci/kokoro/cmake/installed-dependencies/build.sh for the expected setup.  The
# /v directory is a volume pointing to a (clean-ish) checkout of the project:
if [[ -z "${PROJECT_ROOT+x}" ]]; then
  readonly PROJECT_ROOT="/v"
fi
source "${PROJECT_ROOT}/ci/kokoro/linux-config.sh"
source "${PROJECT_ROOT}/ci/colors.sh"

echo
echo "${COLOR_YELLOW}Starting docker build $(date) with $(nproc) cores${COLOR_RESET}"
echo

echo "================================================================"
echo "Compiling on $(date)"
echo "================================================================"
cd "${PROJECT_ROOT}"
cmake -H. -B"${BUILD_OUTPUT}"
cmake --build "${BUILD_OUTPUT}" -- -j "$(nproc)"

echo "================================================================"
echo "Running the unit tests $(date)"
echo "================================================================"
env -C "${BUILD_OUTPUT}" ctest --output-on-failure -j "$(nproc)"

echo "================================================================"
echo "Build finished at $(date)"
echo "================================================================"

exit 0
