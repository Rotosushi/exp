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

#ifndef EXP_CODEGEN_X86_IMR_TUPLE_H
#define EXP_CODEGEN_X86_IMR_TUPLE_H

#include "codegen/x86/imr/operand.h"
#include "imr/tuple.h"

/*
 * #NOTE: I want to implement Tuples in x86, This means we essentially
 * need to implement a c-style struct type. We need to account for
 * the size and alignment of each successive element in the tuple,
 * Which means adding padding if necessary. My initial thought is to
 * explicitly specify padding by adding it as elements to the tuple.
 * So we can do a simple iteration over the elements and directly translate
 * each element into a .byte, .short, .int, or .quad, directive.
 * and translate padding/uninitialized elements into a .zero directive.
 *
 * This would work, but can this work for local tuples too?
 *
 */

typedef struct x86_Tuple {
    u32          size;
    u32          capacity;
    x86_Operand *elements;
} x86_Tuple;

void x86_tuple_create(x86_Tuple *restrict x86_tuple,
                      Tuple const *restrict tuple);

void x86_tuple_destroy(x86_Tuple *restrict x86_tuple);

#endif // !EXP_CODEGEN_X86_IMR_TUPLE_H
