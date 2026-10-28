// # Tetrodotoxin
// Copyright (c) 2023-present Matt Kaes and contributors

#include "toolchain/validation/test.hpp"

#include "perimortem/core/storage/vector.hpp"
#include "perimortem/core/view/null_terminated.hpp"

#include "coherence/compilation/compiler.hpp"
#include "coherence/type_system/callable.hpp"
#include "coherence/type_system/field.hpp"
#include "coherence/type_system/library.hpp"
#include "coherence/type_system/primitive.hpp"
#include "coherence/type_system/routing.hpp"
#include "coherence/type_system/type.hpp"
#include "ttx/data/form/native.hpp"
#include "ttx/graph/policies/none.h"

using namespace Perimortem::Core;
using namespace Toolchain::Validation;
using namespace Coherence::TypeSystem;
using namespace Coherence::Compilation;

using namespace Ttx::Data;
using namespace Ttx::Graph;

static constexpr auto empty_type = Type::create(
    Description::create("Empty"_view, "A type with no members."_view),
    View::Vector<Type::FieldReference>(),
    View::Vector<Type::CallableReference>());

static_assert(empty_type.get_name() == "Empty"_view);
static_assert(empty_type.get_fields().is_empty());
static_assert(empty_type.get_callables().is_empty());
static_assert(Type::Contract::contract_id != Type::Abstract::contract_id);
static_assert(
    Callable::Contract::contract_id != Callable::Abstract::contract_id);

template <typename Concrete>
static auto project(Concrete& value) -> typename Concrete::Abstract {
  auto projected = Concrete::Abstract::convert(value.get_abstract());
  return projected.visit(
      [](const typename Concrete::Abstract& abstract) { return abstract; },
      []() -> typename Concrete::Abstract { __builtin_trap(); });
}

static auto project_type(Abstract value) -> Type::Abstract {
  auto projected = Type::Abstract::convert(value);
  return projected.visit(
      [](const Type::Abstract& abstract) { return abstract; },
      []() -> Type::Abstract { __builtin_trap(); });
}

static auto require_library(Storage::Option<Library> candidate) -> Library {
  return candidate.visit(
      [](const Library& library) { return library; },
      []() -> Library { __builtin_trap(); });
}

struct Pair {
  U64 first;
  U64 second;
};

// GeneratedNesting supplies a new Abstract record at every level. Exact record
// cycle detection cannot classify it as a cycle, so the compiler's structural
// bound provides the termination guarantee under test.
class GeneratedNesting {
  struct Context {
    GeneratedNesting* owner = nullptr;
    Count depth = 0;
  };

 public:
  GeneratedNesting(Count levels) : levels(levels) {
    for (Count depth = 0; depth < contexts.get_size(); ++depth) {
      contexts[depth].owner = this;
      contexts[depth].depth = depth;
    }
  }

  auto get_abstract() -> Abstract { return type(0); }

 private:
  static auto context(void* value) -> Context& {
    return *Data::cast<Context>(value);
  }

  auto type(Count depth) -> Abstract {
    return Abstract(&contexts[depth], type_operations);
  }

  auto field(Count depth) -> Abstract {
    return Abstract(&contexts[depth], field_operations);
  }

  static auto type_abstract(void* value) -> ttx_abstract {
    auto& selected = context(value);
    return selected.owner->type(selected.depth).get_abi();
  }

  static auto visit_fields(
      void* value,
      void* callback_context,
      void (*callback)(void*, ttx_abstract)) -> void {
    auto& selected = context(value);
    if (selected.depth + 1 == selected.owner->levels) {
      return;
    }

    callback(callback_context, selected.owner->field(selected.depth).get_abi());
  }

  static auto visit_callables(
      void* value,
      void* callback_context,
      void (*callback)(void*, ttx_abstract)) -> void {
    (void)value;
    (void)callback_context;
    (void)callback;
  }

  static auto type_supports(void* value, perimortem_uuid requested)
      -> ttx_negotiation_status {
    (void)value;
    if (!requested.high && requested.low <= 1) {
      return TTX_NEGOTIATION_REJECTED;
    }

    const Identity::Uuid contract(requested);
    return contract == Type::Contract::contract_id ||
                   contract == Abstract::contract_id
               ? TTX_NEGOTIATION_SATISFIED
               : TTX_NEGOTIATION_UNKNOWN;
  }

  static auto type_bind(void* value, perimortem_uuid requested, void* output)
      -> ttx_negotiation_status {
    const auto status = type_supports(value, requested);
    if (status != TTX_NEGOTIATION_SATISFIED) {
      return status;
    }

    if (Identity::Uuid(requested) == Type::Contract::contract_id) {
      static constexpr coherence_type_ops operations = {
        type_abstract,
        visit_fields,
        visit_callables,
      };

      *Data::cast<coherence_type>(output) = {value, &operations};
    } else {
      *Data::cast<ttx_abstract>(output) = type_abstract(value);
    }

    return TTX_NEGOTIATION_SATISFIED;
  }

