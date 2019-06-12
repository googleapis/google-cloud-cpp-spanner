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
include(external/curl)
include(external/crc32c)
include(external/grpc)

if (NOT TARGET google-cloud-cpp-project)
    # Give application developers a hook to configure the version and hash
    # downloaded from GitHub.
    set(
        GOOGLE_CLOUD_CPP_URL
        "https://github.com/googleapis/google-cloud-cpp/archive/75b62d0bb931d475137342d2d3932e588161fe95.tar.gz"
        )
    set(GOOGLE_CLOUD_CPP_SHA256
        "1c9e1448b44c20ac901430bc37cc04c443afed938a4ab85d283dd34f15b0b5c1")
    set(
        GOOGLEAPIS_URL
        "https://github.com/google/googleapis/archive/a8ee1416f4c588f2ab92da72e7c1f588c784d3e6.zip"
        )
    set(GOOGLEAPIS_SHA56
        "6b8a9b2bcb4476e9a5a9872869996f0d639c8d5df76dd8a893e79201f211b1cf")

    google_cloud_cpp_set_prefix_vars()

    ExternalProject_Add(
        google-cloud-cpp-project
        DEPENDS grpc-project curl-project crc32c-project
        EXCLUDE_FROM_ALL ON
        PREFIX "${CMAKE_BINARY_DIR}/external/google-cloud-cpp"
        INSTALL_DIR "${GOOGLE_CLOUD_CPP_EXTERNAL_PREFIX}"
        URL ${GOOGLE_CLOUD_CPP_URL}
        URL_HASH SHA256=${GOOGLE_CLOUD_CPP_SHA256}
        LIST_SEPARATOR |
        CMAKE_ARGS -DBUILD_TESTING=OFF
                   -DGOOGLE_CLOUD_CPP_DEPENDENCY_PROVIDER=package
                   -DGOOGLE_CLOUD_CPP_GOOGLEAPIS_URL=${GOOGLEAPIS_URL}
                   -DGOOGLE_CLOUD_CPP_GOOGLEAPIS_SHA256=${GOOGLEAPIS_SHA256}
                   -DCMAKE_PREFIX_PATH=${GOOGLE_CLOUD_CPP_PREFIX_PATH}
                   -DCMAKE_INSTALL_RPATH=${GOOGLE_CLOUD_CPP_INSTALL_RPATH}
                   -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
        BUILD_COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR>
        LOG_DOWNLOAD ON
        LOG_CONFIGURE OFF
        LOG_BUILD OFF
        LOG_INSTALL OFF)
endif ()
