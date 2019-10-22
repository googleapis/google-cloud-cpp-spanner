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

function (set_external_project_build_parallel_level var_name)
    if ("${CMAKE_GENERATOR}" STREQUAL "Unix Makefiles" OR "${CMAKE_GENERATOR}"
                                                          STREQUAL "Ninja")
        if (DEFINED ENV{NCPU})
            set(${var_name}
                "--" "-j" "$ENV{NCPU}"
                PARENT_SCOPE)
        else ()
            include(ProcessorCount)
            processorcount(NCPU)
            set(${var_name}
                "--" "-j" "${NCPU}"
                PARENT_SCOPE)
        endif ()
    else ()
        set(${var_name}
            ""
            PARENT_SCOPE)
    endif ()
endfunction ()

set(GOOGLE_CLOUD_CPP_EXTERNAL_PREFIX
    "${CMAKE_BINARY_DIR}/local"
    CACHE STRING "Configure where are the external projects installed.")
mark_as_advanced(GOOGLE_CLOUD_CPP_EXTERNAL_PREFIX)

function (google_cloud_cpp_set_prefix_vars)
    # When passing a semi-colon delimited list to ExternalProject_Add, we need
    # to escape the semi-colon. Quoting does not work and escaping the semi-
    # colon does not seem to work (see https://reviews.llvm.org/D40257). A
    # workaround is to use LIST_SEPARATOR to change the delimiter, which will
    # then be replaced by an escaped semi-colon by CMake. This allows us to use
    # multiple directories for our RPATH. Normally, it'd make sense to use : as
    # a delimiter since it is a typical path-list separator, but it is a special
    # character in CMake.
    set(GOOGLE_CLOUD_CPP_PREFIX_PATH "${CMAKE_PREFIX_PATH};<INSTALL_DIR>")
    string(REPLACE ";" "|" GOOGLE_CLOUD_CPP_PREFIX_PATH
                   "${GOOGLE_CLOUD_CPP_PREFIX_PATH}")

    # Depending on the platform libraries get installed in `lib` or `lib64`
    set(GOOGLE_CLOUD_CPP_INSTALL_RPATH "<INSTALL_DIR>/lib;<INSTALL_DIR>/lib64")
    string(REPLACE ";" "|" GOOGLE_CLOUD_CPP_INSTALL_RPATH
                   "${GOOGLE_CLOUD_CPP_INSTALL_RPATH}")

    set(GOOGLE_CLOUD_CPP_PREFIX_PATH
        "${GOOGLE_CLOUD_CPP_PREFIX_PATH}"
        PARENT_SCOPE)
    set(GOOGLE_CLOUD_CPP_PREFIX_RPATH
        "${GOOGLE_CLOUD_CPP_PREFIX_RPATH}"
        PARENT_SCOPE)
endfunction ()
