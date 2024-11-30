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

Bitset bitset_create() {
    Bitset B = {.bits = 0};
    return B;
}

void bitset_set_bit(Bitset *bitset, u8 bit_position) {
    bitset->bits |= (1ul << (u64)(bit_position));
}

void bitset_clear_bit(Bitset *bitset, u8 bit_position) {
    bitset->bits &= ~(1ul << (u64)bit_position);
}

bool bitset_check_bit(Bitset *bitset, u8 bit_position) {
    return (bitset->bits >> (u64)bit_position) & 1;
}
