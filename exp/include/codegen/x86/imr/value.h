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
#ifndef EXP_CODEGEN_X86_IMR_VALUE_H
#define EXP_CODEGEN_X86_IMR_VALUE_H

#include "codegen/x86/imr/value/function.h"
#include "codegen/x86/imr/value/tuple.h"
#include "imr/value.h"

typedef enum x86_ValueKind {
    X86_VALUE_KIND_UNINITIALIZED,
    X86_VALUE_KIND_NIL,
    X86_VALUE_KIND_BOOL,
    X86_VALUE_KIND_U8,
    X86_VALUE_KIND_U16,
    X86_VALUE_KIND_U32,
    X86_VALUE_KIND_U64,
    X86_VALUE_KIND_I8,
    X86_VALUE_KIND_I16,
    X86_VALUE_KIND_I32,
    X86_VALUE_KIND_I64,
    X86_VALUE_KIND_TUPLE,
    X86_VALUE_KIND_FUNCTION,
} x86_ValueKind;

typedef struct x86_Value {
    x86_ValueKind kind;
    union {
        bool         nil;
        bool         bool_;
        u8           u8_;
        u16          u16_;
        u32          u32_;
        u64          u64_;
        i8           i8_;
        i16          i16_;
        i32          i32_;
        i64          i64_;
        x86_Tuple    tuple;
        x86_Function function;
    };
    Type const *type;
} x86_Value;

x86_Value *x86_value_allocate();
void       x86_value_deallocate(x86_Value *restrict value);

x86_Value *x86_value_allocate_nil(struct Context *restrict context);
x86_Value *x86_value_allocate_bool(bool b, struct Context *restrict context);
x86_Value *x86_value_allocate_u8(u8 u, struct Context *restrict context);
x86_Value *x86_value_allocate_u16(u16 u, struct Context *restrict context);
x86_Value *x86_value_allocate_u32(u32 u, struct Context *restrict context);
x86_Value *x86_value_allocate_u64(u64 u, struct Context *restrict context);
x86_Value *x86_value_allocate_i8(i8 i, struct Context *restrict context);
x86_Value *x86_value_allocate_i16(i16 i, struct Context *restrict context);
x86_Value *x86_value_allocate_i32(i32 i, struct Context *restrict context);
x86_Value *x86_value_allocate_i64(i64 i, struct Context *restrict context);
x86_Value *x86_value_allocate_tuple(x86_Tuple tuple,
                                    x86_Function *restrict function,
                                    struct Context *restrict context);
x86_Value *x86_value_allocate_function(x86_Function function,
                                       struct Context *restrict context);

#endif // !EXP_CODEGEN_X86_IMR_VALUE_H
