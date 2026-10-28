// # Tetrodotoxin
// Copyright (c) 2023-present Matt Kaes and contributors

#pragma once

#include "ttx/data/status.hpp"

namespace Coherence::TypeSystem {

// Operation is the uncompiled terminal action of a Callable. Its adapter
// interprets input and output values under the forms produced by the type
// compiler. The record keeps no ownership of context or call storage.
class Operation {
 public:
  using Adapter =
      ttx_data_status (*)(void* context, const void* input, void* output);

  static constexpr auto create(void* context, Adapter adapter) -> Operation {
    return Operation(context, adapter);
  }

  auto invoke(const void* input, void* output) const -> Ttx::Data::Status {
    return Ttx::Data::Status(adapter(context, input, output));
  }

  constexpr auto get_context() const -> void* { return context; }
  constexpr auto get_adapter() const -> Adapter { return adapter; }

 private:
  constexpr Operation(void* context, Adapter adapter)
      : context(context), adapter(adapter) {}

  void* context;
  Adapter adapter;
};

}  // namespace Coherence::TypeSystem
