// Copyright (C) 2026 Cade Weinberg
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
#ifndef EXP_CODEGEN_X86_ENV_CONSTANTS_H
#define EXP_CODEGEN_X86_ENV_CONSTANTS_H

#include "codegen/x86/imr/value.h"

typedef struct x86_ConstantList {
    u32         size;
    u32         capacity;
    x86_Value **buffer;
} x86_ConstantList;

typedef struct x86_Constants {
    x86_Value       *nil;
    x86_Value       *true_;
    x86_Value       *false_;
    x86_ConstantList u8_list;
    x86_ConstantList u16_list;
    x86_ConstantList u32_list;
    x86_ConstantList u64_list;
    x86_ConstantList i8_list;
    x86_ConstantList i16_list;
    x86_ConstantList i32_list;
    x86_ConstantList i64_list;
    x86_ConstantList tuple_list;
    x86_ConstantList function_list;
} x86_Constants;

struct x86_Context;

void x86_constants_create(x86_Constants *restrict constants,
                          struct x86_Context *restrict context);
void x86_constants_destroy(x86_Constants *restrict constants);

x86_Value const *x86_constants_nil(x86_Constants *restrict constants);
x86_Value const *x86_constants_true(x86_Constants *restrict constants);
x86_Value const *x86_constants_false(x86_Constants *restrict constants);
x86_Value const *x86_constants_u8(x86_Constants *restrict constants,
                                  u8 u8_,
                                  struct x86_Context *restrict context);
x86_Value const *x86_constants_u16(x86_Constants *restrict constants,
                                   u16 u16_,
                                   struct x86_Context *restrict context);
x86_Value const *x86_constants_u32(x86_Constants *restrict constants,
                                   u32 u32_,
                                   struct x86_Context *restrict context);
x86_Value const *x86_constants_u64(x86_Constants *restrict constants,
                                   u64 u64_,
                                   struct x86_Context *restrict context);
x86_Value const *x86_constants_i8(x86_Constants *restrict constants,
                                  i8 i8_,
                                  struct x86_Context *restrict context);
x86_Value const *x86_constants_i16(x86_Constants *restrict constants,
                                   i16 i16_,
                                   struct x86_Context *restrict context);
x86_Value const *x86_constants_i32(x86_Constants *restrict constants,
                                   i32 i32_,
                                   struct x86_Context *restrict context);
x86_Value const *x86_constants_i64(x86_Constants *restrict constants,
                                   i64 i64_,
                                   struct x86_Context *restrict context);
x86_Value const *x86_constants_tuple(x86_Constants *restrict constants,
                                     x86_Tuple tuple,
                                     x86_Function *restrict function,
                                     struct x86_Context *restrict context);
x86_Value const *x86_constants_function(x86_Constants *restrict constants,
                                        x86_Function function,
                                        struct x86_Context *restrict context);

#endif // !EXP_CODEGEN_X86_ENV_CONSTANTS_H
