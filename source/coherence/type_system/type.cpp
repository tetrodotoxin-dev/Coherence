// # Tetrodotoxin
// Copyright (c) 2023-present Matt Kaes and contributors

#include "coherence/type_system/type.hpp"

#include "perimortem/core/data.hpp"

#include "coherence/type_system/callable.hpp"
#include "coherence/type_system/field.hpp"

using namespace Perimortem::Core;
using namespace Ttx::Graph;
using namespace Coherence::TypeSystem;

static auto type_abstract(void* context) -> ttx_abstract {
  return Data::cast<Type>(context)->get_abstract().get_abi();
}

static auto type_fields(
    void* context,
    void* callback_context,
    void (*callback)(void*, ttx_abstract)) -> void {
  const auto fields = Data::cast<Type>(context)->get_fields();
  for (Count index = 0; index < fields.get_size(); ++index) {
    callback(
        callback_context, fields.get_data()[index].get_abstract().get_abi());
  }
}

static auto type_callables(
    void* context,
    void* callback_context,
    void (*callback)(void*, ttx_abstract)) -> void {
  const auto callables = Data::cast<Type>(context)->get_callables();
  for (Count index = 0; index < callables.get_size(); ++index) {
    callback(
        callback_context, callables.get_data()[index].get_abstract().get_abi());
  }
}

auto Type::bind_contract(Identity::Uuid contract, void* output)
    -> Ttx::Semantic::Negotiation::Status {
  if (contract == Contract::contract_id) {
    static constexpr coherence_type_ops operations = {
      type_abstract,
      type_fields,
      type_callables,
    };

    coherence_type api;
    api.context = this;
    api.operations = &operations;
    *Data::cast<coherence_type>(output) = api;
    return Ttx::Semantic::Negotiation::Status::Satisfied;
  }

  return description.bind(*this, contract, output);
}
