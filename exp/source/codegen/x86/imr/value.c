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
// along with exp.  If not, see <http://www.gnu.org/licenses/>.

#include "codegen/x86/imr/value.h"
#include "env/context.h"
#include "support/allocation.h"

x86_Value *x86_value_allocate() {
    x86_Value *value = allocate(sizeof(x86_Value));
    value->kind      = X86_VALUE_KIND_UNINITIALIZED;
    value->nil       = 0;
    value->type      = NULL;
    return value;
}

void x86_value_deallocate(x86_Value *restrict value) {
    EXP_ASSERT(value != NULL);
    switch (value->kind) {
    case X86_VALUE_KIND_TUPLE: {
        x86_tuple_destroy(&value->tuple);
        break;
    }

    case X86_VALUE_KIND_FUNCTION: {
        x86_function_destroy(&value->function);
        break;
    }

    // other kinds of value do not dynamically allocate any more memory.
    default: break;
    }

    deallocate(value);
}

x86_Value *x86_value_allocate_nil(struct Context *restrict context) {
    x86_Value *value = x86_value_allocate();
    value->kind      = X86_VALUE_KIND_NIL;
    value->nil       = 0;
    value->type      = context_nil_type(context);
    return value;
}

x86_Value *x86_value_allocate_bool(bool b, struct Context *restrict context) {
    x86_Value *value = x86_value_allocate();
    value->kind      = X86_VALUE_KIND_BOOL;
    value->bool_     = b;
    value->type      = context_bool_type(context);
    return value;
}

x86_Value *x86_value_allocate_u8(u8 u, struct Context *restrict context) {
    x86_Value *value = x86_value_allocate();
    value->kind      = X86_VALUE_KIND_U8;
    value->u8_       = u;
    value->type      = context_u8_type(context);
    return value;
}

x86_Value *x86_value_allocate_u16(u16 u, struct Context *restrict context) {
    x86_Value *value = x86_value_allocate();
    value->kind      = X86_VALUE_KIND_U16;
    value->u16_      = u;
    value->type      = context_u16_type(context);
    return value;
}

x86_Value *x86_value_allocate_u32(u32 u, struct Context *restrict context) {
    x86_Value *value = x86_value_allocate();
    value->kind      = X86_VALUE_KIND_U32;
    value->u32_      = u;
    value->type      = context_u32_type(context);
    return value;
}

x86_Value *x86_value_allocate_u64(u64 u, struct Context *restrict context) {
    x86_Value *value = x86_value_allocate();
    value->kind      = X86_VALUE_KIND_U64;
    value->u64_      = u;
    value->type      = context_u64_type(context);
    return value;
}

x86_Value *x86_value_allocate_i8(i8 i, struct Context *restrict context) {
    x86_Value *value = x86_value_allocate();
    value->kind      = X86_VALUE_KIND_I8;
    value->i8_       = i;
    value->type      = context_i8_type(context);
    return value;
}

x86_Value *x86_value_allocate_i16(i16 i, struct Context *restrict context) {
    x86_Value *value = x86_value_allocate();
    value->kind      = X86_VALUE_KIND_I16;
    value->i16_      = i;
    value->type      = context_i16_type(context);
    return value;
}

x86_Value *x86_value_allocate_i32(i32 i, struct Context *restrict context) {
    x86_Value *value = x86_value_allocate();
    value->kind      = X86_VALUE_KIND_I32;
    value->i32_      = i;
    value->type      = context_i32_type(context);
    return value;
}

x86_Value *x86_value_allocate_i64(i64 i, struct Context *restrict context) {
    x86_Value *value = x86_value_allocate();
    value->kind      = X86_VALUE_KIND_I64;
    value->i64_      = i;
    value->type      = context_i64_type(context);
    return value;
}

x86_Value *x86_value_allocate_tuple(x86_Tuple tuple,
                                    x86_Function *restrict function,
                                    struct Context *restrict context) {
    x86_Value *value = x86_value_allocate();
    value->kind      = X86_VALUE_KIND_NIL;
    value->tuple     = tuple;
    value->type      = context_nil_type(context);
    return value;
}

x86_Value *x86_value_allocate_function(x86_Function function,
                                       struct Context *restrict context);
