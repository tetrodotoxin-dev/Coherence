// # Tetrodotoxin
// Copyright (c) 2023-present Matt Kaes and contributors

#pragma once

#include "coherence/type_system/operation.hpp"
#include "ttx/data/form/representation.hpp"
#include "ttx/semantic/realization/invocation.hpp"

namespace Coherence::Compilation {

// Executable runs one Callable through input and output Representations derived
// from its Type graph. The forms define the exact call storage accepted by the
// provider's terminal adapter. Compilation refuses an incomplete signature
// before this object can be created.
//
// Executable owns both Representation descriptors and creates an Invocation
// only for the duration of `accepts` or `invoke`. No pointer into those owned
// descriptions escapes, so copying and moving preserve a usable value. The
// Operation's provider keeps its context and executable code accessible
// through every call.
class Executable {
 public:
  auto get_inputs() const -> const Ttx::Data::Form::Representation& {
    return inputs;
  }

  auto get_output() const -> const Ttx::Data::Form::Representation& {
    return output;
  }

  auto accepts(
      const Ttx::Data::Form::Representation& supplied_inputs,
      const Ttx::Data::Form::Representation& supplied_output) const -> Bool {
    return create_invocation().accepts(supplied_inputs, supplied_output);
  }

  auto invoke(const void* input, void* output) const -> Ttx::Data::Status {
    return create_invocation().invoke(input, output);
  }

  static auto create(
      Ttx::Data::Form::Representation&& inputs,
      Ttx::Data::Form::Representation&& output,
      TypeSystem::Operation operation) -> Executable {
    return Executable(
        Perimortem::Core::Data::take(inputs),
        Perimortem::Core::Data::take(output), operation);
  }

 private:
  auto create_invocation() const -> Ttx::Semantic::Realization::Invocation {
    ttx_invocation api;
    api.context = operation.get_context();
    api.inputs = &inputs.get_abi();
    api.outputs = &output.get_abi();
    api.invoke = operation.get_adapter();
    return Ttx::Semantic::Realization::Invocation(api);
  }

  Executable(
      Ttx::Data::Form::Representation&& inputs,
      Ttx::Data::Form::Representation&& output,
      TypeSystem::Operation operation)
      : inputs(Perimortem::Core::Data::take(inputs)),
        output(Perimortem::Core::Data::take(output)),
        operation(operation) {}

  Ttx::Data::Form::Representation inputs;
  Ttx::Data::Form::Representation output;
  TypeSystem::Operation operation;
};

}  // namespace Coherence::Compilation
