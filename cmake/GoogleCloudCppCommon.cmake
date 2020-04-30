# ~~~
# Copyright 2018 Google LLC
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

# Find out the name of the subproject.
get_filename_component(GOOGLE_CLOUD_CPP_SUBPROJECT
                       "${CMAKE_CURRENT_SOURCE_DIR}" NAME)

# Find out what flags turn on all available warnings and turn those warnings
# into errors.
include(CheckCXXCompilerFlag)
if (NOT MSVC)
    check_cxx_compiler_flag(-Wall GOOGLE_CLOUD_CPP_COMPILER_SUPPORTS_WALL)
    check_cxx_compiler_flag(-Wextra GOOGLE_CLOUD_CPP_COMPILER_SUPPORTS_WEXTRA)
    check_cxx_compiler_flag(-Werror GOOGLE_CLOUD_CPP_COMPILER_SUPPORTS_WERROR)
else ()
    check_cxx_compiler_flag("/std:c++latest"
                            GOOGLE_CLOUD_CPP_COMPILER_SUPPORTS_CPP_LATEST)
endif ()

# If possible, enable a code coverage build type.
include(EnableCoverage)

# Include support for clang-tidy if available
include(EnableClangTidy)

# C++ Exceptions are enabled by default, but allow the user to turn them off.
include(EnableCxxExceptions)

# Get the destination directories based on the GNU recommendations.
include(GNUInstallDirs)

# Pick the right MSVC runtime libraries.
include(SelectMSVCRuntime)

# Enable doxygen
include(EnableDoxygen)

function (google_cloud_cpp_add_common_options target)
    if (GOOGLE_CLOUD_CPP_COMPILER_SUPPORTS_CPP_LATEST)
        target_compile_options(${target} INTERFACE "/std:c++latest")
    endif ()
    if (GOOGLE_CLOUD_CPP_COMPILER_SUPPORTS_WALL)
        target_compile_options(${target} INTERFACE "-Wall")
    endif ()
    if (GOOGLE_CLOUD_CPP_COMPILER_SUPPORTS_WEXTRA)
        target_compile_options(${target} INTERFACE "-Wextra")
    endif ()
    if (GOOGLE_CLOUD_CPP_COMPILER_SUPPORTS_WERROR)
        target_compile_options(${target} INTERFACE "-Werror")
    endif ()
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU"
        AND "${CMAKE_CXX_COMPILER_VERSION}" VERSION_LESS 5.0)
        # With GCC 4.x this warning is too noisy to be useful.
        target_compile_options(${target}
                               INTERFACE "-Wno-missing-field-initializers")
    endif ()
endfunction ()

function (google_cloud_cpp_add_clang_tidy target)
    if (GOOGLE_CLOUD_CPP_CLANG_TIDY_PROGRAM AND GOOGLE_CLOUD_CPP_CLANG_TIDY)
        set_target_properties(
            ${target} PROPERTIES CXX_CLANG_TIDY
                                 "${GOOGLE_CLOUD_CPP_CLANG_TIDY_PROGRAM}")
    endif ()
endfunction ()

#
# google_cloud_cpp_install_headers : install all the headers in a target
#
# Find all the headers in @p target and install them at @p destination,
# preserving the directory structure.
#
# * target the name of the target.
# * destination the destination directory, relative to <PREFIX>. Typically this
#   starts with `include/google/cloud`, the function requires the full
#   destination in case some headers get installed elsewhere in the future.
#
function (google_cloud_cpp_install_headers target destination)
    get_target_property(target_sources ${target} SOURCES)
    foreach (header ${target_sources})
        if (NOT "${header}" MATCHES "\\.h$" AND NOT "${header}" MATCHES
                                                "\\.inc$")
            continue()
        endif ()
        string(REPLACE "${CMAKE_CURRENT_BINARY_DIR}/" "" relative "${header}")
        get_filename_component(dir "${relative}" DIRECTORY)
        install(FILES "${header}" DESTINATION "${destination}/${dir}")
    endforeach ()
endfunction ()
