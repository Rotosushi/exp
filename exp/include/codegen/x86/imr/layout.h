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

/*
 * How large of a size and alignment do we need for a given
 * scalar type? Well, even at the largest scalar type I can think
 * of; dvec4, dmat4, The size is 32, 128 respectively, and the
 * natural alignment is 8 for both. The only time we need larger
 * alignment is when we are trying to align the object to a cache
 * lane. This simply requires user-specified alignment however.
 * I expect that a u32 for both is more than enough. I am willing
 * to be wrong.
 * Composite types have an upper limit of a u64 though?
 * for instance an array type? I mean, we could allow larger, we could
 * make the size integer a memory backed integer, where the only
 * limit is the amount of memory you have. but just to put this into
 * some perspective,
 * u64::max -> 18_446_744_073_709_551_615
 * which is 4? exbibytes of data. in a single array.
 * maybe a modern super-computer would have need for a larger array,
 * but well before that point I would think you gain speed by breaking
 * into smaller components and processing in parallel?
 * With the way I have defined scalar, a 128bit word sized CPU would
 * simply be the new upper limit on the size allowed for a primitive
 * array. oh yeah, you ask for this allocation from the Operating
 * System. do you even have an exbibyte of storage on a single machine?
 * does anyone?
 * (They had this conversation with 32bit word sized CPUs too)
 * yeah, I know. A floppy disk (1.44 Mibibytes) used to be enough to
 * get your work done.
 *
 * Well, we can make a ScalarLayout two u64's with no added space to the
 * unified Layout structure, so why not just do that? We pay nothing more
 * Than we already are, and we gain a bit of flexibility I don't see
 * a use for, but is there if it's needed.
 */

typedef struct x86_ScalarLayout {
    u64 size;
    u64 alignment;
} x86_ScalarLayout;

struct x86_Layout;

typedef struct x86_TupleLayout {
    u64                       alignment;
    u64                       size;
    u32                       length;
    u32                       capacity;
    struct x86_Layout const **buffer;
} x86_TupleLayout;

typedef enum x86_LayoutKind {
    X86_LAYOUT_KIND_SCALAR,
    X86_LAYOUT_KIND_PADDING,
    X86_LAYOUT_KIND_TUPLE,
} x86_LayoutKind;

typedef union x86_LayoutData {
    x86_ScalarLayout scalar;
    u64              padding;
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
    x86_LayoutList paddings;
    x86_LayoutList tuples;
} x86_Layouts;

void x86_layouts_create(x86_Layouts *restrict layouts);
void x86_layouts_destroy(x86_Layouts *restrict layouts);

x86_Layout const *x86_layouts_layout_of_type(x86_Layouts *restrict layouts,
                                             Type const *type);

#endif // !EXP_CODEGEN_X86_IMR_LAYOUT_H
