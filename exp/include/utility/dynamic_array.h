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
 * @file utility/dynamic_array.h
 */

#ifndef EXP_UTILITY_DYNAMIC_ARRAY_H
#define EXP_UTILITY_DYNAMIC_ARRAY_H
#include <assert.h>
#include <stdbool.h>

#include "utility/alloc.h"
#include "utility/array_growth.h"
#include "utility/int_types.h"

#define EXP_DYNAMIC_ARRAY_DECLARATION(DynamicArrayName, DynamicArrayType,      \
                                      DynamicArrayElementType)                 \
    typedef struct {                                                           \
        u64 count;                                                             \
        u64 capacity;                                                          \
        DynamicArrayElementType *array;                                        \
    } type_DynamicArrayName;

#define EXP_DYNAMIC_ARRAY_FUNCTION_DECLARATIONS(                               \
    DynamicArrayName, DynamicArrayType, DynamicArrayElementType)               \
    void DynamicArrayName##_initialize(DynamicArrayType *DynamicArrayName);    \
    void DynamicArrayName##_terminate(DynamicArrayType *DynamicArrayName);     \
    void DynamicArrayName##_append(DynamicArrayType *DynamicArrayName,         \
                                   DynamicArrayElementType element);           \
    DynamicArrayElementType *DynamicArrayName##_at(                            \
        DynamicArrayType *DynamicArrayName, u64 index);

#define EXP_DYNAMIC_ARRAY_FUNCTION_DEFINITIONS(                                      \
    DynamicArrayName, DynamicArrayType, DynamicArrayElementType)                     \
    void DynamicArrayName##_initialize(DynamicArrayType *DynamicArrayName) {         \
        assert(DynamicArrayName != nullptr);                                         \
        DynamicArrayName->count    = 0;                                              \
        DynamicArrayName->capacity = 0;                                              \
        DynamicArrayName->array    = nullptr;                                        \
    }                                                                                \
                                                                                     \
    void DynamicArrayName##_terminate(DynamicArrayType *DynamicArrayName) {          \
        assert(DynamicArrayName != nullptr);                                         \
        DynamicArrayName->count    = 0;                                              \
        DynamicArrayName->capacity = 0;                                              \
        deallocate(DynamicArrayName->array);                                         \
    }                                                                                \
                                                                                     \
    bool DynamicArrayName##_full(DynamicArrayType *DynamicArrayName) {               \
        assert(DynamicArrayName != nullptr);                                         \
        return (DynamicArrayName->count + 1) >= name->capacity;                      \
    }                                                                                \
                                                                                     \
    bool DynamicArrayName##_index_in_bounds(                                         \
        DynamicArrayType *DynamicArrayName, u64 index) {                             \
        assert(DynamicArrayName != nullptr);                                         \
        return index < DynamicArrayName->count;                                      \
    }                                                                                \
                                                                                     \
    void DynamicArrayName##_grow(DynamicArrayType *DynamicArrayName) {               \
        assert(DynamicArrayName != nullptr);                                         \
        Growth g                = array_growth_u64(DynamicArrayName->capacity,       \
                                                   sizeof(DynamicArrayElementType)); \
        DynamicArrayName->array = reallocate(name->array, g.alloc_size);             \
        DynamicArrayName->capacity = g.new_capacity;                                 \
    }                                                                                \
                                                                                     \
    void DynamicArrayName##_append(DynamicArrayType *DynamicArrayName,               \
                                   DynamicArrayElementType element) {                \
        assert(DynamicArrayName != nullptr);                                         \
        if (DynamicArrayName##_full(name)) { DynamicArrayName##_grow(name); }        \
        DynamicArrayName->array[name->count++] = element;                            \
    }

#endif // EXP_UTILITY_DYNAMIC_ARRAY_H
