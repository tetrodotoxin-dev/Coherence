// # Tetrodotoxin
// Copyright (c) 2023-present Matt Kaes and contributors

#pragma once

#include "perimortem/core/identity/uuid.hpp"

#include "coherence/type_system/policies/member.h"

namespace Coherence::TypeSystem::Policies {

// Member identifies a Callable whose containing Type is the leading receiver
// input. The policy is reported through Abstract::supports and has no API
// record to bind.
class Member {
 public:
  static constexpr auto contract_id = Perimortem::Core::Identity::Uuid(
      COHERENCE_MEMBER_ID_HIGH,
      COHERENCE_MEMBER_ID_LOW);
};

}  // namespace Coherence::TypeSystem::Policies
