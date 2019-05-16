# Copyright 2018 Google LLC
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

cc_proto_library(
    name = "cc_http_proto",
    deps = ["//google/api:http_proto"],
)

cc_grpc_library(
    name = "google_api_http",
    srcs = [":cc_http_proto"],
    deps = [":cc_http_proto"],
    proto_only = False,
    well_known_protos = True,
    use_external = True,
)

cc_proto_library(
name = "cc_annotations_proto",
deps = ["//google/api:annotations_proto"],
)

cc_grpc_library(
    name = "google_api_annotations",
    srcs = [":cc_annotations_proto"],
    deps = [":google_api_http", ":cc_annotations_proto"],
    proto_only = False,
    well_known_protos = True,
    use_external = True,
)

cc_proto_library(
    name = "cc_auth_proto",
    deps = ["//google/api:auth_proto"],
)

cc_grpc_library(
    name = "google_api_auth",
    srcs = [":cc_auth_proto"],
    deps = [":google_api_annotations", ":cc_auth_proto"],
    proto_only = False,
    well_known_protos = True,
    use_external = True,
)

cc_proto_library(
    name = "cc_error_details_proto",
    deps = ["//google/rpc:error_details_proto"],
    )

cc_grpc_library(
    name = "google_rpc_error_details",
    srcs = [":cc_error_details_proto"],
    deps = [":cc_error_details_proto"],
    proto_only = False,
    well_known_protos = True,
    use_external = True,
)

cc_proto_library(
    name = "cc_status_proto",
    deps = ["//google/rpc:status_proto"],
)

cc_grpc_library(
    name = "google_rpc_status",
    srcs = [":cc_status_proto"],
    deps = [":cc_status_proto"],
    proto_only = False,
    well_known_protos = True,
    use_external = True,
)

cc_proto_library(
    name = "cc_operations_proto",
    deps = ["//google/longrunning:operations_proto"],
)

cc_grpc_library(
    name = "google_longrunning_operations",
    srcs = [":cc_operations_proto"],
    deps = [":google_rpc_status", ":google_api_annotations", ":cc_operations_proto"],
    proto_only = False,
    well_known_protos = True,
    use_external = True,
)

cc_proto_library(
    name = "cc_policy_proto",
    deps = ["//google/iam/v1:policy_proto"],
)

cc_grpc_library(
    name = "google_iam_v1_policy",
    srcs = [":cc_policy_proto"],
    deps = [":google_api_annotations", ":cc_policy_proto"],
    proto_only = False,
    well_known_protos = True,
    use_external = True,
)

cc_proto_library(
    name = "cc_iam_policy_proto",
    deps = ["//google/iam/v1:iam_policy_proto"],
)

cc_grpc_library(
    name = "google_iam_v1_iam_policy",
    srcs = [":cc_iam_policy_proto"],
    deps = [":google_api_annotations", ":google_iam_v1_policy", ":cc_iam_policy_proto"],
    proto_only = False,
    well_known_protos = True,
    use_external = True,
)

#cc_grpc_library(
#    name = "google_bigtable_v2_data",
#    srcs = ["google/bigtable/v2/data.proto"],
#    deps = [],
#    proto_only = False,
#    well_known_protos = True,
#    use_external = True,
#)

#cc_grpc_library(
#    name = "google_bigtable_v2_bigtable",
#    srcs = ["google/bigtable/v2/bigtable.proto"],
#    deps = [":google_api_annotations", "google_bigtable_v2_data", ":google_rpc_status"],
#    proto_only = False,
#    well_known_protos = True,
#    use_external = True,
#)

#cc_grpc_library(
#    name = "google_bigtable_admin_v2_table",
#    srcs = ["google/bigtable/admin/v2/table.proto"],
#    deps = [":google_api_annotations"],
#    proto_only = False,
#    well_known_protos = True,
#    use_external = True,
#)

#cc_grpc_library(
#    name = "google_bigtable_admin_v2_instance",
#    srcs = ["google/bigtable/admin/v2/instance.proto"],
#    deps = [":google_api_annotations", ":google_bigtable_admin_v2_common"],
#    proto_only = False,
#    well_known_protos = True,
#    use_external = True,
#)

#cc_grpc_library(
#    name = "google_bigtable_admin_v2_common",
#    srcs = ["google/bigtable/admin/v2/common.proto"],
#    deps = [":google_api_annotations"],
#    proto_only = False,
#    well_known_protos = True,
#    use_external = True,
#)

#cc_grpc_library(
#    name = "google_bigtable_admin_v2_bigtable_table_admin",
#    srcs = ["google/bigtable/admin/v2/bigtable_table_admin.proto"],
#    deps = [
#        ":google_api_annotations",
#        ":google_longrunning_operations",
#        ":google_bigtable_admin_v2_table",
#    ],
#    proto_only = False,
#    well_known_protos = True,
#    use_external = True,
#)

#cc_grpc_library(
#    name = "google_bigtable_admin_v2_bigtable_instance_admin",
#    srcs = ["google/bigtable/admin/v2/bigtable_instance_admin.proto"],
#    deps = [
#        ":google_api_annotations",
#        ":google_longrunning_operations",
#        ":google_iam_v1_iam_policy",
#        ":google_bigtable_admin_v2_instance",
#    ],
#    proto_only = False,
#    well_known_protos = True,
#    use_external = True,
#)

