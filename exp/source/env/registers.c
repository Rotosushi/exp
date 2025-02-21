/**
 * Copyright (C) 2025 Cade Weinberg
 *
 * This file is part of exp.
 *
 * exp is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * exp is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with exp.  If not, see <https://www.gnu.org/licenses/>.
 */

/**
 * @file env/registers.c
 */

#include "env/registers.h"
#include "utility/assert.h"
#include "utility/int_types.h"

void registers_initialize(Registers *registers) {
    EXP_ASSERT(registers != nullptr);
    registers->active = bitset_create();
    for (u8 index = 0; index < bitset_length(); ++index) {
        registers->registers[index] = scalar_uninitialized();
    }
}

void registers_terminate(Registers *registers) {
    EXP_ASSERT(registers != nullptr);
    registers_initialize(registers);
}

bool registers_full(Registers *registers) {
    EXP_ASSERT(registers != nullptr);
    return ~registers->active.bits == 0;
}

bool registers_register_available(Registers *registers, u8 register_) {
    EXP_ASSERT(registers != nullptr);
    EXP_ASSERT(register_ < bitset_length());
    return !bitset_check_bit(&registers->active, register_);
}

bool registers_next_available(Registers *registers, u8 *register_) {
    EXP_ASSERT(registers != nullptr);
    u32 bits  = ~registers->active.bits;
    i32 index = __builtin_ctz(bits);
    EXP_ASSERT(index < bitset_length());
    if (index == 0) { return false; }
    *register_ = (u8)index;
    return true;
}

void registers_set(Registers *registers, u8 register_, Scalar value) {
    EXP_ASSERT(registers != nullptr);
    EXP_ASSERT(register_ < bitset_length());
    bitset_set_bit(&registers->active, register_);
    registers->registers[register_] = value;
}

Scalar registers_get(Registers *registers, u8 register_) {
    EXP_ASSERT(registers != nullptr);
    EXP_ASSERT(register_ < bitset_length());
    return registers->registers[register_];
}

Scalar registers_unset(Registers *registers, u8 register_) {
    EXP_ASSERT(registers != nullptr);
    EXP_ASSERT(register_ < bitset_length());
    bitset_clear_bit(&registers->active, register_);
    Scalar value                    = registers->registers[register_];
    registers->registers[register_] = scalar_uninitialized();
    return value;
}
