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

function (google_cloud_cpp_test_name_to_target var fname)
    string(REPLACE "/" "_" target ${fname})
    string(REPLACE ".cc" "" target ${target})
    set("${var}"
        "${target}"
        PARENT_SCOPE)
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
