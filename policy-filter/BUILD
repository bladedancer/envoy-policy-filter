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
        ":policy_filter_config",
        "@envoy//source/exe:envoy_main_entry_lib",
    ],
)

api_proto_package()

envoy_cc_library(
    name = "policy_filter_lib",
    srcs = ["policy_filter.cc"],
    hdrs = ["policy_filter.h"],
    repository = "@envoy",
    deps = [
        ":pkg_cc_proto",
        "@envoy//source/extensions/filters/http/common:pass_through_filter_lib",
    ],
)

envoy_cc_library(
    name = "policy_filter_config",
    srcs = ["policy_filter_config.cc"],
    repository = "@envoy",
    deps = [
        ":policy_filter_lib",
        "@envoy//include/envoy/server:filter_config_interface",
    ],
)

envoy_cc_test(
    name = "policy_filter_integration_test",
    srcs = ["policy_filter_integration_test.cc"],
    repository = "@envoy",
    deps = [
        ":policy_filter_config",
        "@envoy//test/integration:http_integration_lib",
    ],
)
