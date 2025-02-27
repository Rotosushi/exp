/**
 * Copyright 2025 Cade Weinberg. All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
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
