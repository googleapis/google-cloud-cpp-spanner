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

declare -A ORIGINAL_COPYRIGHT_YEAR=(
  [centos-7]=2019
  [centos-8]=2019
  [debian-buster]=2019
  [debian-stretch]=2019
  [fedora]=2019
  [opensuse-leap]=2019
  [opensuse-tumbleweed]=2019
  [ubuntu-trusty]=2019
  [ubuntu-xenial]=2019
  [ubuntu-bionic]=2019
)

source "$(dirname "${BASH_SOURCE[0]}")/version-config.sh"

INSTALL_GOOGLE_CLOUD_CPP_COMMON_FROM_SOURCE="WORKDIR /var/tmp/build
RUN wget -q https://github.com/googleapis/google-cloud-cpp-common/archive/v${GOOGLE_CLOUD_CPP_COMMON_VERSION}.tar.gz && \\
    tar -xf v${GOOGLE_CLOUD_CPP_COMMON_VERSION}.tar.gz && \\
    cd google-cloud-cpp-common-${GOOGLE_CLOUD_CPP_COMMON_VERSION} && \\
    cmake -H. -Bcmake-out -DBUILD_TESTING=OFF && \\
    cmake --build cmake-out -- -j \${NCPU:-4} && \\
    cmake --build cmake-out --target install -- -j \${NCPU:-4} && \\
    ldconfig"

BUILD_AND_TEST_PROJECT_FRAGMENT=$(replace_fragments \
      "INSTALL_CPP_CMAKEFILES_FROM_SOURCE" \
      "INSTALL_GOOGLETEST_FROM_SOURCE" \
      "INSTALL_GOOGLE_BENCHMARK_FROM_SOURCE" \
      "INSTALL_GOOGLE_CLOUD_CPP_COMMON_FROM_SOURCE" <<'_EOF_'
# #### googleapis

# We need a recent version of the Google Cloud Platform proto C++ libraries:

# ```bash
@INSTALL_CPP_CMAKEFILES_FROM_SOURCE@
# ```

# #### google-cloud-cpp-common

# The project also depends on google-cloud-cpp-common, the libraries shared by
# all the Google Cloud C++ client libraries:

# ```bash
@INSTALL_GOOGLE_CLOUD_CPP_COMMON_FROM_SOURCE@
# ```

FROM devtools AS install

# #### Compile and install the main project

# We can now compile, test, and install `@GOOGLE_CLOUD_CPP_REPOSITORY@`.

# ```bash
WORKDIR /home/build/project
COPY . /home/build/project
RUN cmake -H. -Bcmake-out -DBUILD_TESTING=OFF -DCMAKE_BUILD_TYPE=Release
RUN cmake --build cmake-out -- -j "${NCPU:-4}"
RUN cmake --build cmake-out --target install
# ```

## [END INSTALL.md]

ENV PKG_CONFIG_PATH=/usr/local/lib64/pkgconfig

# Verify that the installed files are actually usable
WORKDIR /home/build/test-install-cmake
COPY google/cloud/spanner/quickstart /home/build/test-install-cmake
# Disable pkg-config with CMake to verify it is not used in package discovery.
RUN env -u PKG_CONFIG_PATH cmake -H. -B/i
RUN cmake --build /i -- -j "${NCPU:-4}"
_EOF_
)
