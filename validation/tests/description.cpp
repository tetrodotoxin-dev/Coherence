// # Tetrodotoxin
// Copyright (c) 2023-present Matt Kaes and contributors

#include "coherence/type_system/description.hpp"

#include "toolchain/validation/test.hpp"

#include "perimortem/core/view/null_terminated.hpp"

#include "coherence/type_system/callable.hpp"
#include "coherence/type_system/field.hpp"
#include "coherence/type_system/type.hpp"
#include "ttx/graph/capabilities/describe.hpp"

using namespace Perimortem::Core;
using namespace Toolchain::Validation;
using namespace Ttx::Graph;

using namespace Coherence::TypeSystem;

static Harness DescriptionTests = {.name = "Coherence::Description"};

VALIDATION_TEST(DescriptionTests, graph_binding) {
  auto type = Type::create(
      Description::create(
          "Example"_view,
          "A type used to verify graph description binding."_view),
      View::Vector<Type::FieldReference>(),
      View::Vector<Type::CallableReference>());

  type.get_abstract().bind<Ttx::Graph::Capabilities::Describe>().visit(
      [&](Ttx::Graph::Capabilities::Describe description) {
        description.get_name().visit(
            [&](View::Bytes name) { EXPECT_EQ(name, "Example"_view); },
            [&] { EXPECT(False); });
      },

      [&] { EXPECT(False); });
}
