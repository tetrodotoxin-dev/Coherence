// # Tetrodotoxin
// Copyright (c) 2023-present Matt Kaes and contributors

#pragma once

#include "perimortem/core/identity/uuid.hpp"
#include "perimortem/core/storage/vector.hpp"

#include "coherence/type_system/capabilities/schema.hpp"
#include "coherence/type_system/description.hpp"
#include "coherence/type_system/type.hpp"
#include "ttx/data/form/schema.hpp"
#include "ttx/graph/projection.hpp"

namespace Coherence::TypeSystem {

// Primitive defines a Type with one terminal TTX Schema. Native C++ consumers
// receive the ordinary immutable Type interface. The compiler binds Schema at
// the same graph location to derive the physical form without adding physical
// storage rules to Type.
//
// The referenced Schema graph remains stable through compilation. Primitive
// and its materialized Type own no source descriptions or canonical output
// storage.
//
// Primitive is also a simulacrum for its materialized native Type. Binding
// Type::Abstract returns that contained Type, while `get_abstract` reenters the
// Primitive graph location that supplied it. The two records can use different
// contexts. Reentry preserves the semantic location rather than record pointer
// identity. Consumers preserve each context with its published operations.
class Primitive
    : public Ttx::Graph::
          Projection<Primitive, 0x9c30f068067c4d71ULL, 0xbb67418be6c53e5bULL> {
 public:
  inline static constexpr Perimortem::Core::Storage::
      Vector<Perimortem::Core::Identity::Uuid, 4>
          supported_contracts = {{
            Ttx::Graph::Capabilities::Describe::contract_id,
            Capabilities::Schema::contract_id,
            Type::Contract::contract_id,
            Type::Abstract::contract_id,
          }};

  static constexpr auto create(
      Description description,
      Ttx::Data::Form::Schema::Reference schema) -> Primitive {
    return Primitive(description, schema);
  }

  constexpr auto get_name() const -> Perimortem::Core::View::Bytes {
    return description.get_name();
  }

  constexpr auto get_description() const -> Perimortem::Core::View::Bytes {
    return description.get_documentation();
  }

  constexpr auto get_schema() const -> Ttx::Data::Form::Schema::Reference {
    return schema;
  }

  constexpr auto get_type() const -> const Type& { return type; }

  EXPORTED(COHERENCE) auto bind_contract(
      Perimortem::Core::Identity::Uuid contract,
      void* output) -> Ttx::Semantic::Negotiation::Status;

 private:
  constexpr Primitive(
      Description description,
      Ttx::Data::Form::Schema::Reference schema)
      : description(description),
        schema(schema),
        type(
            Type::create(
                description,
                Perimortem::Core::View::Vector<Type::FieldReference>(),
                Perimortem::Core::View::Vector<Type::CallableReference>())) {}

  Description description;
  Ttx::Data::Form::Schema::Reference schema;
  Type type;
};

}  // namespace Coherence::TypeSystem
