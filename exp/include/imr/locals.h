// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#ifndef EXP_IMR_LOCALS_H
#define EXP_IMR_LOCALS_H

#include "imr/local.h"

typedef struct Locals {
    u32 length;
    u32 capacity;
    Local *buffer;
} Locals;

void locals_initialize(Locals *locals);
void locals_terminate(Locals *locals);

void locals_push(Locals *locals, Local local);
void locals_pop_n(Locals *locals, u32 n);
Local *locals_lookup(Locals *locals, StringView label);

#endif // !EXP_IMR_LOCALS_H
