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

BUILD_AND_TEST_PROJECT_FRAGMENT=$(replace_fragments \
      "INSTALL_CPP_CMAKEFILES_FROM_SOURCE" \
      "INSTALL_GOOGLETEST_FROM_SOURCE" \
      "INSTALL_GOOGLE_CLOUD_CPP_COMMON_FROM_SOURCE" <<'_EOF_'
# #### googleapis

# We need a recent version of the Google Cloud Platform proto C++ libraries:

# ```bash
@INSTALL_CPP_CMAKEFILES_FROM_SOURCE@
# ```

# #### googletest

# We need a recent version of GoogleTest to compile the unit and integration
# tests.

# ```bash
@INSTALL_GOOGLETEST_FROM_SOURCE@
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
RUN cmake -H. -B/o
RUN cmake --build /o -- -j ${NCPU:-4}
WORKDIR /o
RUN ctest -LE integration-tests --output-on-failure
WORKDIR /home/build/project
RUN cmake --build /o --target install
# ```

## [END INSTALL.md]

ENV PKG_CONFIG_PATH=/usr/local/lib64/pkgconfig

# Verify that the installed files are actually usable
WORKDIR /home/build/test-install-cmake
COPY ci/test-install /home/build/test-install-cmake
COPY google/cloud/spanner/integration_tests/spanner_install_test.cc /home/build/test-install-cmake
# Disable pkg-config with CMake to verify it is not used in package discovery.
RUN env -u PKG_CONFIG_PATH cmake -H. -B/i
RUN cmake --build /i -- -j ${NCPU:-4}
_EOF_
)
