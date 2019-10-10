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

set(GOOGLE_CLOUD_CPP_SPANNER_ROOT "${PROJECT_SOURCE_DIR}/../../..")
list(APPEND CMAKE_MODULE_PATH "${GOOGLE_CLOUD_CPP_SPANNER_ROOT}/cmake"
            "${GOOGLE_CLOUD_CPP_SPANNER_ROOT}/ci/super")

include(external/external-project-helpers)
google_cloud_cpp_set_prefix_vars()

include(ExternalProject)
set(
    GOOGLE_CLOUD_CPP_URL
    "https://github.com/googleapis/google-cloud-cpp-common/archive/v0.13.0.tar.gz"
    )
set(GOOGLE_CLOUD_CPP_SHA256
    "ea7f8f64ee8a6964f8755d1024b908bf13170e505f54b57ffc72c0002d478b8c")
google_cloud_cpp_set_prefix_vars()

ExternalProject_Add(
    # Use very short names for the project. On Windows the path limit is about
    # 250 characters. The project name and prefix become part of the path, and
    # in the case of googleapis (a sub-project of `google-cloud-cpp`) using
    # full project names creates target files that exceed Windows limit.
    g-c-cpp
    EXCLUDE_FROM_ALL ON
    PREFIX "${CMAKE_BINARY_DIR}/e/g-c-cpp"
    INSTALL_DIR "${GOOGLE_CLOUD_CPP_EXTERNAL_PREFIX}"
    URL ${GOOGLE_CLOUD_CPP_URL}
    URL_HASH SHA256=${GOOGLE_CLOUD_CPP_SHA256}
    LIST_SEPARATOR |
    CMAKE_ARGS -DBUILD_TESTING=OFF
               -DGOOGLE_CLOUD_CPP_TESTING_UTIL_ENABLE_INSTALL=ON
               -DGOOGLE_CLOUD_CPP_DEPENDENCY_PROVIDER=package
               -DCMAKE_PREFIX_PATH=${GOOGLE_CLOUD_CPP_PREFIX_PATH}
               -DCMAKE_INSTALL_RPATH=${GOOGLE_CLOUD_CPP_INSTALL_RPATH}
               -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
               -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
               -DVCPKG_TARGET_TRIPLET=${VCPKG_TARGET_TRIPLET}
               -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    BUILD_COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR>
    LOG_DOWNLOAD ON
    LOG_CONFIGURE OFF
    LOG_BUILD OFF
    LOG_INSTALL OFF)

ExternalProject_Add(
    # See above to understand why we use very short names here.
    g-c-spanner
    DEPENDS g-c-cpp
    EXCLUDE_FROM_ALL OFF
    BUILD_ALWAYS 1
    PREFIX "${CMAKE_BINARY_DIR}/build/g-c-spanner"
    INSTALL_DIR "${GOOGLE_CLOUD_CPP_EXTERNAL_PREFIX}"
    SOURCE_DIR
    "${GOOGLE_CLOUD_CPP_SPANNER_ROOT}"
    LIST_SEPARATOR
    |
    CMAKE_ARGS -G${CMAKE_GENERATOR}
               -DCMAKE_PREFIX_PATH=${GOOGLE_CLOUD_CPP_PREFIX_PATH}
               -DCMAKE_INSTALL_RPATH=${GOOGLE_CLOUD_CPP_INSTALL_RPATH}
               -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
               -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
               -DVCPKG_TARGET_TRIPLET=${VCPKG_TARGET_TRIPLET}
               -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
               "-DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS} \
                  -I${GOOGLE_CLOUD_CPP_EXTERNAL_PREFIX}/include"
    BUILD_COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR>
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
add_dependencies(google-cloud-cpp-spanner-dependencies g-c-cpp)
