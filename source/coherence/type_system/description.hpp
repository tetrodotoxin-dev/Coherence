// # Tetrodotoxin
// Copyright (c) 2023-present Matt Kaes and contributors

#pragma once

#include "perimortem/core/data.hpp"
#include "perimortem/core/identity/uuid.hpp"
#include "perimortem/core/view/bytes.hpp"

#include "ttx/graph/capabilities/describe.hpp"
#include "ttx/semantic/negotiation/status.hpp"

namespace Coherence::TypeSystem {

// Description provides the human-readable identity attached to one type-system
// value. Both Views retain the provider's source storage. A provider publishes
// the containing value only while those bytes remain readable and unchanged.
// Empty Views are present empty values rather than absent metadata.
class Description {
 public:
  static constexpr auto create(
      Perimortem::Core::View::Bytes name,
      Perimortem::Core::View::Bytes documentation) -> Description {
    return Description(name, documentation);
  }

  constexpr auto get_name() const -> Perimortem::Core::View::Bytes {
    return name;
  }

  constexpr auto get_documentation() const -> Perimortem::Core::View::Bytes {
    return documentation;
  }

  template <typename Concrete>
  auto bind(
      Concrete& value,
      Perimortem::Core::Identity::Uuid contract,
      void* output) const -> Ttx::Semantic::Negotiation::Status {
    if (contract != Ttx::Graph::Capabilities::Describe::contract_id) {
      return Ttx::Semantic::Negotiation::Status::Unknown;
    }

    ttx_describe api;
    api.context = &value;
    api.operations = &Binding<Concrete>::operations;
    *Perimortem::Core::Data::cast<ttx_describe>(output) = api;
    return Ttx::Semantic::Negotiation::Status::Satisfied;
  }

 private:
  template <typename Concrete>
  class Binding {
    static auto get_abstract(void* context) -> ttx_abstract {
      return Perimortem::Core::Data::cast<Concrete>(context)
          ->get_abstract()
          .get_abi();
    }

    static auto get_name(void* context, perimortem_view_bytes* output) -> U8 {
      *output = Perimortem::Core::Data::cast<Concrete>(context)->get_name();
      return 1;
    }

    static auto get_description(void* context, perimortem_view_bytes* output)
        -> U8 {
      *output =
          Perimortem::Core::Data::cast<Concrete>(context)->get_description();
      return 1;
    }

   public:
    inline static constexpr ttx_describe_ops operations = {
      get_abstract,
      get_name,
      get_description,
    };
  };

  constexpr Description(
      Perimortem::Core::View::Bytes name,
      Perimortem::Core::View::Bytes documentation)
      : name(name), documentation(documentation) {}

  Perimortem::Core::View::Bytes name;
  Perimortem::Core::View::Bytes documentation;
};

}  // namespace Coherence::TypeSystem
