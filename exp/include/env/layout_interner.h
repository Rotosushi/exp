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
#ifndef EXP_ENV_LAYOUT_INTERNER_H
#define EXP_ENV_LAYOUT_INTERNER_H

#include "imr/layout.h"

typedef struct LayoutListElement {
    struct Type const *type;
    Layout            *layout;
} LayoutListElement;

typedef struct LayoutList {
    u32                length;
    u32                capacity;
    LayoutListElement *buffer;
} LayoutList;

typedef struct LayoutInterner {
    Layout     nil;
    Layout     bool_;
    Layout     u8_;
    Layout     u16_;
    Layout     u32_;
    Layout     u64_;
    Layout     i8_;
    Layout     i16_;
    Layout     i32_;
    Layout     i64_;
    Layout     function;
    LayoutList padding;
    LayoutList tuple;
} LayoutInterner;

struct Context;
void layout_interner_create(LayoutInterner *restrict interner,
                            struct Context *restrict context);
void layout_interner_destroy(LayoutInterner *restrict interner);

Layout const *layout_interner_get_padding(LayoutInterner *restrict interner,
                                          u64 length);

Layout const *layout_interner_layout_of(LayoutInterner *restrict interner,
                                        struct Type const *type);

#endif // !EXP_ENV_LAYOUT_INTERNER_H
