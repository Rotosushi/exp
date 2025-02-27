// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
#ifndef EXP_ENV_GLOBAL_SYMBOLS_H
#define EXP_ENV_GLOBAL_SYMBOLS_H

#include "utility/string_view.h"

typedef struct GlobalLabels {
    u32 count;
    u32 capacity;
    StringView *buffer;
} Labels;

void labels_initialize(Labels *labels);
void labels_terminate(Labels *labels);

u32 labels_insert(Labels *labels, StringView label);
StringView labels_at(Labels *labels, u32 index);

#endif // !EXP_ENV_GLOBAL_SYMBOLS_H
