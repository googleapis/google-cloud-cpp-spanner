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
project(google-cloud-cpp-spanner CXX C)

set(GOOGLE_CLOUD_CPP_CXX_STANDARD
    11
    CACHE STRING "Configure CMAKE_CXX_STANDARD")
mark_as_advanced(GOOGLE_CLOUD_CPP_CXX_STANDARD)

set(CMAKE_CXX_STANDARD "${GOOGLE_CLOUD_CPP_CXX_STANDARD}")
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

# If ccache is installed use it for the build. This makes the Travis
# configuration agnostic as to wether ccache is installed or not.
option(GOOGLE_CLOUD_CPP_ENABLE_CCACHE "Automatically use ccache if available"
       ON)
mark_as_advanced(GOOGLE_CLOUD_CPP_ENABLE_CCACHE)

if ("${GOOGLE_CLOUD_CPP_ENABLE_CCACHE}")
    find_program(GOOGLE_CLOUD_CPP_CCACHE_PROGRAM ccache NAMES /usr/bin/ccache)
    mark_as_advanced(GOOGLE_CLOUD_CPP_CCACHE_PROGRAM)
    if (GOOGLE_CLOUD_CPP_CCACHE_PROGRAM)
        message(STATUS "ccache found: ${GOOGLE_CLOUD_CPP_CCACHE_PROGRAM}")
        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE
                                     "${GOOGLE_CLOUD_CPP_CCACHE_PROGRAM}")
        set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK
                                     "${GOOGLE_CLOUD_CPP_CCACHE_PROGRAM}")

        set(GOOGLE_CLOUD_CPP_EXTERNAL_PROJECT_CCACHE
            "-DCMAKE_CXX_COMPILER_LAUNCHER=${GOOGLE_CLOUD_CPP_CCACHE_PROGRAM}"
            "-DCMAKE_CC_COMPILER_LAUNCHER=${GOOGLE_CLOUD_CPP_CCACHE_PROGRAM}")
    endif ()
endif ()

# If possible, enable a code coverage build type.
include(EnableCoverage)
include(SelectMSVCRuntime)

# C++ Exceptions are enabled by default, but allow the user to turn them off.
include(EnableCxxExceptions)

# This should be included from the top level CMakeLists file
set(SPANNER_CLIENT_VERSION_MAJOR 1)
set(SPANNER_CLIENT_VERSION_MINOR 2)
set(SPANNER_CLIENT_VERSION_PATCH 0)
string(CONCAT SPANNER_CLIENT_VERSION "${SPANNER_CLIENT_VERSION_MAJOR}" "."
              "${SPANNER_CLIENT_VERSION_MINOR}" "."
              "${SPANNER_CLIENT_VERSION_PATCH}")

include(CTest)

set(GOOGLE_CLOUD_CPP_COMMON_VERSION
    "master"
    CACHE STRING "The version of google-cloud-cpp-common")
mark_as_advanced(GOOGLE_CLOUD_CPP_COMMON_VERSION)

add_subdirectory(google/cloud/spanner)
