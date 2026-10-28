// # Tetrodotoxin
// Copyright (c) 2023-present Matt Kaes and contributors

#pragma once

#include "perimortem/core/storage/result.hpp"

#include "coherence/compilation/executable.hpp"
#include "coherence/type_system/type.hpp"
#include "ttx/data/form/representation.hpp"
#include "ttx/data/status.hpp"
#include "ttx/graph/abstract.hpp"

namespace Coherence::Compilation {

// Compiler derives TTX Representations from logical Type graphs. Structural
// Types place their ordered Fields with each field Type's extent and natural
// alignment. A Type without Fields must supply Schema, which provides the
// terminal data description for primitive values, pointers and foreign forms.
//
// Compilation rejects a graph location that supplies both Fields and Schema,
// a recursive storage cycle, more than 256 nested Types, a missing Type
// contract or an unresolved field Type route. Each successful result owns its
// compiled descriptor bytes and keeps no reference to the source graph.
class Compiler {
 public:
  // The fixed nesting bound guarantees termination when a provider republishes
  // one semantic cycle through distinct Abstract records that cannot be
  // recognized by record identity alone.
  static constexpr Count nesting_limit = 256;

  template <typename Value>
  using Result = Perimortem::Core::Storage::Result<Value, Ttx::Data::Status>;

  static EXPORTED(COHERENCE) auto compile(
      Ttx::Graph::Abstract type,
      Count pointer_size = sizeof(void*))
      -> Result<Ttx::Data::Form::Representation>;

  static auto compile(
      TypeSystem::Type::Abstract type,
      Count pointer_size = sizeof(void*))
      -> Result<Ttx::Data::Form::Representation> {
    return compile(type.get_abstract(), pointer_size);
  }

  static EXPORTED(COHERENCE) auto compile_callable(
      Ttx::Graph::Abstract callable,
      Count pointer_size = sizeof(void*)) -> Result<Executable>;

  static EXPORTED(COHERENCE) auto compile_member(
      Ttx::Graph::Abstract receiver,
      Ttx::Graph::Abstract callable,
      Count pointer_size = sizeof(void*)) -> Result<Executable>;

  static auto compile_member(
      TypeSystem::Type::Abstract receiver,
      Ttx::Graph::Abstract callable,
      Count pointer_size = sizeof(void*)) -> Result<Executable> {
    return compile_member(receiver.get_abstract(), callable, pointer_size);
  }
};

}  // namespace Coherence::Compilation
