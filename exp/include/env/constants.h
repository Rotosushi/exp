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

/**
 * @file env/constants.h
 */

#ifndef EXP_IMR_CONSTANTS_H
#define EXP_IMR_CONSTANTS_H
#include <stddef.h>

#include "imr/value.h"

typedef struct Constants {
    u32 count;
    u32 capacity;
    Value *buffer;
} Constants;

void constants_initialize(Constants *constants);
void constants_terminate(Constants *constants);

Value *constants_at(Constants *constants, u32 constant);
u32 constants_append_tuple(Constants *constants, Tuple tuple);

#endif // !EXP_IMR_CONSTANTS_H
