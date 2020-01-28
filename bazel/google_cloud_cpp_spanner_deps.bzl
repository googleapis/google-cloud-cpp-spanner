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
            strip_prefix = "google-cloud-cpp-common-0.13.0",
            urls = [
                "https://github.com/googleapis/google-cloud-cpp-common/archive/v0.13.0.tar.gz",
            ],
            sha256 = "ea7f8f64ee8a6964f8755d1024b908bf13170e505f54b57ffc72c0002d478b8c",
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
            strip_prefix = "grpc-1.24.2",
            urls = [
                "https://github.com/grpc/grpc/archive/v1.24.2.tar.gz",
                "https://mirror.bazel.build/github.com/grpc/grpc/archive/v1.24.2.tar.gz",
            ],
            sha256 = "fd040f5238ff1e32b468d9d38e50f0d7f8da0828019948c9001e9a03093e1d8f",
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
