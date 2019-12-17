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

if [[ -z "${PROJECT_ROOT+x}" ]]; then
  readonly PROJECT_ROOT="$(cd "$(dirname "$0")/../../.."; pwd)"
fi
source "${PROJECT_ROOT}/ci/kokoro/define-docker-variables.sh"

# If w3m is not installed there is nothing to do.
if ! type w3m >/dev/null 2>&1; then
  exit 0
fi

function dump_report() {
  local filename="$1"

  echo
  echo "================ ${filename} ================"
  w3m -dump "${filename}"
}

export -f dump_report

# Find any analysis reports, currently ABI checks and Clang static analysis are
# the two things that produce them. Note that the Clang static analysis reports
# are copied into the scan-cmake-out directory by the build-docker.sh script.

find "${BUILD_OUTPUT}" -name 'src_compat_report.html' \
    -exec bash -c 'dump_report "$1"' _ {} \; 2>/dev/null || \
  echo "No ABI compatibility reports found."

find scan-cmake-out/ -name '*.html' \
    -exec bash -c 'dump_report "$1"' _ {} \; 2>/dev/null || \
  echo "No static analysis reports found."
