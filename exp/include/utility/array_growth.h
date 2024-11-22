// Copyright (C) 2024 Cade Weinberg
//
// This file is part of exp.
//
// exp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// exp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with exp.  If not, see <https://www.gnu.org/licenses/>.
#ifndef EXP_UTILITY_ARRAY_GROWTH_H
#define EXP_UTILITY_ARRAY_GROWTH_H
#include "utility/int_types.h"

typedef struct Growth {
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
