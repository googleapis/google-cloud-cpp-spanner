# ~~~
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
# ~~~

cmake_minimum_required(VERSION 3.5)
project(google-cloud-cpp-spanner-super-build CXX C)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(GOOGLE_CLOUD_CPP_SPANNER_ROOT "${PROJECT_SOURCE_DIR}/../..")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}")

include(external/google-cloud-cpp-common)
include(external/googletest)
include(external/external-project-helpers)

google_cloud_cpp_set_prefix_vars()

set_external_project_build_parallel_level(PARALLEL)

include(ExternalProject)
ExternalProject_Add(
    google-cloud-cpp-spanner-as-external
    DEPENDS google-cloud-cpp-common-project googletest-project
    EXCLUDE_FROM_ALL OFF
    BUILD_ALWAYS 1
    PREFIX "${CMAKE_BINARY_DIR}/build/google-cloud-cpp-spanner"
    INSTALL_DIR "${GOOGLE_CLOUD_CPP_EXTERNAL_PREFIX}"
    SOURCE_DIR
    "${GOOGLE_CLOUD_CPP_SPANNER_ROOT}"
    LIST_SEPARATOR
    |
    CMAKE_ARGS -G${CMAKE_GENERATOR}
               -DCMAKE_PREFIX_PATH=${GOOGLE_CLOUD_CPP_PREFIX_PATH}
               -DCMAKE_INSTALL_RPATH=${GOOGLE_CLOUD_CPP_INSTALL_RPATH}
               -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
    BUILD_COMMAND ${CMAKE_COMMAND}
                  --build
                  <BINARY_DIR>
                  ${PARALLEL}
    TEST_COMMAND ${CMAKE_CTEST_COMMAND}
                 --output-on-failure
                 -LE
                 integration-tests
                 # TODO(#23) - Use this when install is implemented.
    INSTALL_COMMAND ""
    LOG_DOWNLOAD OFF
    LOG_CONFIGURE OFF
    LOG_BUILD OFF
    LOG_INSTALL OFF)

# This makes it easy to compile the dependencies before the code.
add_custom_target(google-cloud-cpp-spanner-dependencies)
add_dependencies(google-cloud-cpp-spanner-dependencies
                 google-cloud-cpp-common-project googletest-project)
