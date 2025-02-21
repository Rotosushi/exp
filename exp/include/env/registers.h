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
 * @file env/registers.h
 */

#ifndef EXP_ENV_REGISTERS_H
#define EXP_ENV_REGISTERS_H

#include "imr/scalar.h"
#include "utility/bitset.h"

/**
 * @brief Models the registers of the abstract machine.
 *
 * @note we use Scalar instead of Value for our register values
 * to more closely align the language with how 64 bit registers
 * work on a real machine. I hope this makes it easier to translate
 * the language to real machine code.
 *
 * @var active: a bitset representing which registers are currently in use.
 * @var registers: an array of Scalar values representing the registers.
 */
typedef struct Registers {
    Bitset active;
    Scalar registers[bitset_length()];
} Registers;

void registers_initialize(Registers *registers);
void registers_terminate(Registers *registers);

bool registers_register_available(Registers *registers, u8 register_);
bool registers_next_available(Registers *registers, u8 *register_);

void registers_set(Registers *registers, u8 register_, Scalar value);
Scalar registers_get(Registers *registers, u8 register_);
Scalar registers_unset(Registers *registers, u8 register_);

#endif // !EXP_ENV_REGISTERS_H
