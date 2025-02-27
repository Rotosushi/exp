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

#include "targets/x86_64/imr/register_allocator.h"
#include "env/context.h"
#include "imr/function.h"
#include "utility/alloc.h"
#include "utility/array_growth.h"
#include "utility/assert.h"
#include "utility/config.h"

static void
aquire_general_purpose_register(x86_64_RegisterAllocator *register_allocator,
                                u8 index) {
    EXP_ASSERT(register_allocator != nullptr);
    bitset_set_bit(&register_allocator->active_general_purpose_registers,
                   index);
}

static void
release_general_purpose_register(x86_64_RegisterAllocator *register_allocator,
                                 u8 index) {
    EXP_ASSERT(register_allocator != nullptr);
    bitset_clear_bit(&register_allocator->active_general_purpose_registers,
                     index);
}

static bool next_available_general_purpose_register(
    x86_64_RegisterAllocator *register_allocator, u8 *index) {
    EXP_ASSERT(register_allocator != nullptr);
    EXP_ASSERT(index != nullptr);
    Bitset *set = &register_allocator->active_general_purpose_registers;
#if defined(EXP_HOST_COMPILER_GCC) || defined(EXP_HOST_COMPILER_CLANG)
    u32 bits = ~set->bits;
    if (bits == 0) { return false; }
    i32 next = __builtin_ctz(bits);
    EXP_ASSERT(next < bitset_length());
    *index = (u8)next;
    return true;
#else
    for (u8 next = 0; next < bitset_length(); ++next) {
        if (bitset_check_bit(set, next)) { continue; }
        *index = next;
        return true;
    }
    return false;
#endif
}

static void x86_64_allocations_initialize(x86_64_Allocations *allocations) {
    EXP_ASSERT(allocations != nullptr);
    allocations->length   = 0;
    allocations->capacity = 0;
    allocations->buffer   = nullptr;
}

static void x86_64_allocations_terminate(x86_64_Allocations *allocations) {
    EXP_ASSERT(allocations != nullptr);
    deallocate(allocations->buffer);
    x86_64_allocations_initialize(allocations);
}

static bool x86_64_allocations_full(x86_64_Allocations const *allocations) {
    EXP_ASSERT(allocations != nullptr);
    return (allocations->length + 1) >= allocations->capacity;
}

static void x86_64_allocations_grow(x86_64_Allocations *allocations) {
    EXP_ASSERT(allocations != nullptr);
    Growth32 g =
        array_growth_u32(allocations->capacity, sizeof(x86_64_Allocation));
    allocations->buffer   = reallocate(allocations->buffer, g.alloc_size);
    allocations->capacity = g.new_capacity;
}

static u32 x86_64_allocations_append(x86_64_Allocations *allocations,
                                     Local *local) {
    EXP_ASSERT(allocations != nullptr);
    EXP_ASSERT(local != nullptr);
    if (x86_64_allocations_full(allocations)) {
        x86_64_allocations_grow(allocations);
    }

    u32 index                     = allocations->length++;
    x86_64_Allocation *allocation = allocations->buffer + index;
    allocation->local             = *local;
    return index;
}

static x86_64_Allocation *x86_64_allocations_at(x86_64_Allocations *allocations,
                                                u32 index) {
    EXP_ASSERT(allocations != nullptr);
    EXP_ASSERT(index < allocations->length);
    return allocations->buffer + index;
}

void x86_64_register_allocator_initialize(
    x86_64_RegisterAllocator *register_allocator,
    Function *function,
    Context *context) {
    EXP_ASSERT(register_allocator != nullptr);
    register_allocator->active_general_purpose_registers = bitset_create();
    register_allocator->total_stack_size                 = 0;
    register_allocator->active_stack_size                = 0;
    x86_64_allocations_initialize(&register_allocator->allocations);
}

void x86_64_register_allocator_terminate(
    x86_64_RegisterAllocator *register_allocator) {}

/**
 * @brief allocates space in the current function's frame for
 * the given local
 */
x86_64_Allocation *
x86_64_register_allocator_allocate(x86_64_RegisterAllocator *register_allocator,
                                   Local *local,
                                   u32 block_index) {}

void x86_64_register_allocator_reallocate_active(
    x86_64_RegisterAllocator *register_allocator,
    x86_64_Allocation *active,
    u32 block_index) {}
/**
 * @brief handles the case where we want to allocate space for
 *  a given local (ssa or label) using the contents of an existing
 *  allocation of a given local.
 */
x86_64_Allocation *x86_64_regsiter_allocator_allocate_from_active(
    x86_64_RegisterAllocator *register_allocator,
    Local *local,
    x86_64_Allocation *active,
    u32 block_index) {}

/**
 * @brief handles the case where we need to allocate a given local to
 *  a specific GPR, such as the result allocation of a division or modulus
 *  operation.
 */
x86_64_Allocation *x86_64_register_allocator_allocate_to_gpr(
    x86_64_RegisterAllocator *register_allocator,
    Local *local,
    x86_64_GPR gpr,
    u32 block_index) {}

/**
 * @brief handles allocating outgoing arguments to the current stack frame
 *  until a better solution is found.
 *
 * @warning doesn't do any validation of the given address.
 */
x86_64_Allocation *x86_64_register_allocator_allocate_to_address(
    x86_64_RegisterAllocator *register_allocator,
    Local *local,
    x86_64_Address offset) {}

/**
 * @brief creates the allocation for the result of a function.
 * this is used from the callee's perspective, just denoting
 * where the result is to be placed.
 *
 * @note the call instruction allocates space for the
 * result of a function from the caller's perspective.
 */
x86_64_Allocation *
x86_64_allocator_allocate_result(x86_64_RegisterAllocator *register_allocator,
                                 x86_64_Location location,
                                 Type const *type) {}
