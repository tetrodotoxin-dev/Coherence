// # Tetrodotoxin
// Copyright (c) 2023-present Matt Kaes and contributors

#pragma once

#include "perimortem/core/identity/uuid.hpp"
#include "perimortem/core/storage/vector.hpp"

#include "coherence/type_system/description.hpp"
#include "coherence/type_system/reference.hpp"
#include "coherence/type_system/routing.hpp"
#include "ttx/graph/abstract.hpp"
#include "ttx/graph/projection.hpp"

namespace Coherence::TypeSystem {

// Field describes one occurrence of member storage and the Type of value held
// there. Repeated Fields remain distinct declarations even when they select
// the same Type. Routing::type returns that shared Type graph location.
//
// Field contains no offset or physical form. The compiler assigns layout from
// the complete ordered field sequence of the containing Type.
class Field
    : public Ttx::Graph::
          Projection<Field, 0x56e0065fbbf44ea6ULL, 0xa1063dd2ffe2ff4cULL> {
 public:
  // Reference stores either a native Field projection or a graph interface
  // supplied by another provider. The selected interface must remain
  // accessible throughout the containing value's published access window.
  using Reference = TypeSystem::Reference<Abstract>;

  inline static constexpr Perimortem::Core::Storage::
      Vector<Perimortem::Core::Identity::Uuid, 1>
          supported_contracts = {{
            Ttx::Graph::Capabilities::Describe::contract_id,
          }};

  static constexpr auto create(
      Description description,
      Ttx::Graph::Abstract type) -> Field {
    return Field(description, type);
  }

  constexpr auto get_name() const -> Perimortem::Core::View::Bytes {
    return description.get_name();
  }

  constexpr auto get_description() const -> Perimortem::Core::View::Bytes {
    return description.get_documentation();
  }

  constexpr auto get_type() const -> Ttx::Graph::Abstract { return type; }

  auto bind_contract(Perimortem::Core::Identity::Uuid contract, void* output)
      -> Ttx::Semantic::Negotiation::Status {
    return description.bind(*this, contract, output);
  }

  EXPORTED(COHERENCE) auto resolve_route(Perimortem::Core::View::Bytes route)
      -> Ttx::Graph::Abstract;

  EXPORTED(COHERENCE) auto visit_routes(Ttx::Graph::Abstract::Visitor visitor)
      -> void;

 private:
  constexpr Field(Description description, Ttx::Graph::Abstract type)
      : description(description), type(type) {}

  Description description;
  Ttx::Graph::Abstract type;
};

}  // namespace Coherence::TypeSystem
