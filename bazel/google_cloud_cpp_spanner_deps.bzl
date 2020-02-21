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

"""Load dependencies for the google-cloud-cpp-spanner library."""

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive", "http_file")

def google_cloud_cpp_spanner_deps():
    """Loads dependencies need to compile the google-cloud-cpp-spanner library.

    Application developers can call this function from their WORKSPACE file
    to obtain all the necessary dependencies for google-cloud-cpp-spanner,
    including gRPC and its dependencies. This function only loads
    dependencies that have not been previously loaded, allowing
    application developers to override the version of the dependencies
    they want to use.
    """

    # Load rules_cc, used by googletest
    if "rules_cc" not in native.existing_rules():
        http_archive(
            name = "rules_cc",
            strip_prefix = "rules_cc-a508235df92e71d537fcbae0c7c952ea6957a912",
            urls = [
                "https://github.com/bazelbuild/rules_cc/archive/a508235df92e71d537fcbae0c7c952ea6957a912.tar.gz",
            ],
            sha256 = "d21d38c4b8e81eed8fa95ede48dd69aba01a3b938be6ac03d2b9dc61886a7183",
        )

    # Load google-cloud-cpp-common.
    if "com_github_googleapis_google_cloud_cpp_common" not in native.existing_rules():
        http_archive(
            name = "com_github_googleapis_google_cloud_cpp_common",
            strip_prefix = "google-cloud-cpp-common-0.20.0",
            urls = [
                "https://github.com/googleapis/google-cloud-cpp-common/archive/v0.20.0.tar.gz",
            ],
            sha256 = "0f5a5e03a6447d68778b331cbc43ad4e9c27a519275261c4eb7b8f15d554cba3",
        )

    # Load a version of googletest that we know works.
    if "com_google_googletest" not in native.existing_rules():
        http_archive(
            name = "com_google_googletest",
            strip_prefix = "googletest-release-1.10.0",
            urls = [
                "https://github.com/google/googletest/archive/release-1.10.0.tar.gz",
            ],
            sha256 = "9dc9157a9a1551ec7a7e43daea9a694a0bb5fb8bec81235d8a1e6ef64c716dcb",
        )

    # Load a version of benchmark that we know works.
    if "com_google_benchmark" not in native.existing_rules():
        http_archive(
            name = "com_google_benchmark",
            strip_prefix = "benchmark-1.5.0",
            urls = [
                "https://github.com/google/benchmark/archive/v1.5.0.tar.gz",
            ],
            sha256 = "3c6a165b6ecc948967a1ead710d4a181d7b0fbcaa183ef7ea84604994966221a",
        )

    # Load the googleapis dependency.
    if "com_google_googleapis" not in native.existing_rules():
        http_archive(
            name = "com_google_googleapis",
            urls = [
                "https://github.com/googleapis/googleapis/archive/19c4589a3cb44b3679f7b3fba88365b3d055d5f8.tar.gz",
            ],
            strip_prefix = "googleapis-19c4589a3cb44b3679f7b3fba88365b3d055d5f8",
            sha256 = "ef455e46cfb967962aef30248f1a2a69bc78b041e89b04644e24e7844f0215c4",
            build_file = "@com_github_googleapis_google_cloud_cpp_spanner//bazel:googleapis.BUILD",
        )

    # Load protobuf.
    if "com_google_protobuf" not in native.existing_rules():
        http_archive(
            name = "com_google_protobuf",
            strip_prefix = "protobuf-3.11.3",
            urls = [
                "https://github.com/google/protobuf/archive/v3.11.3.tar.gz",
            ],
            sha256 = "cf754718b0aa945b00550ed7962ddc167167bd922b842199eeb6505e6f344852",
        )

    # Load gRPC and its dependencies, using a similar pattern to this function.
    if "com_github_grpc_grpc" not in native.existing_rules():
        http_archive(
            name = "com_github_grpc_grpc",
            strip_prefix = "grpc-78ace4cd5dfcc1f2eced44d22d752f103f377e7b",
            urls = [
                "https://github.com/grpc/grpc/archive/78ace4cd5dfcc1f2eced44d22d752f103f377e7b.tar.gz",
            ],
            sha256 = "a2034a1c8127e35c0cc7b86c1b5ad6d8e79a62c5e133c379b8b22a78ba370015",
        )

    # We use the cc_proto_library() rule from @com_google_protobuf, which
    # assumes that grpc_cpp_plugin and grpc_lib are in the //external: module
    native.bind(
        name = "grpc_cpp_plugin",
        actual = "@com_github_grpc_grpc//src/compiler:grpc_cpp_plugin",
    )

    native.bind(
        name = "grpc_lib",
        actual = "@com_github_grpc_grpc//:grpc++",
    )
