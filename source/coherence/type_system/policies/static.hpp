// # Tetrodotoxin
// Copyright (c) 2023-present Matt Kaes and contributors

#pragma once

#include "perimortem/core/identity/uuid.hpp"

#include "coherence/type_system/policies/static.h"

namespace Coherence::TypeSystem::Policies {

// Static identifies a Callable with no receiver input. Its Operation can be a
// free function or use private provider state through the Operation context.
// The policy is reported through Abstract::supports and has no API record to
// bind.
class Static {
 public:
  static constexpr auto contract_id = Perimortem::Core::Identity::Uuid(
      COHERENCE_STATIC_ID_HIGH,
      COHERENCE_STATIC_ID_LOW);
};

}  // namespace Coherence::TypeSystem::Policies