#cc_library(
#    name = "bigtable_protos",
#    deps = [
#        ":google_api_http",
#        ":google_api_auth",
#        ":google_api_annotations",
#        ":google_rpc_error_details",
#        ":google_rpc_status",
#        ":google_longrunning_operations",
#        ":google_iam_v1_policy",
#        ":google_iam_v1_iam_policy",
#        ":google_bigtable_v2_data",
#        ":google_bigtable_v2_bigtable",
#        ":google_bigtable_admin_v2_common",
#        ":google_bigtable_admin_v2_table",
#        ":google_bigtable_admin_v2_instance",
#        ":google_bigtable_admin_v2_bigtable_table_admin",
#        ":google_bigtable_admin_v2_bigtable_instance_admin",
#        "@com_github_grpc_grpc//:grpc++",
#    ],
#    includes = ["."],
#)

######################
# BEGIN SPANNER PROTOS
######################

# WARNING:
# These Spanner protos exist for prototyping only. Users should not rely on
# these protos and bazel build targets existing in the future. We will
# remove/rename these without notice.

cc_proto_library(
    name = "cc_spanner_proto",
    deps = ["//google/spanner/v1:spanner_proto"],
)

cc_grpc_library(
    name = "grpc_spanner_proto",
    srcs = [":cc_spanner_proto"],
    deps = [":cc_spanner_proto"],
    proto_only = False,
    well_known_protos = True,
    use_external = True,
)    

#cc_grpc_library(
#    name = "google_spanner_v1_keys",
#    srcs = ["google/spanner/v1/keys.proto"],
#    deps = [":google_api_annotations"],
#    proto_only = False,
#    well_known_protos = True,
#    use_external = True,
#)

#cc_grpc_library(
#    name = "google_spanner_v1_mutation",
#    srcs = ["google/spanner/v1/mutation.proto"],
#    deps = [
#        ":google_api_annotations",
#        ":google_spanner_v1_keys",
#    ],
#    proto_only = False,
#    well_known_protos = True,
#    use_external = True,
#)
#
#cc_grpc_library(
#    name = "google_spanner_v1_query_plan",
#    srcs = ["google/spanner/v1/query_plan.proto"],
#    deps = [
#        ":google_api_annotations",
#    ],
#    proto_only = False,
#    well_known_protos = True,
#    use_external = True,
#)
#
#cc_grpc_library(
#    name = "google_spanner_v1_type",
#    srcs = ["google/spanner/v1/type.proto"],
#    deps = [
#        ":google_api_annotations",
#    ],
#    proto_only = False,
#    well_known_protos = True,
#    use_external = True,
#)
#
#cc_grpc_library(
#    name = "google_spanner_v1_transaction",
#    srcs = ["google/spanner/v1/transaction.proto"],
#    deps = [
#        ":google_api_annotations",
#    ],
#    proto_only = False,
#    well_known_protos = True,
#    use_external = True,
#)
#
#cc_grpc_library(
#    name = "google_spanner_v1_result_set",
#    srcs = ["google/spanner/v1/result_set.proto"],
#    deps = [
#        ":google_api_annotations",
#        ":google_spanner_v1_query_plan",
#        ":google_spanner_v1_transaction",
#        ":google_spanner_v1_type",
#    ],
#    proto_only = False,
#    well_known_protos = True,
#    use_external = True,
#)
#
#cc_grpc_library(
#    name = "google_spanner_v1_spanner",
#    srcs = ["google/spanner/v1/spanner.proto"],
#    deps = [
#        ":google_api_annotations",
#        ":google_spanner_v1_keys",
#        ":google_spanner_v1_mutation",
#        ":google_spanner_v1_result_set",
#        ":google_spanner_v1_transaction",
#        ":google_spanner_v1_type",
#    ],
#    proto_only = False,
#    well_known_protos = True,
#    use_external = True,
#)
#

cc_proto_library(
    name = "cc_spanner_admin_instance_proto",
    deps = ["//google/spanner/admin/instance/v1:spanner_admin_instance_proto"],
    )

# Instance Admin
cc_grpc_library(
    name = "google_spanner_admin_instance_v1_spanner_instance_admin",
    srcs = [":cc_spanner_instance_admin_proto"],
    deps = [
        ":cc_annotations_proto",
        ":cc_operations_proto",
        ":cc_iam_policy_proto",
    ],
    proto_only = False,
    well_known_protos = True,
    use_external = True,
)

cc_proto_library(
    name = "cc_spanner_admin_database_proto",
    deps = ["//google/spanner/admin/database/v1:spanner_admin_database_proto"],
)
## Database Admin
cc_grpc_library(
    name = "google_spanner_admin_database_v1_spanner_database_admin",
    srcs = [":cc_spanner_database_admin_proto"],
    deps = [
        ":cc_annotations_proto",
        ":cc_operations_proto",
        ":cc_iam_policy_proto",
    ],
    proto_only = False,
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

#cc_library(
#    name = "spanner_protos",
#    deps = [
#        ":google_spanner_v1_keys",
#        ":google_spanner_v1_mutation",
#        ":google_spanner_v1_query_plan",
#        ":google_spanner_v1_result_set",
#        ":google_spanner_v1_spanner",
#        ":google_spanner_v1_transaction",
#        ":google_spanner_v1_type",
#        ":google_spanner_admin_instance_v1_spanner_instance_admin",
#        ":google_spanner_admin_database_v1_spanner_database_admin",
#        "@com_github_grpc_grpc//:grpc++",
#    ],
#    includes = ["."],
#)
#
####################
# END SPANNER PROTOS
####################
