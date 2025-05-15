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

#include "codegen/x86/value.h"
#include "codegen/GAS/directives.h"
#include "support/assert.h"
#include "support/unreachable.h"

static void print_x86_tuple(String *restrict buffer,
                            Value const *restrict value,
                            x86_Layout const *layout);

void print_x86_value(String *restrict buffer,
                     Value const *restrict value,
                     x86_Layout const *layout) {
    switch (value->kind) {
    case VALUE_KIND_UNINITIALIZED:
        gas_directive_zero(x86_layout_size_of(layout), buffer);
        break;
    case VALUE_KIND_NIL:   gas_directive_u8(0, buffer); break;
    case VALUE_KIND_BOOL:  gas_directive_u8(value->bool_, buffer); break;
    case VALUE_KIND_U8:    gas_directive_u8(value->u8_, buffer); break;
    case VALUE_KIND_U16:   gas_directive_u16(value->u16_, buffer); break;
    case VALUE_KIND_U32:   gas_directive_u32(value->u32_, buffer); break;
    case VALUE_KIND_U64:   gas_directive_u64(value->u64_, buffer); break;
    case VALUE_KIND_I8:    gas_directive_i8(value->i8_, buffer); break;
    case VALUE_KIND_I16:   gas_directive_i16(value->i16_, buffer); break;
    case VALUE_KIND_I32:   gas_directive_i32(value->i32_, buffer); break;
    case VALUE_KIND_I64:   gas_directive_i64(value->i64_, buffer); break;
    case VALUE_KIND_TUPLE: print_x86_tuple(buffer, value, layout); break;
    default:               EXP_UNREACHABLE();
    }
}

static void print_x86_element(String *restrict buffer,
                              Operand           element,
                              x86_Layout const *layout);

static void print_x86_tuple(String *restrict buffer,
                            Value const *restrict value,
                            x86_Layout const *layout) {
    Tuple const *tuple = &value->tuple;
    // It is safe to access this as if it was a tuple layout,
    // unless I made a silly mistake, or some memory was corrupted.
    exp_assert_debug(layout->kind == X86_LAYOUT_KIND_TUPLE);
    x86_TupleLayout const *tuple_layout = &layout->data.tuple;
    // So, the length of the tuple and it's layout should be the "same"
    // except that the layout takes into account the padding that must be
    // present between elements. and these take up element slots.
    // Thus the layout length is the same or greater than.
    for (u32 tuple_index = 0, layout_index = 0; tuple_index < tuple->length;
         ++tuple_index, ++layout_index) {
        Operand           element        = tuple->elements[tuple_index];
        x86_Layout const *element_layout = tuple_layout->buffer[layout_index];
        print_x86_element(buffer, element, element_layout);
        if (tuple_index >= (tuple->length - 1)) { continue; }

        exp_assert(layout_index < (tuple_layout->length - 1));
        x86_Layout const *next_layout = tuple_layout->buffer[layout_index + 1];
        if (next_layout->kind != X86_LAYOUT_KIND_PADDING) { continue; }

        gas_directive_zero(next_layout->data.padding, buffer);
        ++layout_index;
    }
}

static void print_x86_element(String *restrict buffer,
                              Operand           element,
                              x86_Layout const *layout) {
    switch (element.kind) {
    case OPERAND_KIND_SSA:
    case OPERAND_KIND_LABEL:
        PANIC("Not an allowable initializer for a global tuple");
        break;

    case OPERAND_KIND_CONSTANT:
        print_x86_value(buffer, element.data.constant, layout);
        break;

    case OPERAND_KIND_NIL:  gas_directive_u8(0, buffer); break;
    case OPERAND_KIND_BOOL: gas_directive_u8(element.data.bool_, buffer); break;
    case OPERAND_KIND_U8:   gas_directive_u8(element.data.u8_, buffer); break;
    case OPERAND_KIND_U16:  gas_directive_u16(element.data.u16_, buffer); break;
    case OPERAND_KIND_U32:  gas_directive_u32(element.data.u32_, buffer); break;
    case OPERAND_KIND_U64:  gas_directive_u64(element.data.u64_, buffer); break;
    case OPERAND_KIND_I8:   gas_directive_i8(element.data.i8_, buffer); break;
    case OPERAND_KIND_I16:  gas_directive_i16(element.data.i16_, buffer); break;
    case OPERAND_KIND_I32:  gas_directive_i32(element.data.i32_, buffer); break;
    case OPERAND_KIND_I64:  gas_directive_i64(element.data.i64_, buffer); break;
    default:                EXP_UNREACHABLE();
    }
}
