/**
 * Copyright (C) 2025 Cade Weinberg
 *
 * This file is part of exp.
 *
 * exp is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * exp is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with exp.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "imr/layout.h"
#include "support/unreachable.h"

void layout_create_primary(Layout *restrict layout, LayoutPrimary primary) {
    layout->kind         = LAYOUT_KIND_PRIMARY;
    layout->data.primary = primary;
}

void layout_create_padding(Layout *restrict layout, u64 length) {
    layout->kind         = LAYOUT_KIND_PADDING;
    layout->data.padding = length;
}

void layout_create_tuple(Layout *restrict layout,
                         TypeTuple const *restrict tuple,
                         struct LayoutInterner *restrict interner) {
    layout->kind = LAYOUT_KIND_TUPLE;
    layout_tuple_create(&layout->data.tuple, tuple, interner);
}

void layout_create_function(Layout *restrict layout, LayoutPrimary primary) {
    layout->kind         = LAYOUT_KIND_FUNCTION;
    layout->data.primary = primary;
}

void layout_destroy(Layout *restrict layout) {
    switch (layout->kind) {
    case LAYOUT_KIND_TUPLE: layout_tuple_destroy(&layout->data.tuple);

    default: break; // no other layout kinds dynamically allocate.
    }
}

u64 layout_size_of(Layout const *restrict layout) {
    switch (layout->kind) {
    case LAYOUT_KIND_PRIMARY:  return layout->data.primary.size;
    case LAYOUT_KIND_TUPLE:    return layout->data.tuple.primary.size;
    case LAYOUT_KIND_PADDING:  return layout->data.padding;
    case LAYOUT_KIND_FUNCTION: return layout->data.primary.size;

    default: EXP_UNREACHABLE();
    }
}

u64 layout_align_of(Layout const *restrict layout) {
    switch (layout->kind) {
    case LAYOUT_KIND_PRIMARY:  return layout->data.primary.alignment;
    case LAYOUT_KIND_TUPLE:    return layout->data.tuple.primary.alignment;
    case LAYOUT_KIND_PADDING:  return 1;
    case LAYOUT_KIND_FUNCTION: return layout->data.primary.alignment;

    default: EXP_UNREACHABLE();
    }
}
