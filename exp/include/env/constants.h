// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
#ifndef EXP_IMR_CONSTANTS_H
#define EXP_IMR_CONSTANTS_H
#include <stddef.h>

#include "imr/value.h"

typedef struct Constants {
    u32 count;
    u32 capacity;
    Value *buffer;
} Constants;

void constants_initialize(Constants *constants);
void constants_terminate(Constants *constants);

Value *constants_at(Constants *constants, u32 constant);
u32 constants_append_tuple(Constants *constants, Tuple tuple);

#endif // !EXP_IMR_CONSTANTS_H
