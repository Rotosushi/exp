/**
 * Copyright (C) 2025 cade
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

#include "support/bitset.h"

i32 bitset_tests([[maybe_unused]] i32 argc, [[maybe_unused]] char **argv) {
    i32 result = 0;

    Bitset bitset = bitset_create();
    bitset_set(&bitset, 0);
    bitset_set(&bitset, 1);

    result += (bitset_check(&bitset, 0) == true) ? 0 : 1;
    result += (bitset_check(&bitset, 1) == true) ? 0 : 1;
    result += (bitset_check(&bitset, 2) == false) ? 0 : 1;
    result += (bitset_check(&bitset, 3) == false) ? 0 : 1;

    bitset_clear(&bitset, 0);
    bitset_clear(&bitset, 1);
    result += (bitset_check(&bitset, 0) == false) ? 0 : 1;
    result += (bitset_check(&bitset, 1) == false) ? 0 : 1;

    return result;
}
