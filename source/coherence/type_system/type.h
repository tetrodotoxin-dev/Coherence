// # Tetrodotoxin
// Copyright (c) 2023-present Matt Kaes and contributors

#ifndef COHERENCE_TYPE_SYSTEM_TYPE_H
#define COHERENCE_TYPE_SYSTEM_TYPE_H

#include "ttx/graph/abstract.h"

#define COHERENCE_TYPE_ID_HIGH 0x153790825095465bULL
#define COHERENCE_TYPE_ID_LOW 0x9a1ce45c41b8753eULL

// Type identifies one type-system definition and enumerates its storage and
// callable members. Each callback receives the complete Abstract supplied by
// that member's provider and finishes before visitation returns. Repeated
// members remain distinct positions, and their order is the authored order.
//
// Fields describe member storage. Each Callable independently supplies Member
// or Static as a support signal. A Member argument sequence excludes the
// receiver, and compilation prepends this Type as its first input value. A
// Static Callable receives only its authored arguments.
// Free storage and free functions live at their own graph locations rather
// than being attached to a Type.
//
// This contract describes logical structure. Physical layout and invocation
// forms are compiler results and are not stored in this API.
typedef struct coherence_type {
  void* context;
  const struct coherence_type_ops* operations;
} coherence_type;

typedef struct coherence_type_ops {
  ttx_abstract (*get_abstract)(void* context);
  void (*visit_fields)(
      void* context,
      void* callback_context,
      void (*callback)(void* callback_context, ttx_abstract field));
  void (*visit_callables)(
      void* context,
      void* callback_context,
      void (*callback)(void* callback_context, ttx_abstract callable));
} coherence_type_ops;

#endif
