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

  run_args=(
    # Remove the container after running
    "--rm"

    # Set the environment variables for the test program.
    "--env" "GOOGLE_APPLICATION_CREDENTIALS=/c/spanner-credentials.json"
    "--env" "GOOGLE_CLOUD_PROJECT=${GOOGLE_CLOUD_PROJECT}"

    # Mount the config directory as a volume in `/c`
    "--volume" "${CONFIG_DIRECTORY}:/c"
  )
  echo "================================================================"
  echo "Run test program against installed libraries ${DISTRO}."
  docker run "${run_args[@]}" "${INSTALL_RUN_IMAGE}" "/i/quickstart" \
      "${GOOGLE_CLOUD_PROJECT}" "${GOOGLE_CLOUD_CPP_SPANNER_INSTANCE}" quickstart-db
  echo "================================================================"
fi
