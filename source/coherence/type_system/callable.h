// # Tetrodotoxin
// Copyright (c) 2023-present Matt Kaes and contributors

#ifndef COHERENCE_TYPE_SYSTEM_CALLABLE_H
#define COHERENCE_TYPE_SYSTEM_CALLABLE_H

#include "ttx/data/status.h"
#include "ttx/graph/abstract.h"

#define COHERENCE_CALLABLE_ID_HIGH 0x1fa9b2a9b4de46d1ULL
#define COHERENCE_CALLABLE_ID_LOW 0xb24175e1a7a275b3ULL

// Operation supplies the adapter that implements a Callable independently of
// its compiled input and output forms. The type compiler combines this record
// with the Callable signature to produce an Invocation. Each call uses the
// context published with this exact adapter.
typedef struct coherence_operation {
  void* context;
  ttx_data_status (*adapter)(void* context, const void* input, void* output);
} coherence_operation;

// Callable enumerates its positional argument declarations, supplies its
// result Type and exposes the operation implementing the call. Repeated
// arguments remain distinct positions and visitation preserves authored order.
// A None result represents no returned value.
//
// Callable describes logical signature and behavior. It does not store a
// compiled Invocation or assume one host language's call frame layout.
typedef struct coherence_callable {
  void* context;
  const struct coherence_callable_ops* operations;
} coherence_callable;

typedef struct coherence_callable_ops {
  ttx_abstract (*get_abstract)(void* context);
  void (*visit_arguments)(
      void* context,
      void* callback_context,
      void (*callback)(void* callback_context, ttx_abstract argument));
  ttx_abstract (*get_result)(void* context);
  coherence_operation (*get_operation)(void* context);
} coherence_callable_ops;

#endif
