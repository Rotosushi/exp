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
#ifndef EXP_IMR_LAYOUT_H
#define EXP_IMR_LAYOUT_H

#include "imr/layout/tuple.h"

/**
 * @note when we add support for floating point types,
 * vector types, and matrix types, these each will have
 * a kind of layout. Or, we can treat vector types as a
 * tuple of floating point types. But this raises questions
 * - how does the allocator assign two or more registers as the
 *   location of a larger tuple object? i.e. (i64, i64) being
 *   stored in rdx:rax.
 * - how does the allocator assign one register to hold more than
 *   one element of a given tuple object? i.e. (i32, i32) being
 *   held, packed, in rax. or (f32, f32, f32, f32) being held,
 *   packed, in XMM0.
 */

typedef enum LayoutKind {
    LAYOUT_KIND_PRIMARY,
    LAYOUT_KIND_PADDING,
    LAYOUT_KIND_TUPLE,
    LAYOUT_KIND_FUNCTION,
} LayoutKind;

typedef union LayoutData {
    LayoutPrimary primary;
    u64           padding;
    LayoutTuple   tuple;
} LayoutData;

typedef struct Layout {
    LayoutKind kind;
    LayoutData data;
} Layout;

void layout_create_primary(Layout *restrict layout, LayoutPrimary primary);
void layout_create_padding(Layout *restrict layout, u64 length);
void layout_create_tuple(Layout *restrict layout,
                         TypeTuple const *restrict tuple,
                         struct LayoutInterner *restrict interner);
void layout_create_function(Layout *restrict layout, LayoutPrimary primary);

void layout_destroy(Layout *restrict layout);

u64 layout_size_of(Layout const *restrict layout);
u64 layout_align_of(Layout const *restrict layout);

#endif // !EXP_IMR_LAYOUT_H
