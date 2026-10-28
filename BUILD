# Copyright (c) 2023-present Matt Kaes and contributors

load(
    "@tetro_toolchain//:defs.bzl",
    "format",
    "package",
    "snapshot",
    "validation",
    "vscode",
)

package(
    components = {
        "compilation": [
            "type_system",
        ],
        "type_system": [],
    },
    linkage = "shared",
    module = "coherence",
    shared = "//source:coherence_shared",
    static = "//source:coherence_static",
)

format()

snapshot()

validation(deps = [":coherence"])

vscode()
