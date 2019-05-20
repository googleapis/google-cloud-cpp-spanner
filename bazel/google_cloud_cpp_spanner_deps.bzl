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

    # Load a newer version of google test than what gRPC does.
    if "com_github_googleapis_google_cloud_cpp" not in native.existing_rules():
        http_archive(
            name = "com_github_googleapis_google_cloud_cpp",
            strip_prefix = "google-cloud-cpp-2ec0039ed1a91f7a0758feaf9d820a03a7c5f263",
            urls = [
                "https://github.com/googleapis/google-cloud-cpp/archive/2ec0039ed1a91f7a0758feaf9d820a03a7c5f263.tar.gz",
            ],
            sha256 = "c765bca47bb1cfcf651b5399be59a3ea8b0a49040f0010869c7f1c32b8a18b78",
        )

    # Load a newer version of google test than what gRPC does.
    if "com_google_googletest" not in native.existing_rules():
        http_archive(
            name = "com_google_googletest",
            strip_prefix = "googletest-b6cd405286ed8635ece71c72f118e659f4ade3fb",
            urls = [
                "https://github.com/google/googletest/archive/b6cd405286ed8635ece71c72f118e659f4ade3fb.tar.gz",
            ],
            sha256 = "8d9aa381a6885fe480b7d0ce8ef747a0b8c6ee92f99d74ab07e3503434007cb0",
        )

    # Load the googleapis dependency.
    if "com_google_googleapis" not in native.existing_rules():
        http_archive(
            name = "com_google_googleapis",
            urls = [
                "https://github.com/google/googleapis/archive/32a10f69e2c9ce15bba13ab1ff928bacebb25160.zip",
            ],
            strip_prefix = "googleapis-32a10f69e2c9ce15bba13ab1ff928bacebb25160",
            sha256 = "2c5dc6d4575e0804ccf3ffb22ca36d68621013c0b2980042d6fb4a04a4447b17",
            build_file = "//bazel:googleapis.BUILD",
        )

    # Load gRPC and its dependencies, using a similar pattern to this function.
    # This implictly loads "com_google_protobuf", which we use.
    if "com_github_grpc_grpc" not in native.existing_rules():
        http_archive(
            name = "com_github_grpc_grpc",
            strip_prefix = "grpc-1.21.0-pre1",
            urls = [
                "https://github.com/grpc/grpc/archive/v1.21.0-pre1.tar.gz",
                "https://mirror.bazel.build/github.com/grpc/grpc/archive/v1.21.0-pre1.tar.gz",
            ],
            sha256 = "cab0fb2a1ab25745234e412afb45bce6ebbe722c992ac1806ccfa279d9b04363",
        )

    # We use the cc_proto_library() rule from @com_google_protobuf, which
    # assumes that grpc_cpp_plugin and grpc_lib are in the //external: module
    native.bind(
        name = "grpc_cpp_plugin",
        actual = "@com_github_grpc_grpc//:grpc_cpp_plugin",
    )

    native.bind(
        name = "grpc_lib",
        actual = "@com_github_grpc_grpc//:grpc++",
    )
