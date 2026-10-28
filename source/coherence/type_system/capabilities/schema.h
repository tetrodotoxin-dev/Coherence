// # Tetrodotoxin
// Copyright (c) 2023-present Matt Kaes and contributors

#ifndef COHERENCE_TYPE_SYSTEM_CAPABILITIES_SCHEMA_H
#define COHERENCE_TYPE_SYSTEM_CAPABILITIES_SCHEMA_H

#include "ttx/data/form/schema.h"
#include "ttx/graph/abstract.h"

#define COHERENCE_SCHEMA_ID_HIGH 0x7fa643ae4f6a42eeULL
#define COHERENCE_SCHEMA_ID_LOW 0x88557cd3ec02db84ULL

// Schema supplies the terminal data description for a Type with no structural
// Fields. The referenced Schema graph remains stable through the call that
// compiles it. Compilation owns its resulting canonical Representation and
// retains no source pointers.
//
// Structural Types derive their forms from Fields and do not need this
// capability. A provider supplying both Fields and Schema is inconsistent and
// the type compiler rejects that graph location.
typedef struct coherence_schema {
  void* context;
  const struct coherence_schema_ops* operations;
} coherence_schema;

typedef struct coherence_schema_ops {
  ttx_abstract (*get_abstract)(void* context);
  ttx_schema_reference (*get_schema)(void* context);
} coherence_schema_ops;

#endif
