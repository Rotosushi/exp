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
#include "codegen/GAS/directives.h"
#include "support/allocation.h"
#include "support/unreachable.h"

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

x86_Value *x86_value_allocate_nil(Type const *type) {
    x86_Value *value = x86_value_allocate();
    value->kind      = X86_VALUE_KIND_NIL;
    value->nil       = 0;
    value->type      = type;
    return value;
}

x86_Value *x86_value_allocate_bool(bool b, Type const *type) {
    x86_Value *value = x86_value_allocate();
    value->kind      = X86_VALUE_KIND_BOOL;
    value->bool_     = b;
    value->type      = type;
    return value;
}

x86_Value *x86_value_allocate_u8(u8 u, Type const *type) {
    x86_Value *value = x86_value_allocate();
    value->kind      = X86_VALUE_KIND_U8;
    value->u8_       = u;
    value->type      = type;
    return value;
}

x86_Value *x86_value_allocate_u16(u16 u, Type const *type) {
    x86_Value *value = x86_value_allocate();
    value->kind      = X86_VALUE_KIND_U16;
    value->u16_      = u;
    value->type      = type;
    return value;
}

x86_Value *x86_value_allocate_u32(u32 u, Type const *type) {
    x86_Value *value = x86_value_allocate();
    value->kind      = X86_VALUE_KIND_U32;
    value->u32_      = u;
    value->type      = type;
    return value;
}

x86_Value *x86_value_allocate_u64(u64 u, Type const *type) {
    x86_Value *value = x86_value_allocate();
    value->kind      = X86_VALUE_KIND_U64;
    value->u64_      = u;
    value->type      = type;
    return value;
}

x86_Value *x86_value_allocate_i8(i8 i, Type const *type) {
    x86_Value *value = x86_value_allocate();
    value->kind      = X86_VALUE_KIND_I8;
    value->i8_       = i;
    value->type      = type;
    return value;
}

x86_Value *x86_value_allocate_i16(i16 i, Type const *type) {
    x86_Value *value = x86_value_allocate();
    value->kind      = X86_VALUE_KIND_I16;
    value->i16_      = i;
    value->type      = type;
    return value;
}

x86_Value *x86_value_allocate_i32(i32 i, Type const *type) {
    x86_Value *value = x86_value_allocate();
    value->kind      = X86_VALUE_KIND_I32;
    value->i32_      = i;
    value->type      = type;
    return value;
}

x86_Value *x86_value_allocate_i64(i64 i, Type const *type) {
    x86_Value *value = x86_value_allocate();
    value->kind      = X86_VALUE_KIND_I64;
    value->i64_      = i;
    value->type      = type;
    return value;
}

x86_Value *x86_value_allocate_tuple(x86_Tuple tuple, Type const *type) {
    x86_Value *value = x86_value_allocate();
    value->kind      = X86_VALUE_KIND_NIL;
    value->tuple     = tuple;
    value->type      = type;
    return value;
}

x86_Value *x86_value_allocate_function(x86_Function function,
                                       Type const  *type) {
    x86_Value *value = x86_value_allocate();
    value->kind      = X86_VALUE_KIND_FUNCTION;
    value->function  = function;
    value->type      = type;
    return value;
}

bool x86_value_equal(x86_Value const *A, x86_Value const *B) {
    EXP_ASSERT(A != NULL);
    EXP_ASSERT(B != NULL);
    if (A == B) { return true; }
    if (A->kind != B->kind) { return false; }

    switch (A->kind) {
    case X86_VALUE_KIND_UNINITIALIZED:
        return type_equality(A->type, B->type);
        break;
    case X86_VALUE_KIND_NIL: return true;

    case X86_VALUE_KIND_BOOL: {
        return A->bool_ == B->bool_;
    }

    case X86_VALUE_KIND_U8: {
        return A->u8_ == B->u8_;
    }

    case X86_VALUE_KIND_U16: {
        return A->u16_ == B->u16_;
    }

    case X86_VALUE_KIND_U32: {
        return A->u32_ == B->u32_;
    }

    case X86_VALUE_KIND_U64: {
        return A->u64_ == B->u64_;
    }

    case X86_VALUE_KIND_I8: {
        return A->i8_ == B->i8_;
    }

    case X86_VALUE_KIND_I16: {
        return A->i16_ == B->i16_;
    }

    case X86_VALUE_KIND_I32: {
        return A->i32_ == B->i32_;
    }

    case X86_VALUE_KIND_I64: {
        return A->i64_ == B->i64_;
    }

    case X86_VALUE_KIND_TUPLE: {
        return x86_tuple_equal(&A->tuple, &B->tuple);
    }

    case X86_VALUE_KIND_FUNCTION: {
        return A == B;
    }

    default: EXP_UNREACHABLE();
    }
}

void print_x86_value(String *restrict buffer, x86_Value const *restrict value) {
    EXP_ASSERT(value != NULL);
    EXP_ASSERT(buffer != NULL);
    Layout const *layout = value->type->layout;
    switch (value->kind) {
    case X86_VALUE_KIND_UNINITIALIZED:
        gas_directive_zero(layout_size_of(layout), buffer);
        break;
    case X86_VALUE_KIND_NIL:  gas_directive_u8(0, buffer); break;
    case X86_VALUE_KIND_BOOL: gas_directive_u8(value->bool_, buffer); break;
    case X86_VALUE_KIND_U8:   gas_directive_u8(value->u8_, buffer); break;
    case X86_VALUE_KIND_U16:  gas_directive_u16(value->u16_, buffer); break;
    case X86_VALUE_KIND_U32:  gas_directive_u32(value->u32_, buffer); break;
    case X86_VALUE_KIND_U64:  gas_directive_u64(value->u64_, buffer); break;
    case X86_VALUE_KIND_I8:   gas_directive_i8(value->i8_, buffer); break;
    case X86_VALUE_KIND_I16:  gas_directive_i16(value->i16_, buffer); break;
    case X86_VALUE_KIND_I32:  gas_directive_i32(value->i32_, buffer); break;
    case X86_VALUE_KIND_I64:  gas_directive_i64(value->i64_, buffer); break;
    case X86_VALUE_KIND_TUPLE:
        print_x86_tuple(buffer, &value->tuple, value->type);
        break;
    case X86_VALUE_KIND_FUNCTION:
        print_x86_function(buffer, &value->function);
        break;
    default: EXP_UNREACHABLE();
    }
}
