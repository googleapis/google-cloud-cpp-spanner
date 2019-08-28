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

if [[ $# -eq 1 ]]; then
  export DISTRO="${1}"
elif [[ -n "${KOKORO_JOB_NAME:-}" ]]; then
  # Kokoro injects the KOKORO_JOB_NAME environment variable, the value of this
  # variable is cloud-cpp/spanner/<config-file-name-without-cfg> (or more
  # generally <path/to/config-file-without-cfg>). By convention we name these
  # files `$foo.cfg` for continuous builds and `$foo-presubmit.cfg` for
  # presubmit builds. Here we extract the value of "foo" and use it as the build
  # name.
  DISTRO="$(basename "${KOKORO_JOB_NAME}" "-presubmit")"
  export DISTRO
else
 echo "Aborting build as the distribution name is not defined."
 echo "If you are invoking this script via the command line use:"
 echo "    $0 <distro-name>"
 echo
 echo "If this script is invoked by Kokoro, the CI system is expected to set"
 echo "the KOKORO_JOB_NAME environment variable."
 exit 1
fi

echo "================================================================"
echo "Change working directory to project root $(date)."
cd "$(dirname "$0")/../../.."

# TODO: this is a watered down version of
# google-cloud-cpp/ci/kokoro/install/build.sh with the functionality related to
# caching the image and uploading to gcr removed. We want to add that
# eventually.
echo "================================================================"
echo "Build base image with minimal development tools for ${DISTRO} $(date)."


readonly DEV_IMAGE="test-install-${DISTRO}"

devtools_flags=(
  # Only build up to the stage that installs the minimal development tools, but
  # does not compile any of our code.
  "--target" "devtools"
  # Create the image with the same tag as the cache we are using.
  "-t" "${DEV_IMAGE}:latest"
  "-f" "ci/kokoro/install/Dockerfile.${DISTRO}"
)

echo "Running docker build with " "${devtools_flags[@]}"
docker build "${devtools_flags[@]}" ci

echo "================================================================"
echo "Run validation script for INSTALL instructions on ${DISTRO}."
RUN_IMAGE="spanci-test-install-full-${DISTRO}:latest"
readonly RUN_IMAGE
docker build -t "${RUN_IMAGE}" \
  "--cache-from=${DEV_IMAGE}:latest" \
  "--target=install" \
  -f "ci/kokoro/install/Dockerfile.${DISTRO}" .
echo "================================================================"

CONFIG_DIRECTORY="${KOKORO_GFILE_DIR:-/dev/shm}"
readonly CONFIG_DIRECTORY
if [[ -f "${CONFIG_DIRECTORY}/spanner-integration-tests-config.sh" ]]; then
  # shellcheck source=/dev/null
  source "${CONFIG_DIRECTORY}/spanner-integration-tests-config.sh"

  run_args=(
    # Remove the container after running
    "--rm"

    # Set the environment variables for the test program.
    "--env" "GOOGLE_APPLICATION_CREDENTIALS=/c/spanner-credentials.json"
    "--env" "GOOGLE_CLOUD_PROJECT=${GOOGLE_CLOUD_PROJECT}"
    "--env" "GOOGLE_CLOUD_CPP_SPANNER_INSTANCE=${GOOGLE_CLOUD_CPP_SPANNER_INSTANCE}"

    # Mount the config directory as a volumne in `/c`
    "--volume" "${CONFIG_DIRECTORY}:/c"
  )
  echo "================================================================"
  echo "Run test program against installed libraries ${DISTRO}."
  docker run "${run_args[@]}" "${RUN_IMAGE}" "/o/spanner_install_test"
  echo "================================================================"
fi
