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

readonly BAZEL_BIN="/usr/local/bin/bazel"
echo "Using Bazel in ${BAZEL_BIN}"

echo "================================================================"
echo "Compile a project that depends on google-cloud-cpp-spanner $(date)"
echo "================================================================"
bazel_args=("--test_output=errors" "--verbose_failures=true" "--keep_going")
if [[ -n "${BAZEL_CONFIG}" ]]; then
    bazel_args+=(--config "${BAZEL_CONFIG}")
fi

cd ci/quickstart

echo "================================================================"
echo "Fetching dependencies $(date)"
echo "================================================================"
"${PROJECT_ROOT}/ci/retry-command.sh" \
    "${BAZEL_BIN}" fetch -- //...:all

echo "================================================================"
echo "Building project $(date)"
echo "================================================================"
"${BAZEL_BIN}" build  "${bazel_args[@]}" \
    -- //...:all

if [[ -f "${CONFIG_DIRECTORY}/spanner-integration-tests-config.sh" ]]; then
  echo "================================================================"
  echo "Testing a program built with spanner-as-a-dependency $(date)"
  echo "================================================================"
  source "${CONFIG_DIRECTORY}/spanner-integration-tests-config.sh"

  # Pick one of the instances at random
  mapfile -t INSTANCES < <(gcloud "--project=${GOOGLE_CLOUD_PROJECT}" \
                  spanner instances list --filter=NAME:test-instance --format='csv(name)[no-heading]')
  readonly INSTANCES
  GOOGLE_CLOUD_CPP_SPANNER_INSTANCE="${INSTANCES[$(( RANDOM % ${#INSTANCES} ))]}"
  if ! gcloud "--project=${GOOGLE_CLOUD_PROJECT}" \
           spanner databases list "--instance=${GOOGLE_CLOUD_CPP_SPANNER_INSTANCE}" | grep -q quickstart-db; then
    echo "Quickstart database (quickstart-db) already exists"
  else
    echo "Creating quickstart-db database"
    gcloud "--project=${GOOGLE_CLOUD_PROJECT}" \
        spanner databases create "--instance=${GOOGLE_CLOUD_CPP_SPANNER_INSTANCE}" quickstart-db
  fi

  # Run the program using the command-line parameters.
  "${BAZEL_BIN}" run \
      "${bazel_args[@]}" \
      "--spawn_strategy=local" \
      "--action_env=GOOGLE_APPLICATION_CREDENTIALS=/c/spanner-credentials.json" \
      :quickstart -- "${GOOGLE_CLOUD_PROJECT}" "${GOOGLE_CLOUD_CPP_SPANNER_INSTANCE}" quickstart-db

fi

echo "================================================================"
echo "Build finished at $(date)"
echo "================================================================"

exit 0