  static auto field_route(void* value, perimortem_view_bytes route)
      -> ttx_abstract {
    auto& selected = context(value);
    return View::Bytes(route) == Routing::type
               ? selected.owner->type(selected.depth + 1).get_abi()
               : ttx_none();
  }

  static auto field_visit(void* value, ttx_graph_visitor visitor) -> void {
    auto& selected = context(value);
    visitor.callback(
        visitor.callback_context, Routing::type,
        selected.owner->type(selected.depth + 1).get_abi());
  }

  static auto empty_route(void* value, perimortem_view_bytes route)
      -> ttx_abstract {
    (void)value;
    (void)route;
    return ttx_none();
  }

  static auto empty_visit(void* value, ttx_graph_visitor visitor) -> void {
    (void)value;
    (void)visitor;
  }

  static auto field_supports(void* value, perimortem_uuid requested)
      -> ttx_negotiation_status {
    (void)value;
    if (!requested.high && requested.low <= 1) {
      return TTX_NEGOTIATION_REJECTED;
    }

    return Identity::Uuid(requested) == Abstract::contract_id
               ? TTX_NEGOTIATION_SATISFIED
               : TTX_NEGOTIATION_UNKNOWN;
  }

  static auto field_bind(void* value, perimortem_uuid requested, void* output)
      -> ttx_negotiation_status {
    const auto status = field_supports(value, requested);
    if (status == TTX_NEGOTIATION_SATISFIED) {
      auto& selected = context(value);
      *Data::cast<ttx_abstract>(output) =
          selected.owner->field(selected.depth).get_abi();
    }

    return status;
  }

  static auto state(void* value) -> ttx_abstract_state {
    (void)value;
    return TTX_ABSTRACT_PRESENT;
  }

  inline static constexpr ttx_abstract_ops type_operations = {
    type_supports, type_bind, empty_route, empty_visit, state,
  };

  inline static constexpr ttx_abstract_ops field_operations = {
    field_supports, field_bind, field_route, field_visit, state,
  };

  Storage::Vector<Context, Compiler::nesting_limit + 1> contexts;
  Count levels;
};

static auto add(void* context, const void* input, void* output)
    -> ttx_data_status {
  const auto bias = *Data::cast<U64>(context);
  const auto& pair = *Data::cast<Pair>(input);
  *Data::cast<U64>(output) = pair.first + pair.second + bias;
  return TTX_DATA_SUCCESS;
}

static Harness TypeSystemTests = {.name = "Coherence::TypeSystem"};

