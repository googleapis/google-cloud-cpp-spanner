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

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # Apache 2.0

load("@com_github_grpc_grpc//bazel:cc_grpc_library.bzl", "cc_grpc_library")

# This rule is needed by `google-cloud-cpp-common`. It lets us include headers
# from googleapis targets using angle brackets like system includes.
cc_library(
    name = "googleapis_system_includes",
    includes = [
        ".",
    ],
)

# TODO(googleapis/google-cloud-cpp#3838): Delete this rule once it's no longer
# used after the next release of -common.
cc_library(
    name = "grpc_utils_protos",
    deps = [
        ":googleapis_system_includes",
        "@com_github_grpc_grpc//:grpc++",
        "//google/rpc:status_cc_proto",
    ],
)
