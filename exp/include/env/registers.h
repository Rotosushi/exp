// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
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

bool registers_next_available(Registers *registers, u8 *register_);

void registers_set(Registers *registers, u8 register_, Scalar value);
Scalar registers_get(Registers *registers, u8 register_);
Scalar registers_unset(Registers *registers, u8 register_);

#endif // !EXP_ENV_REGISTERS_H
