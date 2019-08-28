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

export BAZEL_CONFIG=""
export RUN_INTEGRATION_TESTS="no"
driver_script="ci/kokoro/macos/build-bazel.sh"

if [[ $# -eq 1 ]]; then
  export BUILD_NAME="${1}"
elif [[ -n "${KOKORO_JOB_NAME:-}" ]]; then
  # Kokoro injects the KOKORO_JOB_NAME environment variable, the value of this
  # variable is cloud-cpp/spanner/<config-file-name-without-cfg> (or more
  # generally <path/to/config-file-without-cfg>). By convention we name these
  # files `$foo.cfg` for continuous builds and `$foo-presubmit.cfg` for
  # presubmit builds. Here we extract the value of "foo" and use it as the build
  # name.
  BUILD_NAME="$(basename "${KOKORO_JOB_NAME}" "-presubmit")"
  export BUILD_NAME
else
  echo "Aborting build as the build name is not defined."
  echo "If you are invoking this script via the command line use:"
  echo "    $0 <build-name>"
  echo
  echo "If this script is invoked by Kokoro, the CI system is expected to set"
  echo "the KOKORO_JOB_NAME environment variable."
  exit 1
fi

if [[ -z "${PROJECT_ROOT+x}" ]]; then
  readonly PROJECT_ROOT="$(cd "$(dirname "$0")/../../.."; pwd)"
fi
cd "${PROJECT_ROOT}"

script_flags=("${PROJECT_ROOT}")

if [[ "${BUILD_NAME}" = "bazel" ]]; then
  driver_script="ci/kokoro/macos/build-bazel.sh"
elif [[ "${BUILD_NAME}" = "cmake-super" ]]; then
  driver_script="ci/kokoro/macos/build-cmake.sh"
  script_flags+=("ci/super" "cmake-out/macos")
else
  echo "Unknown BUILD_NAME (${BUILD_NAME})."
  exit 1
fi

RUN_INTEGRATION_TESTS=no
GOOGLE_APPLICATION_CREDENTIALS=/dev/null
if [[ -n "${KOKORO_GFILE_DIR:-}" ]] &&
   [[ -f "${KOKORO_GFILE_DIR}/spanner-integration-tests-config.sh" ]]; then
  source "${KOKORO_GFILE_DIR}/spanner-integration-tests-config.sh"
  GOOGLE_APPLICATION_CREDENTIALS="${KOKORO_GFILE_DIR}/spanner-credentials.json"
  RUN_INTEGRATION_TESTS=yes

  # Download the gRPC `roots.pem` file. On macOS gRPC does not use the native
  # trust store. One needs to set GRPC_DEFAULT_SSL_ROOTS_FILE_PATH. There was
  # a PR to fix this:
  #    https://github.com/grpc/grpc/pull/16246
  # But it was closed without being merged, and there are open bugs:
  #    https://github.com/grpc/grpc/issues/16571
  echo "    Getting roots.pem for gRPC."
  wget -q https://raw.githubusercontent.com/grpc/grpc/master/etc/roots.pem
  export GRPC_DEFAULT_SSL_ROOTS_FILE_PATH="$PWD/roots.pem"
fi
export RUN_INTEGRATION_TESTS
export GOOGLE_APPLICATION_CREDENTIALS

exec "${driver_script}" "${script_flags[@]}"
