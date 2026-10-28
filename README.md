# Coherence

Coherence provides semantic identity systems for independent tools and runtimes
using [TTX](https://github.com/tetrodotoxin-dev/TTX).

TTX brings portable data forms and contract negotiation into a semantic graph.
Coherence uses that foundation to give related graph locations shared identity
and structure. Providers and consumers can work from the same model while
keeping their own languages and representations.

## Type System

The [Type System](source/coherence/type_system) module provides a common base
for representing type systems inside a TTX graph. It captures the identities
and relationships shared by most type systems, while policies and domain
behavior remain explicit TTX contracts. Tools can extend the model while
preserving the common structure used for interoperability.

## Compilation

The optional [Compilation](source/coherence/compilation) module derives
canonical TTX representations and executable calls from a type graph. Projects
can use the type system on its own and add compilation when they need a native
ABI.

## Building and testing

Python 3 and the Bazel version in [.bazelversion](.bazelversion) are required.
Bazel acquires the build dependencies declared in [MODULE.bazel](MODULE.bazel).

Run these commands from the repository root:

```sh
bazel run :format -- --check
bazel build //source/...
bazel run :tests
bazel run :benchmarks --config=release
bazel build :sdk --config=release
bazel run :snapshot
```

Tests and benchmarks are standalone executables. The SDK packages headers and
static and shared libraries for independently built consumers. A snapshot
archives the clean Git commit that reproduces the source and validation state.

Generate the VS Code configuration with `bazel run :vscode`.

Coherence is available under the [MIT license](LICENSE).
