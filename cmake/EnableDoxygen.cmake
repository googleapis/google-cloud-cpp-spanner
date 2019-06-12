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

if (${CMAKE_VERSION} VERSION_LESS "3.9")

    # Old versions of CMake have really poor support for Doxygen generation.
    message(STATUS "Doxygen generation only enabled for cmake 3.9 and higher")
else ()
    # Automatically download the Doxygen documentation for `std::` from
    # https://cppreference.com

    find_package(Doxygen)
    if (Doxygen_FOUND)
        set(DOXYGEN_RECURSIVE YES)
        set(DOXYGEN_FILE_PATTERNS
            *.h
            *.cc
            *.proto
            *.dox)
        set(DOXYGEN_EXAMPLE_RECURSIVE YES)
        set(DOXYGEN_EXCLUDE "third_party" "cmake-build-debug" "cmake-out")
        set(DOXYGEN_EXCLUDE_SYMLINKS YES)
        set(DOXYGEN_QUIET YES)
        set(DOXYGEN_WARN_AS_ERROR YES)
        set(DOXYGEN_INLINE_INHERITED_MEMB YES)
        set(DOXYGEN_JAVADOC_AUTOBRIEF YES)
        set(DOXYGEN_BUILTIN_STL_SUPPORT YES)
        set(DOXYGEN_IDL_PROPERTY_SUPPORT NO)
        set(DOXYGEN_EXTRACT_ALL YES)
        set(DOXYGEN_EXTRACT_STATIC YES)
        set(DOXYGEN_SORT_MEMBERS_CTORS_1ST YES)
        set(DOXYGEN_GENERATE_TODOLIST NO)
        set(DOXYGEN_GENERATE_BUGLIST NO)
        set(DOXYGEN_GENERATE_TESTLIST NO)
        set(DOXYGEN_CLANG_ASSISTED_PARSING YES)
        set(DOXYGEN_CLANG_OPTIONS
            "-std=c++11 -I${PROJECT_SOURCE_DIR} -I${PROJECT_BINARY_DIR}")
        set(DOXYGEN_GENERATE_LATEX NO)
        set(DOXYGEN_GRAPHICAL_HIERARCHY NO)
        set(DOXYGEN_DIRECTORY_GRAPH NO)
        set(DOXYGEN_CLASS_GRAPH NO)
        set(DOXYGEN_COLLABORATION_GRAPH NO)
        set(DOXYGEN_INCLUDE_GRAPH NO)
        set(DOXYGEN_INCLUDED_BY_GRAPH NO)
        set(DOXYGEN_DOT_TRANSPARENT YES)
        set(DOXYGEN_MACRO_EXPANSION YES)
        set(DOXYGEN_EXPAND_ONLY_PREDEF YES)
        set(DOXYGEN_HTML_TIMESTAMP)
        set(DOXYGEN_STRIP_FROM_INC_PATH "${PROJECT_SOURCE_DIR}")
        set(DOXYGEN_SHOW_USED_FILES NO)
        set(DOXYGEN_REFERENCES_LINK_SOURCE NO)
        set(DOXYGEN_SOURCE_BROWSER YES)
        set(DOXYGEN_GENERATE_TAGFILE
            "${CMAKE_CURRENT_BINARY_DIR}/${GOOGLE_CLOUD_CPP_SUBPROJECT}.tag")
        set(DOXYGEN_EXCLUDE_SYMBOL "internal")
        set(DOXYGEN_PREDEFINED
            "SPANNER_CLIENT_NS=v${SPANNER_CLIENT_VERSION_MAJOR}")
        set(DOXYGEN_EXCLUDE_PATTERNS "*/google/cloud/spanner/README.md"
                                     "*/google/cloud/spanner/internal/*"
                                     "*/google/cloud/spanner/*_test.cc")

        doxygen_add_docs(doxygen-docs
                         ${CMAKE_CURRENT_SOURCE_DIR}
                         WORKING_DIRECTORY
                         ${CMAKE_CURRENT_SOURCE_DIR}
                         COMMENT
                         "Generate HTML documentation")
    endif ()
endif ()
