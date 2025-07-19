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
#include "support/allocation.h"
#include "support/assert.h"
#include "support/panic.h"

void dynamic_bitset_matrix_create(DynamicBitsetMatrix *restrict matrix) {
    exp_assert(matrix != NULL);
    matrix->rows    = 0;
    matrix->columns = 0;
    matrix->buffer  = NULL;
}

void dynamic_bitset_matrix_destroy(DynamicBitsetMatrix *restrict matrix) {
    deallocate(matrix->buffer);
    dynamic_bitset_matrix_create(matrix);
}

void dynamic_bitset_matrix_resize(DynamicBitsetMatrix *restrict matrix,
                                  u64 rows,
                                  u64 columns) {
    exp_assert(matrix != NULL);
    u64 alloc_size = 0;
    if (__builtin_mul_overflow(rows, columns, &alloc_size)) {
        PANIC("mul overflow");
    }

    if (__builtin_mul_overflow(
            alloc_size, sizeof(*matrix->buffer), &alloc_size)) {
        PANIC("mul overflow");
    }

    matrix->buffer  = reallocate(matrix->buffer, alloc_size);
    matrix->rows    = rows;
    matrix->columns = columns;
}

static u64 row_of_element(u64 row) { return row / 64; }

static u64 element_of_column(u64 column) { return column / 64; }

static u64 bit_of_column(u64 column) { return column % 64; }

void dynamic_bitset_matrix_set(DynamicBitsetMatrix *restrict matrix,
                               u64 row,
                               u64 column) {
    exp_assert(matrix != NULL);
    matrix->buffer[row_of_element(row) + element_of_column(column)] |=
        (1ULL << bit_of_column(column));
}

void dynamic_bitset_matrix_clear(DynamicBitsetMatrix *restrict matrix,
                                 u64 row,
                                 u64 column) {
    exp_assert(matrix != NULL);
    matrix->buffer[row_of_element(row) + element_of_column(column)] &=
        ~(1ULL << bit_of_column(column));
}

bool dynamic_bitset_matrix_check(DynamicBitsetMatrix *restrict matrix,
                                 u64 row,
                                 u64 column) {
    exp_assert(matrix != NULL);
    return (matrix->buffer[row_of_element(row) + element_of_column(column)] >>
            bit_of_column(column)) &
           1ULL;
}
