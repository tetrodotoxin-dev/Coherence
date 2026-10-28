// # Tetrodotoxin
// Copyright (c) 2023-present Matt Kaes and contributors

#pragma once

#include "perimortem/core/storage/union.hpp"

#include "ttx/graph/abstract.hpp"
#include "ttx/graph/policies/unknown.h"

namespace Coherence::TypeSystem {

// Reference stores either one native C++ projection or an Abstract supplied by
// another provider. The selected record keeps its own context and operations.
// Reference adds no C ABI and does not require a foreign provider to expose a
// C++ object. Default construction supplies Unknown because no graph location
// or explicit dead end was selected.
template <typename Projection>
class Reference {
 public:
  constexpr Reference() = default;
  explicit constexpr Reference(Projection projection) : value(projection) {}
  explicit constexpr Reference(Ttx::Graph::Abstract abstract)
      : value(abstract) {}

  constexpr auto get_abstract() const -> Ttx::Graph::Abstract {
    return value.visit(
        [](const Projection& projection) { return projection.get_abstract(); },
        [](const Ttx::Graph::Abstract& abstract) { return abstract; },
        [] { return Ttx::Graph::Abstract(ttx_unknown()); });
  }

  template <typename... Visitors>
  constexpr auto visit(Visitors... visitors) const -> decltype(auto) {
    return value.visit(visitors...);
  }

 private:
  Perimortem::Core::Storage::Union<Projection, Ttx::Graph::Abstract> value;
};

}  // namespace Coherence::TypeSystem
