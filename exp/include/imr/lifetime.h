// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
#ifndef EXP_IMR_LIFETIME_H
#define EXP_IMR_LIFETIME_H

#include "utility/int_types.h"

typedef struct Lifetime {
    u32 first_use;
    u32 last_use;
} Lifetime;

Lifetime lifetime_construct(u32 first, u32 last);
Lifetime lifetime_create();
Lifetime lifetime_immortal();

#endif // EXP_IMR_LIFETIME_H
