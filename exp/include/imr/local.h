// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
#ifndef EXP_IMR_LOCAL_H
#define EXP_IMR_LOCAL_H

#include "imr/lifetime.h"
#include "imr/location.h"
#include "imr/type.h"
#include "utility/string_view.h"

typedef struct Local {
    Type const *type;
    StringView label;
    Lifetime lifetime;
    Location location;
} Local;

void local_initialize(Local *local);

void local_update_type(Local *local, Type const *type);
void local_update_label(Local *local, StringView label);
void local_update_first_use(Local *local, u32 first_use);
void local_update_last_use(Local *local, u32 last_use);
void local_update_location(Local *local, Location location);

#endif // EXP_IMR_LOCAL_H
