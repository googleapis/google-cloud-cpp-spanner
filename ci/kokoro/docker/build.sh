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

export CC=gcc
export CXX=g++
export DISTRO=ubuntu
export DISTRO_VERSION=18.04
export BAZEL_CONFIG=""
export CMAKE_SOURCE_DIR="."

in_docker_script="ci/kokoro/docker/build-in-docker-bazel.sh"

if [[ "${BUILD_NAME+x}" != "x" ]]; then
 echo "The BUILD_NAME is not defined or is empty. Fix the Kokoro .cfg file."
 exit 1
elif [[ "${BUILD_NAME}" = "clang-tidy" ]]; then
  # Compile with clang-tidy(1) turned on. The build treats clang-tidy warnings
  # as errors.
  export DISTRO=fedora-install
  export DISTRO_VERSION=30
  export CC=clang
  export CXX=clang++
  export CHECK_STYLE=yes
  export GENERATE_DOCS=yes
  export CLANG_TIDY=yes
  in_docker_script="ci/kokoro/docker/build-in-docker-cmake.sh"
elif [[ "${BUILD_NAME}" = "integration" ]]; then
  export CC=gcc
  export CXX=g++
  export RUN_INTEGRATION_TESTS=yes
elif [[ "${BUILD_NAME}" = "asan" ]]; then
  export BAZEL_CONFIG=asan
  export CC=clang
  export CXX=clang++
elif [[ "${BUILD_NAME}" = "ubsan" ]]; then
  export BAZEL_CONFIG=ubsan
  export CC=clang
  export CXX=clang++
elif [[ "${BUILD_NAME}" = "tsan" ]]; then
  export BAZEL_CONFIG=tsan
  export CC=clang
  export CXX=clang++
elif [[ "${BUILD_NAME}" = "cmake" ]]; then
  export DISTRO=fedora-install
  export DISTRO_VERSION=30
  in_docker_script="ci/kokoro/docker/build-in-docker-cmake.sh"
elif [[ "${BUILD_NAME}" = "cmake-super" ]]; then
  export CMAKE_SOURCE_DIR="ci/super"
  in_docker_script="ci/kokoro/docker/build-in-docker-cmake.sh"
else
  echo "Unknown BUILD_NAME (${BUILD_NAME}). Fix the Kokoro .cfg file."
  exit 1
fi

if [[ -z "${PROJECT_ROOT+x}" ]]; then
  readonly PROJECT_ROOT="$(cd "$(dirname "$0")/../../.."; pwd)"
fi
source "${PROJECT_ROOT}/ci/kokoro/linux-config.sh"
source "${PROJECT_ROOT}/ci/define-dump-log.sh"

echo "================================================================"
NCPU=$(nproc)
export NCPU
cd "${PROJECT_ROOT}"
echo "Building with ${NCPU} cores $(date) on ${PWD}."


echo "================================================================"
echo "Capture Docker version to troubleshoot $(date)."
sudo docker version
echo "================================================================"

echo "================================================================"
echo "Creating Docker image with all the development tools $(date)."
# We do not want to print the log unless there is an error, so disable the -e
# flag. Later, we will want to print out the emulator(s) logs *only* if there
# is an error, so disabling from this point on is the right choice.
set +e
mkdir -p "${BUILD_OUTPUT}"
if ! "${PROJECT_ROOT}/ci/retry-command.sh" \
       "${PROJECT_ROOT}/ci/kokoro/create-docker-image.sh" \
         >"${BUILD_OUTPUT}/create-build-docker-image.log" 2>&1 </dev/null; then
  dump_log "${BUILD_OUTPUT}/create-build-docker-image.log"
  exit 1
fi
echo "Docker image created $(date)."
sudo docker image ls
echo "================================================================"

echo "================================================================"
echo "Running the full build $(date)."
# When running on Travis the build gets a tty, and docker can produce nicer
# output in that case, but on Kokoro the script does not get a tty, and Docker
# terminates the program if we pass the `-it` flag in that case.
interactive_flag=""
if [[ -t 0 ]]; then
  interactive_flag="-it"
fi

# Make sure the user has a $HOME directory inside the Docker container.
mkdir -p "${BUILD_HOME}"

sudo docker run \
     --cap-add SYS_PTRACE \
     ${interactive_flag} \
     --env DISTRO="${DISTRO}" \
     --env DISTRO_VERSION="${DISTRO_VERSION}" \
     --env CXX="${CXX}" \
     --env CC="${CC}" \
     --env NCPU="${NCPU:-2}" \
     --env CHECK_STYLE="${CHECK_STYLE:-}" \
     --env CLANG_TIDY="${CLANG_TIDY:-}" \
     --env BAZEL_CONFIG="${BAZEL_CONFIG:-}" \
     --env RUN_INTEGRATION_TESTS="${RUN_INTEGRATION_TESTS:-}" \
     --env TERM="${TERM:-dumb}" \
     --env HOME="/v/${BUILD_HOME}" \
     --env USER="${USER}" \
     --user "${UID:-0}" \
     --volume "${PWD}":/v \
     --volume "${KOKORO_GFILE_DIR:-/dev/shm}":/c \
     --workdir /v \
     "${IMAGE}:tip" \
     "/v/${in_docker_script}" "${CMAKE_SOURCE_DIR}" \
     "${BUILD_OUTPUT}-${BUILD_NAME}"

exit_status=$?
echo "Build finished with ${exit_status} exit status $(date)."
echo "================================================================"

if [[ "${exit_status}" != 0 ]]; then
  echo "================================================================"
  echo "Build failed, printing out logs $(date)."
  "${PROJECT_ROOT}/ci/kokoro/dump-logs.sh"
  echo "================================================================"
fi

exit ${exit_status}
