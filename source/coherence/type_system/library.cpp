// # Tetrodotoxin
// Copyright (c) 2023-present Matt Kaes and contributors

#include "coherence/type_system/library.hpp"

#include "coherence/type_system/reference.hpp"
#include "ttx/graph/policies/none.hpp"

using namespace Perimortem::Core;
using namespace Ttx::Graph;
using namespace Coherence::TypeSystem;

auto Library::resolve_route(View::Bytes route) -> Ttx::Graph::Abstract {
  for (Count index = 0; index < entries.get_size(); ++index) {
    const auto& entry = entries.get_data()[index];
    if (entry.get_route() == route) {
      return entry.get_type().get_abstract();
    }
  }

  return Ttx::Graph::Policies::None::get_none().get_abstract();
}

auto Library::visit_routes(Ttx::Graph::Abstract::Visitor visitor) -> void {
  for (Count index = 0; index < entries.get_size(); ++index) {
    const auto& entry = entries.get_data()[index];
    visitor(entry.get_route(), entry.get_type().get_abstract());
  }
}
