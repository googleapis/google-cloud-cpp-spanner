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

"""Examples for the Cloud Spanner C++ client library."""

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # Apache 2.0

load(":spanner_client_samples.bzl", "spanner_client_samples")

cc_library(
    name = "spanner_sample_utils",
    srcs = ["sample_utils.cc"],
    hdrs = ["sample_utils.h"],
    deps = [
        "//google/cloud/spanner:spanner_client",
    ],
)

[cc_test(
    name = "spanner_client_" + test.replace("/", "_").replace(".cc", ""),
    srcs = [test],
    tags = ["integration-tests"],
    deps = [
        "//google/cloud/spanner:spanner_client",
        "//google/cloud/spanner/samples:spanner_sample_utils",
        "@com_github_googleapis_google_cloud_cpp//google/cloud:google_cloud_cpp_common",
    ],
) for test in spanner_client_samples]
