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

include(EnableClangTidy)
include(EnableWerror)

function (spanner_client_define_samples)
    add_library(spanner_sample_utils sample_utils.cc sample_utils.h)
    target_link_libraries(spanner_sample_utils
                          PRIVATE googleapis-c++::spanner_client)
    target_include_directories(spanner_sample_utils
                               PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
                                      $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>
                                      $<INSTALL_INTERFACE:include>)
    set(spanner_client_samples database_admin_samples.cc data_client_samples.cc)

    # Export the list of unit tests to a .bzl file so we do not need to maintain
    # the list in two places.
    export_list_to_bazel("spanner_client_samples.bzl" "spanner_client_samples")

    # Generate a target for each unit test.
    foreach (fname ${spanner_client_samples})
        string(REPLACE "/"
                       "_"
                       target
                       ${fname})
        string(REPLACE ".cc"
                       ""
                       target
                       ${target})
        add_executable(${target} ${fname})
        target_link_libraries(${target}
                              PRIVATE googleapis-c++::spanner_client
                                      spanner_sample_utils)
        google_cloud_cpp_add_clang_tidy(${target})
        google_cloud_cpp_add_common_options(${target})
        add_test(NAME ${target} COMMAND ${target})
        set_tests_properties(${target} PROPERTIES LABELS "integration-tests")
    endforeach ()
endfunction ()

if (BUILD_TESTING)
    spanner_client_define_samples()
endif (BUILD_TESTING)
