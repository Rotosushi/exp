// Copyright (C) 2025 Cade Weinberg
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
#ifndef EXP_IMR_LAYOUT_TUPLE_H
#define EXP_IMR_LAYOUT_TUPLE_H

#include "imr/layout/primary.h"
#include "imr/type/tuple.h"

struct Layout;

typedef struct LayoutTuple {
    LayoutPrimary         primary;
    u32                   length;
    u32                   capacity;
    struct Layout const **buffer;
} LayoutTuple;

struct LayoutInterner;

void layout_tuple_create(LayoutTuple *restrict layout,
                         TypeTuple const *restrict tuple,
                         struct LayoutInterner *restrict interner);

void layout_tuple_destroy(LayoutTuple *restrict layout);

u64 layout_tuple_get_element_offset(LayoutTuple const *restrict layout,
                                    u32 index);

#endif // EXP_IMR_LAYOUT_TUPLE_H
