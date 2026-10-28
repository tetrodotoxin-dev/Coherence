// # Tetrodotoxin
// Copyright (c) 2023-present Matt Kaes and contributors

#ifndef COHERENCE_TYPE_SYSTEM_POLICIES_STATIC_H
#define COHERENCE_TYPE_SYSTEM_POLICIES_STATIC_H

// Static promises that compiling a Callable requires no receiver. The
// operation can be free or capture provider state through its own context.
// This policy is a support signal and supplies no API record through bind.
#define COHERENCE_STATIC_ID_HIGH 0xc2186c9239b3459bULL
#define COHERENCE_STATIC_ID_LOW 0x8c65e4f21c125298ULL

#endif
