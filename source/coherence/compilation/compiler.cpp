// # Tetrodotoxin
// Copyright (c) 2023-present Matt Kaes and contributors

#include "coherence/compilation/compiler.hpp"

#include "toolchain/memory/lifetime.hpp"

#include "perimortem/core/data.hpp"
#include "perimortem/core/view/null_terminated.hpp"
#include "perimortem/memory/allocator/arena.hpp"
#include "perimortem/memory/thread/vector.hpp"

#include "coherence/type_system/callable.hpp"
#include "coherence/type_system/capabilities/schema.hpp"
#include "coherence/type_system/policies/member.hpp"
#include "coherence/type_system/policies/static.hpp"
#include "coherence/type_system/routing.hpp"
#include "coherence/type_system/type.hpp"
#include "ttx/graph/policies/none.hpp"

using namespace Perimortem::Core;
using namespace Perimortem::Memory;
using namespace Toolchain::Memory;
using namespace Ttx::Data;
using namespace Coherence::Compilation;
using namespace Coherence::TypeSystem;

using Abstracts = Thread::Vector<Ttx::Graph::Abstract>;
using Stack = Thread::Vector<ttx_abstract>;
using SchemaResult = Storage::Result<Form::Schema::Reference, Status>;

static auto contains(const Stack& stack, Ttx::Graph::Abstract candidate)
    -> Bool {
  const ttx_abstract selected = candidate.get_abi();
  const auto values = stack.get_view();
  for (Count index = 0; index < values.get_size(); ++index) {
    if (values[index].context == selected.context &&
        values[index].operations == selected.operations) {
      return True;
    }
  }

  return False;
}

static auto align(Count offset, Count alignment)
    -> Storage::Result<Count, Status> {
  if (alignment == 0) {
    return Status::Invalid;
  }

  const Count remainder = offset % alignment;
  const Count padding = remainder ? alignment - remainder : 0;
  if (padding > Count(-1) - offset) {
    return Status::Overflow;
  }

  return offset + padding;
}

static auto build_type(
    Ttx::Graph::Abstract abstract,
    Count pointer_size,
    Count depth,
    Stack& stack,
    Allocator::Arena& arena) -> SchemaResult;

static auto build_types(
    View::Vector<Ttx::Graph::Abstract> types,
    Count pointer_size,
    Count depth,
    Stack& stack,
    Allocator::Arena& arena) -> SchemaResult {
  if (types.is_empty()) {
    auto& schema = arena.construct_from<Form::Schema>(
        [] { return Form::Schema::composite({}, 0, 1); });
    return Form::Schema::Reference(schema);
  }

  auto positions = arena.reserve<Form::Schema::Position>(types.get_size());
  Count extent = 0;
  Count alignment = 1;
  for (Count index = 0; index < types.get_size(); ++index) {
    auto child =
        build_type(types.get_data()[index], pointer_size, depth, stack, arena);
    Form::Schema::Reference reference;
    Status failure = Status::Success;
    Bool accepted = False;
    child.visit(
        [&](Form::Schema::Reference value) {
          reference = value;
          accepted = True;
        },

        [&](Status status) { failure = status; });
    if (!accepted) {
      return failure;
    }

    Count offset = 0;
    auto selected = align(extent, reference.get_alignment(pointer_size));
    selected.visit(
        [&](Count value) {
          offset = value;
          accepted = True;
        },

        [&](Status status) {
          accepted = False;
          failure = status;
        });

    if (!accepted || reference.get_extent(pointer_size) > Count(-1) - offset) {
      return accepted ? Status::Overflow : failure;
    }

    Lifetime::construct<Form::Schema::Position>(
        &positions.get_data()[index], [&] {
          Form::Schema::Position position;
          position.reference = reference;
          position.offset = offset;
          return position;
        });

    extent = offset + reference.get_extent(pointer_size);
    if (reference.get_alignment(pointer_size) > alignment) {
      alignment = reference.get_alignment(pointer_size);
    }
  }

  Status failure = Status::Success;
  Bool accepted = False;
  auto final_extent = align(extent, alignment);
  final_extent.visit(
      [&](Count value) {
        extent = value;
        accepted = True;
      },

      [&](Status status) { failure = status; });
  if (!accepted) {
    return failure;
  }

  auto& schema = arena.construct_from<Form::Schema>([&] {
    return Form::Schema::composite(positions.get_view(), extent, alignment);
  });

  return Form::Schema::Reference(schema);
}

static auto build_declarations(
    View::Vector<Ttx::Graph::Abstract> declarations,
    Count pointer_size,
    Count depth,
    Stack& stack,
    Allocator::Arena& arena) -> SchemaResult {
  Abstracts types(declarations.get_size());
  for (Count index = 0; index < declarations.get_size(); ++index) {
    types.insert(declarations.get_data()[index].resolve_route(Routing::type));
  }

  return build_types(types.get_view(), pointer_size, depth, stack, arena);
}

