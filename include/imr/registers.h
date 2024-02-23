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
#ifndef EXP_IMR_REGISTERS_H
#define EXP_IMR_REGISTERS_H
#include <stdint.h>

#include "imr/value.h"

#define REGISTERS_CAPACITY 32

/**
 * @brief represents the working registers of
 * the virtual machine
 *
 */
typedef struct Registers {
  Value buffer[REGISTERS_CAPACITY];
} Registers;

/**
 * @brief return the <Value*> of the register at the
 * given <index>
 *
 * @param registers
 * @param index
 * @return Value*
 */
Value *registers_at(Registers *restrict registers, uint8_t index);

#endif // !EXP_IMR_REGISTERS_H