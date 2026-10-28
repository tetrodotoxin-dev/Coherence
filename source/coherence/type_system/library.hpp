// # Tetrodotoxin
// Copyright (c) 2023-present Matt Kaes and contributors

#pragma once

#include "perimortem/core/identity/uuid.hpp"
#include "perimortem/core/storage/option.hpp"
#include "perimortem/core/storage/vector.hpp"
#include "perimortem/core/view/vector.hpp"

#include "coherence/type_system/description.hpp"
#include "coherence/type_system/type.hpp"
#include "ttx/graph/projection.hpp"

namespace Coherence::TypeSystem {

// Library is a named graph location that exposes Types through explicit routes.
// Route identity belongs to membership in this Library rather than to a Type's
// presentation metadata. Construction refuses repeated routes, so resolution
// selects at most one entry. Different Libraries can assign different routes
// to the same Type or reuse a route in their independent namespaces.
//
// The Entry View, every route and every projected Type remain accessible
// throughout the Library's published access window. Entries use ordinary
// Abstracts, so one Library can compose Types from different runtimes. Library
// owns no routes or Type values.
class Library
    : public Ttx::Graph::
          Projection<Library, 0xf0a4e97b331d4d42ULL, 0xa01b9fa0cd1b09d9ULL> {
 public:
  // Entry assigns one route to a Type reference. The route need not match the
  // Type's described name. Its bytes remain readable and unchanged throughout
  // the containing Library's published access window. The selected Type
  // reference retains the access established by its native projection or graph
  // provider. Entry adds no ownership to either source.
  class Entry {
   public:
    static constexpr auto create(
        Perimortem::Core::View::Bytes route,
        Type::Reference type) -> Entry {
      return Entry(route, type);
    }

    constexpr auto get_route() const -> Perimortem::Core::View::Bytes {
      return route;
    }

    constexpr auto get_type() const -> const Type::Reference& { return type; }

   private:
    constexpr Entry(Perimortem::Core::View::Bytes route, Type::Reference type)
        : route(route), type(type) {}

    Perimortem::Core::View::Bytes route;
    Type::Reference type;
  };

  inline static constexpr Perimortem::Core::Storage::
      Vector<Perimortem::Core::Identity::Uuid, 1>
          supported_contracts = {{
            Ttx::Graph::Capabilities::Describe::contract_id,
          }};

  static constexpr auto create(
      Description description,
      Perimortem::Core::View::Vector<Entry> entries)
      -> Perimortem::Core::Storage::Option<Library> {
    for (Count index = 0; index < entries.get_size(); ++index) {
      for (Count previous = 0; previous < index; ++previous) {
        if (entries.get_data()[index].get_route() ==
            entries.get_data()[previous].get_route()) {
          return {};
        }
      }
    }

    return Library(description, entries);
  }

  constexpr auto get_name() const -> Perimortem::Core::View::Bytes {
    return description.get_name();
  }

  constexpr auto get_description() const -> Perimortem::Core::View::Bytes {
    return description.get_documentation();
  }

  constexpr auto get_entries() const -> Perimortem::Core::View::Vector<Entry> {
    return entries;
  }

  auto bind_contract(Perimortem::Core::Identity::Uuid contract, void* output)
      -> Ttx::Semantic::Negotiation::Status {
    return description.bind(*this, contract, output);
  }

  EXPORTED(COHERENCE) auto resolve_route(Perimortem::Core::View::Bytes route)
      -> Ttx::Graph::Abstract;

  EXPORTED(COHERENCE) auto visit_routes(Ttx::Graph::Abstract::Visitor visitor)
      -> void;

 private:
  constexpr Library(
      Description description,
      Perimortem::Core::View::Vector<Entry> entries)
      : description(description), entries(entries) {}

  Description description;
  Perimortem::Core::View::Vector<Entry> entries;
};

}  // namespace Coherence::TypeSystem