static auto build_type(
    Ttx::Graph::Abstract abstract,
    Count pointer_size,
    Count depth,
    Stack& stack,
    Allocator::Arena& arena) -> SchemaResult {
  if (depth == Compiler::nesting_limit) {
    return Status::Bounds;
  }

  if (abstract.get_state() != Ttx::Graph::Abstract::State::Present) {
    return Status::Unsupported;
  }

  if (contains(stack, abstract)) {
    return Status::Invalid;
  }

  stack.insert(abstract.get_abi());

  Abstracts fields;
  Bool supplied_type = False;
  abstract.bind<Type::Contract>().visit(
      [&](Type::Contract value) {
        supplied_type = True;

        auto collect = [&](Ttx::Graph::Abstract field) {
          fields.insert(field);
        };

        value.visit_fields(collect);
      },

      [] {});
  if (!supplied_type) {
    stack.remove(stack.get_size() - 1);
    return Status::Unsupported;
  }

  Bool supplied_schema = False;
  Form::Schema::Reference schema;
  abstract.bind<Capabilities::Schema>().visit(
      [&](Capabilities::Schema value) {
        supplied_schema = True;
        schema = value.get_schema();
      },

      [] {});

  if (supplied_schema) {
    stack.remove(stack.get_size() - 1);
    return fields.is_empty() ? SchemaResult(schema)
                             : SchemaResult(Status::Incompatible);
  }

  auto result = build_declarations(
      fields.get_view(), pointer_size, depth + 1, stack, arena);
  stack.remove(stack.get_size() - 1);
  return result;
}

static auto compile_operation(
    Ttx::Graph::Abstract callable,
    const Ttx::Graph::Abstract* receiver,
    Count pointer_size) -> Compiler::Result<Executable> {
  if (pointer_size != 4 && pointer_size != 8) {
    return Status::Unsupported;
  }

  const auto scope = receiver ? callable.supports<Policies::Member>()
                              : callable.supports<Policies::Static>();
  if (scope != Ttx::Semantic::Negotiation::Status::Satisfied) {
    return Status::Unsupported;
  }

  Bool supplied_callable = False;
  Ttx::Graph::Abstract result =
      Ttx::Graph::Policies::None::get_none().get_abstract();
  Operation operation = Operation::create(nullptr, nullptr);
  Abstracts arguments;
  callable.bind<Callable::Contract>().visit(
      [&](Callable::Contract value) {
        supplied_callable = True;

        auto collect = [&](Ttx::Graph::Abstract argument) {
          arguments.insert(argument);
        };

        value.visit_arguments(collect);
        result = value.get_result();
        operation = value.get_operation();
      },

      [] {});
  if (!supplied_callable || !operation.get_adapter()) {
    return Status::Unsupported;
  }

  Allocator::Arena arena;
  Stack stack;
  Abstracts input_types(arguments.get_size() + (receiver ? 1 : 0));
  if (receiver) {
    input_types.insert(*receiver);
  }

  const auto declarations = arguments.get_view();
  for (Count index = 0; index < declarations.get_size(); ++index) {
    input_types.insert(
        declarations.get_data()[index].resolve_route(Routing::type));
  }

  SchemaResult input_schema =
      input_types.get_size() == 1
          ? build_type(
                input_types.get_view().get_data()[0], pointer_size, 0, stack,
                arena)
          : build_types(input_types.get_view(), pointer_size, 0, stack, arena);
  Form::Representation input_form;
  Status failure = Status::Success;
  Bool accepted_inputs = False;
  input_schema.visit(
      [&](Form::Schema::Reference schema) {
        auto compiled = Form::Representation::compile(schema, pointer_size);
        compiled.visit(
            [&](Form::Representation& value) {
              input_form = Data::take(value);
              accepted_inputs = True;
            },

            [&](Status status) { failure = status; });
      },

      [&](Status status) { failure = status; });
  if (!accepted_inputs) {
    return failure;
  }

  SchemaResult output_schema =
      result.get_state() == Ttx::Graph::Abstract::State::None
          ? build_types({}, pointer_size, 0, stack, arena)
          : build_type(result, pointer_size, 0, stack, arena);
  Form::Representation output_form;
  Bool accepted_output = False;
  output_schema.visit(
      [&](Form::Schema::Reference schema) {
        auto compiled = Form::Representation::compile(schema, pointer_size);
        compiled.visit(
            [&](Form::Representation& value) {
              output_form = Data::take(value);
              accepted_output = True;
            },

            [&](Status status) { failure = status; });
      },

      [&](Status status) { failure = status; });
  if (!accepted_output) {
    return failure;
  }

  return Executable::create(
      Data::take(input_form), Data::take(output_form), operation);
}

auto Compiler::compile(Ttx::Graph::Abstract type, Count pointer_size)
    -> Result<Form::Representation> {
  if (pointer_size != 4 && pointer_size != 8) {
    return Status::Unsupported;
  }

  Allocator::Arena arena;
  Stack stack;
  auto schema = build_type(type, pointer_size, 0, stack, arena);
  return schema.visit(
      [&](Form::Schema::Reference value) {
        return Form::Representation::compile(value, pointer_size);
      },

      [](Status status) -> Result<Form::Representation> { return status; });
}

auto Compiler::compile_callable(
    Ttx::Graph::Abstract callable,
    Count pointer_size) -> Result<Executable> {
  return compile_operation(callable, nullptr, pointer_size);
}

auto Compiler::compile_member(
    Ttx::Graph::Abstract receiver,
    Ttx::Graph::Abstract callable,
    Count pointer_size) -> Result<Executable> {
  return compile_operation(callable, &receiver, pointer_size);
}
