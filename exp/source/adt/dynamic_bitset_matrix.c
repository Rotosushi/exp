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

#include "adt/dynamic_bitset_matrix.h"

void dynamic_bitset_matrix_create(DynamicBitsetMatrix *restrict matrix);
void dynamic_bitset_matrix_destroy(DynamicBitsetMatrix *restrict matrix);

void dynamic_bitset_matrix_resize(DynamicBitsetMatrix *restrict matrix,
                                  u64 rows,
                                  u64 columns);

void dynamic_bitset_matrix_set(DynamicBitsetMatrix *restrict matrix,
                               u64 row,
                               u64 column);
void dynamic_bitset_matrix_clear(DynamicBitsetMatrix *restrict matrix,
                                 u64 row,
                                 u64 column);
void dynamic_bitset_matrix_check(DynamicBitsetMatrix *restrict matrix,
                                 u64 row,
                                 u64 column);
