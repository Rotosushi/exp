// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
#ifndef EXP_UTILITY_ARRAY_GROWTH_H
#define EXP_UTILITY_ARRAY_GROWTH_H
#include "utility/int_types.h"

typedef struct Growth64 {
    u64 new_capacity;
    u64 alloc_size;
} Growth64;
Growth64 array_growth_u64(u64 current_capacity, u64 element_size);

typedef struct Growth32 {
    u32 new_capacity;
    u64 alloc_size;
} Growth32;
Growth32 array_growth_u32(u32 current_capacity, u64 element_size);

typedef struct Growth16 {
    u16 new_capacity;
    u64 alloc_size;
} Growth16;
Growth16 array_growth_u16(u16 current_capacity, u64 element_size);

typedef struct Growth8 {
    u8 new_capacity;
    u64 alloc_size;
} Growth8;
Growth8 array_growth_u8(u8 current_capacity, u64 element_size);

#endif // !EXP_UTILITY_ARRAY_GROWTH_H
