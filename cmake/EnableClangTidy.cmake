# ~~~
# Copyright 2019 Google Inc.
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

option(GOOGLE_CLOUD_CPP_CLANG_TIDY
       "If set compiles the Cloud Cloud C++ Libraries with clang-tidy." "")
mark_as_advanced(GOOGLE_CLOUD_CPP_CLANG_TIDY)

if (${CMAKE_VERSION} VERSION_LESS "3.6")
    message(STATUS "clang-tidy is not enabled because cmake version is too old")
else ()
    if (${CMAKE_VERSION} VERSION_LESS "3.8")
        message(WARNING "clang-tidy exit code ignored in this version of cmake")
    endif ()
    find_program(
        GOOGLE_CLOUD_CPP_CLANG_TIDY_PROGRAM
        NAMES "clang-tidy"
        DOC "Path to clang-tidy executable")
    mark_as_advanced(GOOGLE_CLOUD_CPP_CLANG_TIDY_PROGRAM)
    if (NOT GOOGLE_CLOUD_CPP_CLANG_TIDY_PROGRAM)
        message(STATUS "clang-tidy not found.")
    else ()
        message(
            STATUS "clang-tidy found: ${GOOGLE_CLOUD_CPP_CLANG_TIDY_PROGRAM}")
    endif ()
endif ()

function (google_cloud_cpp_add_clang_tidy target)
    if (GOOGLE_CLOUD_CPP_CLANG_TIDY_PROGRAM AND GOOGLE_CLOUD_CPP_CLANG_TIDY)
        set_target_properties(
            ${target} PROPERTIES CXX_CLANG_TIDY
                                 "${GOOGLE_CLOUD_CPP_CLANG_TIDY_PROGRAM}")
    endif ()
endfunction ()
