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

#ifndef EXP_CODEGEN_X86_IMR_LAYOUT_H
#define EXP_CODEGEN_X86_IMR_LAYOUT_H

#include "imr/type.h"

/*
 * #NOTE: We need the concept of a Layout for the Tuple type,
 * And each scalar type also has a layout. We just use a different
 * method of storing them. Currently we use hardcoded integer literals
 * denoting the size and alignment held within switch statements.
 * However there is no reason why we cannot use a similar strategy
 * to Types, where we have an abstract "Layout" struct, which holds
 * This data for each type, and since Layout is a constant thing,
 * Per type, we can intern these layouts as well.
 */

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
 * If the strategy for local tuples is to create a unique global
 * initializer tuple, and copy it's data into the local space.
 * Then yes, this can be used to properly initialize a tuple on the
 * stack.
 *
 * It is an important observation that the size_of, and align_of
 * functions will both need to be computed via the x86_TupleLayout
 * object, and not a TupleType, as they are now. With the currently
 * available function signatures we would be forced into constructing
 * the layout fresh each time we wanted to query it's properties.
 * This is obviously inefficent, however the way around this is some
 * form of dynamic storage which caches the results, and can be queried
 * to retrieve the Layout. Similar to the API for Constants and Types
 * within the context. Which essentially needs an x86_Context again.
 * Can we smuggle a pointer to such a structure through the Target?
 * It has to be a void pointer to be general to any possible target, but
 * yes.
 *
 */

typedef struct x86_ScalarLayout {
    u8 size;
    u8 alignment;
} x86_ScalarLayout;

typedef enum x86_TupleLayoutElementKind {
    X86_TUPLE_LAYOUT_ELEMENT_KIND_TUPLE,
    X86_TUPLE_LAYOUT_ELEMENT_KIND_PADDING,
    X86_TUPLE_LAYOUT_ELEMENT_KIND_SCALAR,
} x86_TupleLayoutElementKind;

struct x86_TupleLayout;

typedef union x86_TupleLayoutElementData {
    struct x86_TupleLayout const *tuple;
    u8                            padding;
    x86_ScalarLayout              scalar;
} x86_TupleLayoutElementData;

typedef struct x86_TupleLayoutElement {
    x86_TupleLayoutElementKind kind;
    x86_TupleLayoutElementData data;
} x86_TupleLayoutElement;

typedef struct x86_TupleLayout {
    u64                     alignment;
    u64                     size;
    u32                     length;
    u32                     capacity;
    x86_TupleLayoutElement *buffer;
} x86_TupleLayout;

typedef enum x86_LayoutKind {
    X86_LAYOUT_KIND_SCALAR,
    X86_LAYOUT_KIND_TUPLE,
} x86_LayoutKind;

typedef union x86_LayoutData {
    x86_ScalarLayout scalar;
    x86_TupleLayout  tuple;
} x86_LayoutData;

typedef struct x86_Layout {
    x86_LayoutKind kind;
    x86_LayoutData data;
} x86_Layout;

u64 x86_layout_size_of(x86_Layout const *restrict layout);
u64 x86_layout_align_of(x86_Layout const *restrict layout);

typedef struct x86_LayoutListElement {
    Type const *type;
    x86_Layout *layout;
} x86_LayoutListElement;

typedef struct x86_LayoutList {
    u32                    length;
    u32                    capacity;
    x86_LayoutListElement *buffer;
} x86_LayoutList;

typedef struct x86_Layouts {
    x86_Layout     nil;
    x86_Layout     bool_;
    x86_Layout     u8_;
    x86_Layout     u16_;
    x86_Layout     u32_;
    x86_Layout     u64_;
    x86_Layout     i8_;
    x86_Layout     i16_;
    x86_Layout     i32_;
    x86_Layout     i64_;
    x86_LayoutList tuples;
} x86_Layouts;

void x86_layouts_create(x86_Layouts *restrict layouts);
void x86_layouts_destroy(x86_Layouts *restrict layouts);

x86_Layout const *x86_layouts_layout_of(x86_Layouts *restrict layouts,
                                        Type const *type);

#endif // !EXP_CODEGEN_X86_IMR_LAYOUT_H
