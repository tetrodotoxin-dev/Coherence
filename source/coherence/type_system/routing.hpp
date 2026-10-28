// # Tetrodotoxin
// Copyright (c) 2023-present Matt Kaes and contributors

#pragma once

#include "perimortem/core/view/bytes.hpp"

namespace Coherence::TypeSystem {

// Routing defines the public byte sequences used to navigate Coherence type
// graphs. Providers and consumers use these values instead of independently
// spelling route names. Each route documents the selection it requests and any
// policy that remains separately negotiated.
class Routing {
  inline static constexpr U8 type_data[] = {'t', 'y', 'p', 'e'};

 public:
  // Type selects the value Type described by a Field declaration. The returned
  // Abstract keeps its own provider context and contracts. Repeated observation
  // stability remains a separate Constant policy supplied by the provider.
  inline static constexpr Perimortem::Core::View::Bytes type = type_data;
};

}  // namespace Coherence::TypeSystem
