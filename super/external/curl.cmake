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

include(ExternalProject)
include(external/external-project-helpers)
include(external/c-ares)
include(external/ssl)
include(external/zlib)

if (NOT TARGET curl-project)
    # Give application developers a hook to configure the version and hash
    # downloaded from GitHub.
    set(GOOGLE_CLOUD_CPP_CURL_URL
        "https://curl.haxx.se/download/curl-7.60.0.tar.gz")
    set(GOOGLE_CLOUD_CPP_CURL_SHA256
        "e9c37986337743f37fd14fe8737f246e97aec94b39d1b71e8a5973f72a9fc4f5")

    google_cloud_cpp_set_prefix_vars()

    set_external_project_build_parallel_level(PARALLEL)

    ExternalProject_Add(
        curl-project
        DEPENDS c-ares-project ssl-project zlib-project
        EXCLUDE_FROM_ALL ON
        PREFIX "${CMAKE_BINARY_DIR}/external/curl"
        INSTALL_DIR "${GOOGLE_CLOUD_CPP_EXTERNAL_PREFIX}"
        URL ${GOOGLE_CLOUD_CPP_CURL_URL}
        URL_HASH SHA256=${GOOGLE_CLOUD_CPP_CURL_SHA256}
        LIST_SEPARATOR |
        # libcurl automatically enables a number of protocols. With static
        # libraries this is a problem. The indirect dependencies, such as
        # libldap, become hard to predict and manage. Setting HTTP_ONLY=ON and
        # CMAKE_ENABLE_OPENSSL=ON disables most optional protocols and meets
        # our needs. If the application needs a version of libcurl with other
        # protocols enabled they can provide their own curl-project target.
        CMAKE_ARGS -G${CMAKE_GENERATOR}
                   -DHTTP_ONLY=ON
                   -DCMAKE_ENABLE_OPENSSL=ON
                   -DENABLE_ARES=ON
                   -DCMAKE_PREFIX_PATH=${GOOGLE_CLOUD_CPP_PREFIX_PATH}
                   -DCMAKE_INSTALL_PATH=${GOOGLE_CLOUD_CPP_INSTALL_PATH}
                   -DCMAKE_INSTALL_RPATH=${GOOGLE_CLOUD_CPP_INSTALL_RPATH}
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
