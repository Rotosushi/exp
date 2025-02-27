// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
#ifndef EXP_UTILITY_MEMORY_H
#define EXP_UTILITY_MEMORY_H

#include "utility/int_types.h"

void memory_copy(void *restrict target, u64 target_length,
                 void const *restrict source, u64 source_length);

void memory_move(void *target, u64 target_length, void const *source,
                 u64 source_length);

i32 memory_compare(void const *target, u64 target_length, void const *source,
                   u64 source_length);

#endif // EXP_UTILITY_MEMORY_H
