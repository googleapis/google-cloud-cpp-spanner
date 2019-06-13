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
elif [[ "${BUILD_NAME}" = "msan" ]]; then
  # We use Fedora for this build because (1) I was able to find instructions on
  # how to build libc++ with msan for that distribution, (2) Fedora has a
  # relatively recent version of Clang (8.0 as I write this).
  export DISTRO=fedora-libcxx-msan
  export DISTRO_VERSION=30
  export BAZEL_CONFIG=msan
  export CC=clang
  export CXX=clang++
elif [[ "${BUILD_NAME}" = "cmake" ]]; then
  export DISTRO=fedora-install
  export DISTRO_VERSION=30
  in_docker_script="ci/kokoro/docker/build-in-docker-cmake.sh"
elif [[ "${BUILD_NAME}" = "cmake-super" ]]; then
  export CMAKE_SOURCE_DIR="ci/super"
  in_docker_script="ci/kokoro/docker/build-in-docker-cmake.sh"
elif [[ "${BUILD_NAME}" = "gcc-4.8" ]]; then
  # The oldest version of GCC we support is 4.8, this build checks the code
  # against that version. The use of CentOS 7 for that build is not a
  # coincidence: the reason we support GCC 4.8 is to support this distribution
  # (and its commercial cousin: RHEL 7).
  export DISTRO=centos
  export DISTRO_VERSION=7
elif [[ "${BUILD_NAME}" = "cxx17" ]]; then
  export GOOGLE_CLOUD_CPP_CXX_STANDARD=17
  export DISTRO=fedora-install
  export DISTRO_VERSION=30
  export CC=gcc
  export CXX=g++
  in_docker_script="ci/kokoro/docker/build-in-docker-cmake.sh"
else
  echo "Unknown BUILD_NAME (${BUILD_NAME}). Fix the Kokoro .cfg file."
  exit 1
fi

if [[ -z "${PROJECT_ROOT+x}" ]]; then
  readonly PROJECT_ROOT="$(cd "$(dirname "$0")/../../.."; pwd)"
fi
source "${PROJECT_ROOT}/ci/kokoro/define-docker-variables.sh"
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
readonly CREATE_DOCKER_IMAGE_LOG="${BUILD_OUTPUT}/create-build-docker-image.log"
echo "Logging to ${CREATE_DOCKER_IMAGE_LOG}"
if ! "${PROJECT_ROOT}/ci/retry-command.sh" \
       "${PROJECT_ROOT}/ci/kokoro/create-docker-image.sh" \
         >"${CREATE_DOCKER_IMAGE_LOG}" 2>&1 </dev/null; then
  dump_log "${CREATE_DOCKER_IMAGE_LOG}"
  exit 1
fi
echo "Docker image created $(date)."
sudo docker image ls
echo "================================================================"

echo "================================================================"
echo "Running the full build $(date)."
# The default user for a Docker container has uid 0 (root). To avoid creating
# root-owned files in the build directory we tell docker to use the current
# user ID, if known.
docker_uid="${UID:-0}"
docker_user="${USER:-root}"
docker_home_prefix="${PWD}/cmake-out/home"
if [[ "${docker_uid}" == "0" ]]; then
  docker_home_prefix="${PWD}/cmake-out/root"
fi

# Make sure the user has a $HOME directory inside the Docker container.
mkdir -p "${BUILD_HOME}"

# We use an array for the flags so they are easier to document.
docker_flags=(
    # Grant the PTRACE capability to the Docker container running the build,
    # this is needed by tools like AddressSanitizer.
    "--cap-add" "SYS_PTRACE"

    # The name and version of the distribution, this is used to call
    # define-docker-variables.sh and determine the Docker image built, and the
    # output directory for any artifacts.
    "--env" "DISTRO=${DISTRO}"
    "--env" "DISTRO_VERSION=${DISTRO_VERSION}"

    # The C++ and C compiler, both Bazel and CMake use this environment variable
    # to select the compiler binary.
    "--env" "CXX=${CXX}"
    "--env" "CC=${CC}"

    # The number of CPUs, probably should be removed, the scripts can detect
    # this themselves in Kokoro (it was a problem on Travis).
    "--env" "NCPU=${NCPU:-4}"

    # If set to 'yes', the build script will run the style checks, including
    # clang-format, cmake-format, and buildifier.
    "--env" "CHECK_STYLE=${CHECK_STYLE:-}"

    # If set to 'yes', the build script will configure clang-tidy. Currently
    # only the CMake builds use this flag.
    "--env" "CLANG_TIDY=${CLANG_TIDY:-}"

    # If set to 'yes', run the integration tests. Currently only the Bazel
    # builds use this flag.
    "--env" "RUN_INTEGRATION_TESTS=${RUN_INTEGRATION_TESTS:-}"

    # If set to 'yes', run Doxygen to generate the documents and detect simple
    # errors in the documentation (e.g. partially documented parameter lists,
    # invalid links to functions or types). Currently only the CMake builds use
    # this flag.
    "--env" "GENERATE_DOCS=${GENERATE_DOCS:-}"

    # If set, pass -DGOOGLE_CLOUD_CPP_CXX_STANDARD=<value> to CMake.
    "--env" "GOOGLE_CLOUD_CPP_CXX_STANDARD=${GOOGLE_CLOUD_CPP_CXX_STANDARD:-}"

    # When running the integration tests this directory contains the
    # configuration files needed to run said tests. Make it available inside
    # the Docker container.
    "--volume" "${KOKORO_GFILE_DIR:-/dev/shm}:/c"

    # The argument for the --config option in Bazel, this is how we tell Bazel
    # to build with ASAN, UBSAN, TSAN, etc.
    "--env" "BAZEL_CONFIG=${BAZEL_CONFIG:-}"

    # Let the Docker image script know what kind of terminal we are using, that
    # produces properly colorized error messages.
    "--env" "TERM=${TERM:-dumb}"

    # Run the docker script and this user id. Because the docker image gets to
    # write in ${PWD} you typically want this to be your user id.
    "--user" "${docker_uid}"

    # Bazel needs this environment variable to work correctly.
    "--env" "USER=${docker_user}"

    # We give Bazel and CMake a fake $HOME inside the docker image. Bazel caches
    # build byproducts in this directory. CMake (when ccache is enabled) uses
    # it to store $HOME/.ccache
    "--env" "HOME=/h"
    "--volume" "${PWD}/${BUILD_HOME}:/h"

    # Mount the current directory (which is the top-level directory for the
    # project) as `/v` inside the docker image, and move to that directory.
    "--volume" "${PWD}:/v"
    "--workdir" "/v"
)

# When running the builds from the command-line they get a tty, and the scripts
# running inside the Docker container can produce nicer output. On Kokoro the
# script does not get a tty, and Docker terminates the program if we pass the
# `-it` flag.
if [[ -t 0 ]]; then
  docker_flags+=("-it")
fi

sudo docker run "${docker_flags[@]}" "${IMAGE}:tip" \
     "/v/${in_docker_script}" "${CMAKE_SOURCE_DIR}" \
     "${BUILD_OUTPUT}"

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
