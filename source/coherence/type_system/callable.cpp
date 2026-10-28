// # Tetrodotoxin
// Copyright (c) 2023-present Matt Kaes and contributors

#include "coherence/type_system/callable.hpp"

#include "perimortem/core/data.hpp"

#include "coherence/type_system/field.hpp"
#include "coherence/type_system/type.hpp"

using namespace Perimortem::Core;
using namespace Ttx::Graph;
using namespace Coherence::TypeSystem;

static auto callable_abstract(void* context) -> ttx_abstract {
  return Data::cast<Callable>(context)->get_abstract().get_abi();
}

static auto callable_arguments(
    void* context,
    void* callback_context,
    void (*callback)(void*, ttx_abstract)) -> void {
  const auto arguments = Data::cast<Callable>(context)->get_arguments();
  for (Count index = 0; index < arguments.get_size(); ++index) {
    callback(
        callback_context, arguments.get_data()[index].get_abstract().get_abi());
  }
}

static auto callable_result(void* context) -> ttx_abstract {
  return Data::cast<Callable>(context)->get_result().get_abi();
}

static auto callable_operation(void* context) -> coherence_operation {
  const auto operation = Data::cast<Callable>(context)->get_operation();
  coherence_operation api;
  api.context = operation.get_context();
  api.adapter = operation.get_adapter();
  return api;
}

auto Callable::bind_contract(Identity::Uuid contract, void* output)
    -> Ttx::Semantic::Negotiation::Status {
  if (contract == Contract::contract_id) {
    static constexpr coherence_callable_ops operations = {
      callable_abstract,
      callable_arguments,
      callable_result,
      callable_operation,
    };

    coherence_callable api;
    api.context = this;
    api.operations = &operations;
    *Data::cast<coherence_callable>(output) = api;
    return Ttx::Semantic::Negotiation::Status::Satisfied;
  }

  return description.bind(*this, contract, output);
}
