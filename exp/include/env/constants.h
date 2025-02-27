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
    u32 count;
    u32 capacity;
    Value *buffer;
} Constants;

void constants_initialize(Constants *constants);
void constants_terminate(Constants *constants);
Operand constants_append(Constants *constants, Value value);
Value *constants_at(Constants *constants, u32 index);

struct Context;
void print_constants(Constants const *constants,
                     FILE *file,
                     struct Context *context);

#endif // !EXP_IMR_CONSTANTS_H
