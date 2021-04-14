load(
    "@envoy//bazel:envoy_build_system.bzl",
    "envoy_cc_binary",
    "envoy_cc_library",
    "envoy_cc_test",
)
load("@envoy_api//bazel:api_build_system.bzl", "api_proto_package")

package(default_visibility = ["//visibility:public"])

envoy_cc_binary(
    name = "envoy",
    repository = "@envoy",
    deps = [
        ":ext_policy_config",
        "@envoy//source/exe:envoy_main_entry_lib",
    ],
)

api_proto_package(
    deps = [
        "@envoy_api//envoy/config/core/v3:pkg",
        "@envoy_api//envoy/type/v3:pkg",
        "@com_github_cncf_udpa//udpa/annotations:pkg",
    ],
)

envoy_cc_library(
    name = "ext_policy_config",
    srcs = ["config.cc"],
    hdrs = ["config.h"],
    repository = "@envoy",
    deps = [
        ":ext_policy",
        ":client_lib",
        "@envoy//source/extensions/filters/http/common:factory_base_lib",
        "@envoy//include/envoy/network:filter_interface",
        "@envoy//include/envoy/registry:registry",
        "@envoy//include/envoy/server:filter_config_interface",
    ],
)


envoy_cc_library(
    name = "ext_policy",
    srcs = ["ext_policy.cc"],
    hdrs = ["ext_policy.h"],
    repository = "@envoy",
    deps = [
        "@envoy//source/extensions/filters/http/common:pass_through_filter_lib",
        ":client_interface",
        ":mutation_utils_lib"
    ],
)

envoy_cc_library(
    name = "client_interface",
    hdrs = ["client.h"],
    repository = "@envoy",
    deps = [
        ":pkg_cc_proto",
    ],
)

envoy_cc_library(
    name = "client_lib",
    srcs = ["client_impl.cc"],
    hdrs = ["client_impl.h"],
    repository = "@envoy",
    deps = [
        ":client_interface",
        "@envoy//include/envoy/grpc:async_client_interface",
        "@envoy//include/envoy/stats:stats_interface",
        "@envoy//include/envoy/upstream:cluster_manager_interface",
        "@envoy//source/common/grpc:typed_async_client_lib",
        "@envoy_api//envoy/config/core/v3:pkg_cc_proto",
        ":pkg_cc_proto",
    ],
)

envoy_cc_library(
    name = "mutation_utils_lib",
    srcs = ["mutation_utils.cc"],
    hdrs = ["mutation_utils.h"],
    repository = "@envoy",
    deps = [
        "@envoy//include/envoy/buffer:buffer_interface",
        "@envoy//include/envoy/http:header_map_interface",
        "@envoy//source/common/http:header_utility_lib",
        "@envoy//source/common/protobuf:utility_lib",
        ":pkg_cc_proto",
    ],
)