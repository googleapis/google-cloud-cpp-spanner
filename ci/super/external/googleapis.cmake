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

find_package(Threads REQUIRED)
include(external/external-project-helpers)

include(external/grpc)

if (NOT TARGET googleapis-project)
    # Give application developers a hook to configure the version and hash
    # downloaded from GitHub.
    set(GOOGLE_CLOUD_CPP_GOOGLEAPIS_URL
        "https://github.com/googleapis/cpp-cmakefiles/archive/v0.1.5.tar.gz")
    set(GOOGLE_CLOUD_CPP_GOOGLEAPIS_SHA256
        "f1443a10c545114b19fe9dc352568cb03b588813b12cc5db8780b64ae9a09ce1")

    google_cloud_cpp_set_prefix_vars()

    set_external_project_build_parallel_level(PARALLEL)

    include(ExternalProject)
    ExternalProject_Add(
        googleapis-project
        DEPENDS grpc-project
        EXCLUDE_FROM_ALL ON
        PREFIX "${CMAKE_BINARY_DIR}/external/googleapis"
        INSTALL_DIR "${GOOGLE_CLOUD_CPP_EXTERNAL_PREFIX}"
        URL ${GOOGLE_CLOUD_CPP_GOOGLEAPIS_URL}
        URL_HASH SHA256=${GOOGLE_CLOUD_CPP_GOOGLEAPIS_SHA256}
        LIST_SEPARATOR |
        CMAKE_ARGS -G${CMAKE_GENERATOR}
                   -DCMAKE_PREFIX_PATH=${GOOGLE_CLOUD_CPP_PREFIX_PATH}
                   -DCMAKE_INSTALL_RPATH=${GOOGLE_CLOUD_CPP_INSTALL_RPATH}
                   -DBUILD_SHARED_LIBS=${BUILD_SHARED_LIBS}
                   -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
        BUILD_COMMAND ${CMAKE_COMMAND}
                      --build
                      <BINARY_DIR>
                      ${PARALLEL}
        LOG_DOWNLOAD ON
        LOG_CONFIGURE ON
        LOG_BUILD ON
        LOG_INSTALL ON)
endif ()
