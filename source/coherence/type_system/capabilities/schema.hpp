// # Tetrodotoxin
// Copyright (c) 2023-present Matt Kaes and contributors

#pragma once

#include "perimortem/core/identity/uuid.hpp"

#include "coherence/type_system/capabilities/schema.h"
#include "ttx/data/form/schema.hpp"
#include "ttx/graph/abstract.hpp"

namespace Coherence::TypeSystem::Capabilities {

// Schema exposes a terminal source description for type compilation. The
// returned reference follows the synchronous validity contract in schema.h.
class Schema {
 public:
  using Api = coherence_schema;
  static constexpr auto contract_id = Perimortem::Core::Identity::Uuid(
      COHERENCE_SCHEMA_ID_HIGH,
      COHERENCE_SCHEMA_ID_LOW);

  explicit constexpr Schema(Api api) : api(api) {}

  constexpr auto get_abi() const -> Api { return api; }

  auto get_abstract() const -> Ttx::Graph::Abstract {
    return Ttx::Graph::Abstract(api.operations->get_abstract(api.context));
  }

  auto get_schema() const -> Ttx::Data::Form::Schema::Reference {
    return Ttx::Data::Form::Schema::Reference(
        api.operations->get_schema(api.context));
  }

 private:
  Api api;
};

}  // namespace Coherence::TypeSystem::Capabilities
