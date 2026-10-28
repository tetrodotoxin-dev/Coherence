// # Tetrodotoxin
// Copyright (c) 2023-present Matt Kaes and contributors

#include "coherence/type_system/field.hpp"

#include "coherence/type_system/routing.hpp"
#include "ttx/graph/policies/none.hpp"

using namespace Perimortem::Core;
using namespace Ttx::Graph;
using namespace Coherence::TypeSystem;

auto Field::resolve_route(View::Bytes route) -> Ttx::Graph::Abstract {
  return route == Routing::type ? type
                                : Policies::None::get_none().get_abstract();
}

auto Field::visit_routes(Ttx::Graph::Abstract::Visitor visitor) -> void {
  visitor(Routing::type, type);
}
