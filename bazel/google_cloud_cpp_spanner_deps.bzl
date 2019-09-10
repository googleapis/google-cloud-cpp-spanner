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
            strip_prefix = "google-cloud-cpp-0.13.0",
            urls = [
                "https://github.com/googleapis/google-cloud-cpp/archive/v0.13.0.tar.gz",
            ],
            sha256 = "35058ff14e4f9f49f78da2f1bbf1c03f27e8e40ec65c51f62720346e99803392",
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
                "https://github.com/googleapis/googleapis/archive/9c9f778aedde02f9826d2ae5d0f9c96409ba0f25.tar.gz",
            ],
            strip_prefix = "googleapis-9c9f778aedde02f9826d2ae5d0f9c96409ba0f25",
            sha256 = "13af135d8cc9b81b47d6fbfc258fe790a151956d06e01fd16671aa49fe536ab1",
            build_file = "@com_github_googleapis_google_cloud_cpp_spanner//bazel:googleapis.BUILD",
        )

    # Load gRPC and its dependencies, using a similar pattern to this function.
    # This implictly loads "com_google_protobuf", which we use.
    if "com_github_grpc_grpc" not in native.existing_rules():
        http_archive(
            name = "com_github_grpc_grpc",
            strip_prefix = "grpc-1.22.0",
            urls = [
                "https://github.com/grpc/grpc/archive/v1.22.0.tar.gz",
                "https://mirror.bazel.build/github.com/grpc/grpc/archive/v1.22.0.tar.gz",
            ],
            sha256 = "11ac793c562143d52fd440f6549588712badc79211cdc8c509b183cb69bddad8",
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
