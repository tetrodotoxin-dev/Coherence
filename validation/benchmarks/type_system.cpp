// # Tetrodotoxin
// Copyright (c) 2023-present Matt Kaes and contributors

#include "toolchain/validation/benchmark.hpp"

#include "perimortem/core/diagnostics/log.hpp"
#include "perimortem/core/view/null_terminated.hpp"

#include "coherence/type_system/type.hpp"

using namespace Perimortem::Core;
using namespace Toolchain::Validation;
using namespace Coherence::TypeSystem;

static Harness TypeSystemBenchmarks = {.name = "Coherence::TypeSystem"};

static auto type = Type::create(
    Description::create("Value"_view, "A benchmark Type."_view),
    View::Vector<Type::FieldReference>(),
    View::Vector<Type::CallableReference>());

// Each iteration performs the complete UUID binding and native typed recovery
// from an Abstract. This measures the generated projection path used when a C++
// consumer crosses into the graph and returns to a concrete value.
VALIDATION_BENCHMARK(TypeSystemBenchmarks, native_projection) {
  const auto abstract = type.get_abstract();
  for (Count index = 0; index < 1024; ++index) {
    auto projected = Type::Abstract::convert(abstract);
    if (!projected) {
      Diagnostics::Log::fatal("Type projection failed."_view);
    }
  }
}
