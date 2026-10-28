# Copyright (c) 2023-present Matt Kaes and contributors

load("@tetro_toolchain//:defs.bzl", "WINDOWS", "library")

package(default_visibility = ["//visibility:public"])

_COMPONENTS = {
    "compilation": [
        "type_system",
    ],
    "type_system": [],
}

_COMPONENT_DEPS = {
    "type_system": [
        "@perimortem//:core",
        "@perimortem//:memory",
        "@ttx//:graph",
    ],
}

_LIBRARY = dict(
    srcs = glob(["coherence/**/*.cpp"]),
    hdrs = glob([
        "coherence/**/*.h",
        "coherence/**/*.hpp",
    ]),
    includes = ["."],
)

library(
    name = "coherence_static",
    srcs = _LIBRARY["srcs"],
    hdrs = _LIBRARY["hdrs"],
    component_deps = _COMPONENT_DEPS,
    components = _COMPONENTS,
    includes = _LIBRARY["includes"],
    module = "coherence",
    deps = [
        "@perimortem_static//:core",
        "@perimortem_static//:memory",
        "@ttx//source:ttx_static",
    ],
)

library(
    name = "coherence_shared",
    srcs = _LIBRARY["srcs"],
    hdrs = _LIBRARY["hdrs"],
    includes = _LIBRARY["includes"],
    linkage = "shared",
    module = "coherence",
    shared_lib_name = select({
        WINDOWS: "coherence.dll",
        "//conditions:default": "libcoherence.so",
    }),
    deps = [
        "@perimortem//:core",
        "@perimortem//:memory",
        "@ttx//:graph",
    ],
)
