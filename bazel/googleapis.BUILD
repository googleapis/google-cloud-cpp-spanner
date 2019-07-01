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

######################
# BEGIN SPANNER PROTOS
######################

# WARNING:
# These Spanner protos exist for prototyping only. Users should not rely on
# these protos and bazel build targets existing in the future. We will
# remove/rename these without notice.

cc_proto_library(
    name = "spanner_cc_proto",
    deps = ["//google/spanner/v1:spanner_proto"],
)

cc_grpc_library(
    name = "grpc_spanner_proto",
    srcs = ["//google/spanner/v1:spanner_proto"],
    deps = [":spanner_cc_proto"],
    grpc_only = True,
    well_known_protos = True,
    use_external = True,
)

cc_proto_library(
    name = "spanner_admin_instance_cc_proto",
    deps = ["//google/spanner/admin/instance/v1:spanner_admin_instance_proto"],
)

# Instance Admin
cc_grpc_library(
    name = "google_spanner_admin_instance_v1_spanner_instance_admin",
    srcs = ["//google/spanner/admin/instance/v1:spanner_admin_instance_proto"],
    deps = [
        ":spanner_admin_instance_cc_proto",
        ":spanner_cc_proto",
        "//google/longrunning:longrunning_cc_grpc",
    ],
    grpc_only = True,
    well_known_protos = True,
    use_external = True,
)

cc_proto_library(
    name = "spanner_admin_database_cc_proto",
    deps = ["//google/spanner/admin/database/v1:spanner_admin_database_proto"],
)

# Database Admin
cc_grpc_library(
    name = "google_spanner_admin_database_v1_spanner_database_admin",
    srcs = ["//google/spanner/admin/database/v1:spanner_admin_database_proto"],
    deps = [
        ":spanner_admin_database_cc_proto",
        "//google/longrunning:longrunning_cc_grpc",
    ],
    grpc_only = True,
    well_known_protos = True,
    use_external = True,
)

cc_library(
    name = "spanner_protos",
    deps = [
        ":grpc_spanner_proto",
        ":google_spanner_admin_instance_v1_spanner_instance_admin",
        ":google_spanner_admin_database_v1_spanner_database_admin",
        "@com_github_grpc_grpc//:grpc++",
    ],
    includes = ["."],
)

####################
# END SPANNER PROTOS
####################

cc_library(
    name = "grpc_utils_protos",
    includes = [
        ".",
    ],
    deps = [
        "@com_github_grpc_grpc//:grpc++",
        "//google/rpc:status_cc_proto"
    ],
)

# TODO(googleapis/google-cloud-cpp#2807) - will not need these.
cc_proto_library(
    name = "bigtableadmin_cc_proto",
    deps = ["//google/bigtable/admin/v2:bigtableadmin_proto"],
)

cc_proto_library(
    name = "bigtable_cc_proto",
    deps = ["//google/bigtable/v2:bigtable_proto"],
)

cc_grpc_library(
    name = "bigtableadmin_cc_grpc",
    srcs = [
        "//google/bigtable/admin/v2:bigtableadmin_proto",
    ],
    grpc_only = True,
    use_external = True,
    well_known_protos = True,
    deps = [
        ":bigtableadmin_cc_proto",
        "//google/longrunning:longrunning_cc_grpc"
    ],
)

cc_grpc_library(
    name = "bigtable_cc_grpc",
    srcs = ["//google/bigtable/v2:bigtable_proto"],
    grpc_only = True,
    use_external = True,
    well_known_protos = True,
    deps = [
        ":bigtable_cc_proto",
    ],
)

cc_library(
    name = "bigtable_protos",
    includes = [
        ".",
    ],
    deps = [
        "@com_github_grpc_grpc//:grpc++",
        ":bigtable_cc_grpc",
        ":bigtable_cc_proto",
        ":bigtableadmin_cc_grpc",
        ":bigtableadmin_cc_proto",
        "//google/rpc:error_details_cc_proto"
    ],
)
