// # Tetrodotoxin
// Copyright (c) 2023-present Matt Kaes and contributors

#pragma once

#include "perimortem/core/data.hpp"
#include "perimortem/core/identity/uuid.hpp"
#include "perimortem/core/storage/vector.hpp"
#include "perimortem/core/view/vector.hpp"

#include "coherence/type_system/callable.h"
#include "coherence/type_system/description.hpp"
#include "coherence/type_system/field.hpp"
#include "coherence/type_system/operation.hpp"
#include "coherence/type_system/policies/member.hpp"
#include "coherence/type_system/policies/static.hpp"
#include "coherence/type_system/reference.hpp"
#include "ttx/graph/abstract.hpp"
#include "ttx/graph/projection.hpp"

namespace Coherence::TypeSystem {

// Callable describes one operation independently of its compiled call frame.
// Arguments are ordered Field declarations, the result is one Type graph
// location and Operation supplies the terminal adapter. The compiler derives
// the input and output forms before exposing an Invocation to a consumer.
//
// Callable stores Views and an operation record without owning their contexts
// or source ranges. Argument and result Abstracts can come from native
// projections or foreign providers. The Callable provider keeps every
// referenced interface accessible through the published access window.
class Callable
    : public Ttx::Graph::
          Projection<Callable, 0x3afe480c75d34dfeULL, 0xa305c2ee6300f0f3ULL> {
 public:
  // Contract exposes the callable signature and terminal operation to every
  // provider language using the Coherence ABI. Projection's native Abstract
  // remains an optional C++ recovery path and does not replace that contract.
  class Contract {
   public:
    using Api = coherence_callable;
    static constexpr auto contract_id = Perimortem::Core::Identity::Uuid(
        COHERENCE_CALLABLE_ID_HIGH,
        COHERENCE_CALLABLE_ID_LOW);

    explicit constexpr Contract(Api api) : api(api) {}

    constexpr auto get_abi() const -> Api { return api; }

    auto get_abstract() const -> Ttx::Graph::Abstract {
      return Ttx::Graph::Abstract(api.operations->get_abstract(api.context));
    }

    template <typename Function>
    auto visit_arguments(Function& callback) const -> void {
      api.operations->visit_arguments(
          api.context, &callback, [](void* context, ttx_abstract argument) {
            (*Perimortem::Core::Data::cast<Function>(context))(
                Ttx::Graph::Abstract(argument));
          });
    }

    auto get_result() const -> Ttx::Graph::Abstract {
      return Ttx::Graph::Abstract(api.operations->get_result(api.context));
    }

    auto get_operation() const -> TypeSystem::Operation {
      const coherence_operation operation =
          api.operations->get_operation(api.context);
      return TypeSystem::Operation::create(
          operation.context, operation.adapter);
    }

   private:
    Api api;
  };

  enum class Scope : U8 {
    Static,
    Member,
  };

  // Reference stores either a native Callable projection or a graph interface
  // supplied by another provider. Argument uses the same composition model for
  // Field declarations.
  using Reference = TypeSystem::Reference<Abstract>;
  using Argument = Field::Reference;

  inline static constexpr Perimortem::Core::Storage::
      Vector<Perimortem::Core::Identity::Uuid, 2>
          supported_contracts = {{
            Ttx::Graph::Capabilities::Describe::contract_id,
            Contract::contract_id,
          }};

  static constexpr auto create(
      Description description,
      Perimortem::Core::View::Vector<Argument> arguments,
      Ttx::Graph::Abstract result,
      Scope scope,
      Operation operation) -> Callable {
    return Callable(description, arguments, result, scope, operation);
  }

  constexpr auto get_name() const -> Perimortem::Core::View::Bytes {
    return description.get_name();
  }

  constexpr auto get_description() const -> Perimortem::Core::View::Bytes {
    return description.get_documentation();
  }

  constexpr auto get_arguments() const
      -> Perimortem::Core::View::Vector<Argument> {
    return arguments;
  }

  constexpr auto get_result() const -> Ttx::Graph::Abstract { return result; }
  constexpr auto get_scope() const -> Scope { return scope; }
  constexpr auto get_operation() const -> Operation { return operation; }

  auto supports_contract(Perimortem::Core::Identity::Uuid contract) const
      -> Ttx::Semantic::Negotiation::Status {
    if (contract == Policies::Member::contract_id) {
      return scope == Scope::Member
                 ? Ttx::Semantic::Negotiation::Status::Satisfied
                 : Ttx::Semantic::Negotiation::Status::Rejected;
    }

    if (contract == Policies::Static::contract_id) {
      return scope == Scope::Static
                 ? Ttx::Semantic::Negotiation::Status::Satisfied
                 : Ttx::Semantic::Negotiation::Status::Rejected;
    }

    return Ttx::Semantic::Negotiation::Status::Unknown;
  }

  EXPORTED(COHERENCE) auto bind_contract(
      Perimortem::Core::Identity::Uuid contract,
      void* output) -> Ttx::Semantic::Negotiation::Status;

 private:
  constexpr Callable(
      Description description,
      Perimortem::Core::View::Vector<Argument> arguments,
      Ttx::Graph::Abstract result,
      Scope scope,
      Operation operation)
      : description(description),
        arguments(arguments),
        result(result),
        scope(scope),
        operation(operation) {}

  Description description;
  Perimortem::Core::View::Vector<Argument> arguments;
  Ttx::Graph::Abstract result;
  Scope scope;
  Operation operation;
};

}  // namespace Coherence::TypeSystem
