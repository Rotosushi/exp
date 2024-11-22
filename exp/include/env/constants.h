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
#include "utility/io.h"

typedef struct Constants {
    u16 count;
    u16 capacity;
    Value *buffer;
} Constants;

Constants constants_create();
void constants_destroy(Constants *restrict values);
Operand constants_append(Constants *restrict values, Value value);
Value *constants_at(Constants *restrict values, u16 index);

struct Context;
void print_constants(Constants const *restrict values,
                     FILE *restrict file,
                     struct Context *restrict context);

#endif // !EXP_IMR_CONSTANTS_H