VALIDATION_TEST(TypeSystemTests, native_and_graph_models) {
  static constexpr auto integer_schema =
      Form::Schema::primitive(Form::Schema::Value::U64);
  auto integer = Primitive::create(
      Description::create("u64"_view, "An unsigned 64-bit integer."_view),
      integer_schema);
  const auto integer_graph = integer.get_abstract();
  const auto integer_type = project_type(integer_graph);
  EXPECT_EQ(&integer_type.get_typed(), &integer.get_type());

  const auto returned_integer = integer_type.get_abstract();
  EXPECT_EQ(
      returned_integer.get_abi().context, integer_graph.get_abi().context);
  EXPECT(
      returned_integer.supports<Primitive::Abstract>() ==
      Ttx::Semantic::Negotiation::Status::Satisfied);
  integer_graph.bind<Type::Contract>().visit(
      [&](Type::Contract type) {
        EXPECT_EQ(
            type.get_abstract().get_abi().context,
            integer_graph.get_abi().context);
      },

      [&] { EXPECT(False); });

  auto left = Field::create(
      Description::create("left"_view, "The first integer."_view),
      integer_type.get_abstract());
  auto right = Field::create(
      Description::create("right"_view, "The second integer."_view),
      integer_type.get_abstract());

  const Storage::Vector<Field::Reference, 2> arguments = {{
    Field::Reference(project(left)),
    Field::Reference(right.get_abstract()),
  }};

  U64 bias = 4;
  auto sum = Callable::create(
      Description::create("sum"_view, "Adds two integers."_view),
      arguments.get_view(), integer_type.get_abstract(),
      Callable::Scope::Static, Operation::create(&bias, add));

  const Storage::Vector<Callable::Reference, 1> callables = {{
    Callable::Reference(project(sum)),
  }};

  auto pair = Type::create(
      Description::create("Pair"_view, "Two integer members."_view),
      arguments.get_view(), callables.get_view());

  const auto graph = pair.get_abstract();

  const Storage::Vector<Library::Entry, 2> library_types = {{
    Library::Entry::create("u64"_view, Type::Reference(integer_type)),
    Library::Entry::create("Pair"_view, Type::Reference(graph)),
  }};

  library_types[0].get_type().visit(
      [&](const Type::Abstract& type) {
        EXPECT_EQ(type.get_abi().context, integer_type.get_abi().context);
        EXPECT_EQ(type.get_abi().operations, integer_type.get_abi().operations);
        Type::Abstract::convert(type.get_abstract())
            .visit(
                [&](const Type::Abstract& returned) {
                  EXPECT_EQ(&returned.get_typed(), &type.get_typed());
                },

                [&] { EXPECT(False); });
      },

      [&](const Abstract&) { EXPECT(False); }, [&] { EXPECT(False); });

  auto library = require_library(
      Library::create(
          Description::create(
              "ExampleTypes"_view,
              "Native and foreign Type references in one library."_view),
          library_types.get_view()));

  const Storage::Vector<Library::Entry, 2> duplicate_routes = {{
    Library::Entry::create("value"_view, Type::Reference(integer_type)),
    Library::Entry::create("value"_view, Type::Reference(graph)),
  }};

  EXPECT_NOT(
      Library::create(
          Description::create("Invalid"_view, "Repeated routes."_view),
          duplicate_routes.get_view()));

  const Storage::Vector<Library::Entry, 2> repeated_descriptions = {{
    Library::Entry::create("first"_view, Type::Reference(integer_type)),
    Library::Entry::create("second"_view, Type::Reference(integer_type)),
  }};

  EXPECT(
      Library::create(
          Description::create("Aliases"_view, "Two routes to one Type."_view),
          repeated_descriptions.get_view()));
  Type::Abstract::convert(library.resolve_route("u64"_view))
      .visit(
          [&](const Type::Abstract& resolved) {
            EXPECT_EQ(resolved.get_typed().get_name(), "u64"_view);
          },

          [&] { EXPECT(False); });
  EXPECT_EQ(library.resolve_route("Pair"_view), graph);

  Count visited_types = 0;
  auto visit_type = [&](View::Bytes, Abstract) { ++visited_types; };
  library.visit_routes(Abstract::Visitor(visit_type));
  EXPECT_EQ(visited_types, Count(2));

  graph.bind<Type::Contract>().visit(
      [&](Type::Contract type) {
        Count field_count = 0;
        auto field = [&](Abstract declaration) {
          EXPECT_EQ(
              declaration.resolve_route(Routing::type),
              integer_type.get_abstract());
          ++field_count;
        };

        type.visit_fields(field);
        EXPECT_EQ(field_count, Count(2));

        Count callable_count = 0;
        auto callable = [&](Abstract declaration) {
          declaration.bind<Callable::Contract>().visit(
              [&](Callable::Contract reflected) {
                Count argument_count = 0;
                auto argument = [&](Abstract) { ++argument_count; };
                reflected.visit_arguments(argument);
                EXPECT_EQ(argument_count, Count(2));
                EXPECT_EQ(reflected.get_result(), integer_type.get_abstract());

                Pair input;
                input.first = 10;
                input.second = 20;

                U64 output = 0;
                EXPECT(
                    reflected.get_operation().invoke(&input, &output) ==
                    Status::Success);
                EXPECT_EQ(output, U64(34));
              },

              [&] { EXPECT(False); });
          ++callable_count;
        };

        type.visit_callables(callable);
        EXPECT_EQ(callable_count, Count(1));
      },

      [&] { EXPECT(False); });

  Compiler::compile(integer.get_abstract())
      .visit(
          [&](const Form::Representation& representation) {
            EXPECT_EQ(representation.get_extent(), Count(8));
            EXPECT_EQ(representation.get_alignment(), Count(8));
          },

          [&](Status) { EXPECT(False); });

  Compiler::compile(graph).visit(
      [&](const Form::Representation& representation) {
        EXPECT_EQ(representation.get_extent(), Count(16));
        EXPECT_EQ(representation.get_alignment(), Count(8));
      },

      [&](Status) { EXPECT(False); });

  Compiler::compile_callable(sum.get_abstract())
      .visit(
          [&](Executable& executable) {
            EXPECT_EQ(executable.get_inputs().get_extent(), Count(16));
            EXPECT_EQ(executable.get_output().get_extent(), Count(8));

            Executable copied(executable);
            Executable moved(Data::take(copied));
            EXPECT(moved.accepts(moved.get_inputs(), moved.get_output()));

            Pair input;
            input.first = 5;
            input.second = 6;

            U64 output = 0;
            EXPECT(moved.invoke(&input, &output) == Status::Success);
            EXPECT_EQ(output, U64(15));
          },

          [&](Status) { EXPECT(False); });
}

VALIDATION_TEST(TypeSystemTests, bounded_generated_nesting) {
  GeneratedNesting accepted(Compiler::nesting_limit);
  Compiler::compile(accepted.get_abstract())
      .visit(
          [&](const Form::Representation&) {}, [&](Status) { EXPECT(False); });

  GeneratedNesting refused(Compiler::nesting_limit + 1);
  Compiler::compile(refused.get_abstract())
      .visit(
          [&](const Form::Representation&) { EXPECT(False); },
          [&](Status status) { EXPECT(status == Status::Bounds); });
}
