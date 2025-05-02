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
#ifndef EXP_IMR_CONSTANTS_H
#define EXP_IMR_CONSTANTS_H
#include <stddef.h>

#include "imr/value.h"

typedef struct ConstantList {
    u32     size;
    u32     capacity;
    Value **buffer;
} ConstantList;

typedef struct Constants {
    Value       *nil;
    Value       *true_;
    Value       *false_;
    ConstantList u8_list;
    ConstantList u16_list;
    ConstantList u32_list;
    ConstantList u64_list;
    ConstantList i8_list;
    ConstantList i16_list;
    ConstantList i32_list;
    ConstantList i64_list;
    ConstantList tuple_list;
    ConstantList function_list;
} Constants;

void constants_create(Constants *restrict constants);
void constants_destroy(Constants *restrict constants);

Value const *constants_nil(Constants *restrict constants);
Value const *constants_true(Constants *restrict constants);
Value const *constants_false(Constants *restrict constants);
Value const *constants_u8(Constants *restrict constants, u8 u8_);
Value const *constants_u16(Constants *restrict constants, u16 u16_);
Value const *constants_u32(Constants *restrict constants, u32 u32_);
Value const *constants_u64(Constants *restrict constants, u64 u64_);
Value const *constants_i8(Constants *restrict constants, i8 i8_);
Value const *constants_i16(Constants *restrict constants, i16 i16_);
Value const *constants_i32(Constants *restrict constants, i32 i32_);
Value const *constants_i64(Constants *restrict constants, i64 i64_);
Value const *constants_tuple(Constants *restrict constants, Value *tuple);
Value const *constants_function(Constants *restrict constants, Value *function);

#endif // !EXP_IMR_CONSTANTS_H
