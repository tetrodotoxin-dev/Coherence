// # Tetrodotoxin
// Copyright (c) 2023-present Matt Kaes and contributors

#include "coherence/type_system/primitive.hpp"

#include "perimortem/core/data.hpp"

using namespace Perimortem::Core;
using namespace Coherence::TypeSystem;

static auto primitive_abstract(void* context) -> ttx_abstract {
  return Data::cast<Primitive>(context)->get_abstract().get_abi();
}

static auto primitive_type(void* context) -> const void* {
  return &Data::cast<Primitive>(context)->get_type();
}

static auto primitive_fields(
    void* context,
    void* callback_context,
    void (*callback)(void*, ttx_abstract)) -> void {
  (void)context;
  (void)callback_context;
  (void)callback;
}

static auto primitive_callables(
    void* context,
    void* callback_context,
    void (*callback)(void*, ttx_abstract)) -> void {
  (void)context;
  (void)callback_context;
  (void)callback;
}

static auto primitive_schema(void* context) -> ttx_schema_reference {
  return Data::cast<Primitive>(context)->get_schema();
}

auto Primitive::bind_contract(Identity::Uuid contract, void* output)
    -> Ttx::Semantic::Negotiation::Status {
  if (contract == Capabilities::Schema::contract_id) {
    static constexpr coherence_schema_ops operations = {
      primitive_abstract,
      primitive_schema,
    };

    coherence_schema api;
    api.context = this;
    api.operations = &operations;
    *Data::cast<coherence_schema>(output) = api;
    return Ttx::Semantic::Negotiation::Status::Satisfied;
  }

  if (contract == Type::Contract::contract_id) {
    static constexpr coherence_type_ops operations = {
      primitive_abstract,
      primitive_fields,
      primitive_callables,
    };

    coherence_type api;
    api.context = this;
    api.operations = &operations;
    *Data::cast<coherence_type>(output) = api;
    return Ttx::Semantic::Negotiation::Status::Satisfied;
  }

  if (contract == Type::Abstract::contract_id) {
    static constexpr Type::Abstract::Operations operations(
        primitive_type, primitive_abstract);
    const Type::Abstract::Api api(this, operations);
    *Data::cast<Type::Abstract::Api>(output) = api;
    return Ttx::Semantic::Negotiation::Status::Satisfied;
  }

  return description.bind(*this, contract, output);
}
