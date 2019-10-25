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

CONFIG_DIRECTORY="${KOKORO_GFILE_DIR:-/dev/shm}"
readonly CONFIG_DIRECTORY
if [[ -f "${CONFIG_DIRECTORY}/spanner-integration-tests-config.sh" ]]; then
  source "${CONFIG_DIRECTORY}/spanner-integration-tests-config.sh"

  run_args=(
    # Remove the container after running
    "--rm"

    # Set the environment variables for the test program.
    "--env" "GOOGLE_APPLICATION_CREDENTIALS=/c/spanner-credentials.json"
    "--env" "GOOGLE_CLOUD_PROJECT=${GOOGLE_CLOUD_PROJECT}"
    "--env" "RUN_SLOW_INTEGRATION_TESTS=${RUN_SLOW_INTEGRATION_TESTS:-no}"
    "--env" "GOOGLE_CLOUD_CPP_SPANNER_INSTANCE=${GOOGLE_CLOUD_CPP_SPANNER_INSTANCE}"
    "--env" "GOOGLE_CLOUD_CPP_SPANNER_IAM_TEST_SA=${GOOGLE_CLOUD_CPP_SPANNER_IAM_TEST_SA}"

    # Mount the config directory as a volumne in `/c`
    "--volume" "${CONFIG_DIRECTORY}:/c"
  )
  echo "================================================================"
  echo "Run test program against installed libraries ${DISTRO}."
  docker run "${run_args[@]}" "${INSTALL_RUN_IMAGE}" "/i/spanner_install_test"
  echo "================================================================"
fi
