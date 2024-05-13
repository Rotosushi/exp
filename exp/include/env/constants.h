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

#include "imr/instruction.h"
#include "imr/value.h"
#include "utility/io.h"

typedef struct Constants {
  u16 length;
  u16 capacity;
  Value *buffer;
} Constants;

/**
 * @brief create a Constants buffer
 *
 * @return Constants
 */
Constants constants_create();

/**
 * @brief destroy a Constants buffer
 *
 * @param constants
 */
void constants_destroy(Constants *restrict constants);

/**
 * @brief add a new constant to the Constants buffer
 *
 * @param constants
 * @param value
 * @return Value*
 */
Operand constants_add(Constants *restrict constants, Value value);

/**
 * @brief return the constant at the given index in the buffer
 *
 * @param constants
 * @param index
 * @return Value*
 */
Value *constants_at(Constants *restrict constants, u16 index);

void print_constants(Constants const *restrict c, FILE *restrict file);

#endif // !EXP_IMR_CONSTANTS_H