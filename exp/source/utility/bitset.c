/**
 * Copyright (C) 2024 Cade Weinberg
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

#include "utility/bitset.h"
#include "utility/assert.h"

Bitset bitset_create() {
    Bitset bitset = {.bits = 0};
    return bitset;
}

bool bitset_empty(Bitset *bitset) {
    EXP_ASSERT(bitset != nullptr);
    return bitset->bits == 0;
}

static u32 bit_at(u8 position) {
    EXP_ASSERT(position < bitset_length());
    return 1u << (u32)position;
}

static u32 state_at(u8 position, bool state) {
    EXP_ASSERT(position < bitset_length());
    return (u32)state << (u32)position;
}

void bitset_set_bit(Bitset *bitset, u8 position) {
    EXP_ASSERT(bitset != nullptr);
    EXP_ASSERT(position < bitset_length());
    bitset->bits |= bit_at(position);
}

void bitset_clear_bit(Bitset *bitset, u8 position) {
    EXP_ASSERT(bitset != nullptr);
    EXP_ASSERT(position < bitset_length());
    bitset->bits &= ~bit_at(position);
}

void bitset_assign_bit(Bitset *bitset, u8 position, bool state) {
    EXP_ASSERT(bitset != nullptr);
    EXP_ASSERT(position < bitset_length());
    bitset->bits = (bitset->bits & ~bit_at(position)) |
                   (state_at(position, state) & bit_at(position));
}

bool bitset_check_bit(Bitset *bitset, u8 bit_position) {
    EXP_ASSERT(bitset != nullptr);
    EXP_ASSERT(bit_position < bitset_length());
    return (bitset->bits >> (u32)bit_position) & 1;
}
