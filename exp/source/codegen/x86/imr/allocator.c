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
#include <assert.h>

#include "codegen/x86/imr/allocator.h"
#include "codegen/x86/imr/registers.h"
#include "intrinsics/size_of.h"
#include "support/allocation.h"
#include "support/array_growth.h"
#include "support/assert.h"
#include "support/panic.h"
#include "support/unreachable.h"

static x64_GPRP x64_gprp_create() {
    x64_GPRP gprp = {.bitset = 0,
                     .buffer = callocate(16, sizeof(x64_Allocation *))};
    return gprp;
}

static void x64_gprp_destroy(x64_GPRP *restrict gprp) {
    gprp->bitset = 0;
    deallocate(gprp->buffer);
}

#define SET_BIT(B, r) ((B) |= (u16)(1 << r))
#define CLR_BIT(B, r) ((B) &= (u16)(~(1 << r)))
#define CHK_BIT(B, r) (((B) >> r) & 1)

static void x64_gprp_aquire(x64_GPRP *restrict gprp, x86_64_GPR gpr) {
    SET_BIT(gprp->bitset, x86_64_gpr_index(gpr));
}

static void x64_gprp_release(x64_GPRP *restrict gprp, x86_64_GPR gpr) {
    CLR_BIT(gprp->bitset, x86_64_gpr_index(gpr));
    gprp->buffer[x86_64_gpr_index(gpr)] = NULL;
}

static void x64_gprp_release_index(x64_GPRP *restrict gprp, u8 gpr_index) {
    CLR_BIT(gprp->bitset, gpr_index);
    gprp->buffer[gpr_index] = NULL;
}

static bool x64_gprp_any_available(x64_GPRP *restrict gprp,
                                   u8 *restrict gpr_index) {
    for (u8 i = 0; i < 16; ++i) {
        if (!CHK_BIT(gprp->bitset, i)) {
            *gpr_index = i;
            return true;
        }
    }
    return false;
}

static void
x64_gprp_allocate_to_gpr_index(x64_GPRP *restrict gprp,
                               u8 gpr_index,
                               x64_Allocation *restrict allocation) {
    u64 size = size_of(allocation->type);
    exp_assert_debug(x86_64_gpr_valid_size(size));
    x86_64_GPR gpr = x86_64_gpr_with_size(gpr_index, size);
    SET_BIT(gprp->bitset, x86_64_gpr_index(gpr));
    gprp->buffer[x86_64_gpr_index(gpr)] = allocation;
    allocation->location                = x64_location_gpr(gpr);
}

static void x64_gprp_allocate_to_gpr(x64_GPRP *restrict gprp,
                                     x86_64_GPR gpr,
                                     x64_Allocation *restrict allocation) {
    u64 size = size_of(allocation->type);
    exp_assert_debug(x86_64_gpr_valid_size(size));
    if (!x86_64_gpr_is_sized(gpr)) { gpr = x86_64_gpr_resize(gpr, size); }
    exp_assert_debug(size <= x86_64_gpr_size(gpr));

    SET_BIT(gprp->bitset, x86_64_gpr_index(gpr));
    gprp->buffer[x86_64_gpr_index(gpr)] = allocation;
    allocation->location                = x64_location_gpr(gpr);
}

/**
 * @brief allocate the given allocation to the next available GPR
 *
 * @pre the allocation must be a valid size for a GPR.
 *
 * @return true if able to allocate
 * @return false otherwise
 */
static bool x64_gprp_allocate(x64_GPRP *restrict gprp,
                              x64_Allocation *restrict allocation) {
    u8 gpr_index;
    if (x64_gprp_any_available(gprp, &gpr_index)) {
        x64_gprp_allocate_to_gpr_index(gprp, gpr_index, allocation);
        return true;
    }

    return false;
}

/**
 * @brief moves the allocation from it's current GPR to the next available
 * (different) GPR
 *
 * @pre the allocation must be allocated to a GPR managed by this GPRP
 * @pre the allocation must be a valid size for a GPR
 *
 * @return true if able to allocate
 * @return false otherwise
 */
static bool x64_gprp_reallocate(x64_GPRP *restrict gprp,
                                x64_Allocation *restrict allocation) {
    assert(allocation->location.kind == LOCATION_GPR);
    u8 gpr_index;
    if (x64_gprp_any_available(gprp, &gpr_index)) {
        x64_gprp_release(gprp, allocation->location.gpr);
        x64_gprp_allocate_to_gpr_index(gprp, gpr_index, allocation);
        return true;
    }
    return false;
}

static x64_Allocation *x64_gprp_allocation_at(x64_GPRP *restrict gprp,
                                              x86_64_GPR gpr) {
    return gprp->buffer[x86_64_gpr_index(gpr)];
}

static x64_Allocation *x64_gprp_allocation_of(x64_GPRP *restrict gprp,
                                              u64 ssa) {
    for (u8 i = 0; i < 16; ++i) {
        x64_Allocation *cursor = gprp->buffer[i];
        if (cursor == NULL) { continue; }
        if (cursor->ssa == ssa) { return cursor; }
    }

    return NULL;
}

static x64_Allocation *x64_gprp_oldest_allocation(x64_GPRP *restrict gprp) {
    x64_Allocation *oldest = NULL;

    for (u8 i = 0; i < 16; ++i) {
        x64_Allocation *cursor = gprp->buffer[i];
        if (cursor == NULL) { continue; }

        if ((oldest == NULL) ||
            (oldest->lifetime.last_use < cursor->lifetime.last_use)) {
            oldest = cursor;
        }
    }
    return oldest;
}

static void x64_gprp_release_expired_allocations(x64_GPRP *restrict gprp,
                                                 u64 Idx) {
    for (u8 i = 0; i < 16; ++i) {
        x64_Allocation *cursor = gprp->buffer[i];
        if (cursor == NULL) { continue; }

        if (cursor->lifetime.last_use < Idx) {
            x64_gprp_release_index(gprp, i);
        }
    }
}

#undef SET_BIT
#undef CLR_BIT
#undef CHK_BIT

static x64_StackAllocations x64_stack_allocations_create() {
    x64_StackAllocations stack_allocations = {
        .active_stack_size = 0,
        .total_stack_size  = 0,
        .count             = 0,
        .capacity          = 0,
        .buffer            = NULL,
    };
    return stack_allocations;
}

static void x64_stack_allocations_destroy(
    x64_StackAllocations *restrict stack_allocations) {
    if (stack_allocations->buffer != NULL) {
        deallocate(stack_allocations->buffer);
    }

    stack_allocations->count             = 0;
    stack_allocations->capacity          = 0;
    stack_allocations->active_stack_size = 0;
    stack_allocations->total_stack_size  = 0;
}

static bool
stack_allocations_full(x64_StackAllocations *restrict stack_allocations) {
    return (stack_allocations->count + 1) >= stack_allocations->capacity;
}

static void
stack_allocations_grow(x64_StackAllocations *restrict stack_allocations) {
    Growth_u64 g =
        array_growth_u64(stack_allocations->capacity, sizeof(x64_Allocation *));
    stack_allocations->buffer =
        reallocate(stack_allocations->buffer, g.alloc_size);
    stack_allocations->capacity = g.new_capacity;
}

static void
x64_stack_allocations_append(x64_StackAllocations *restrict stack_allocations,
                             x64_Allocation *restrict allocation) {
    if (stack_allocations_full(stack_allocations)) {
        stack_allocations_grow(stack_allocations);
    }

    stack_allocations->buffer[stack_allocations->count] = allocation;
    stack_allocations->count += 1;
}

static void
x64_stack_allocations_allocate(x64_StackAllocations *restrict stack_allocations,
                               x64_Allocation *restrict allocation) {
    if (__builtin_add_overflow(stack_allocations->total_stack_size,
                               size_of(allocation->type),
                               &stack_allocations->total_stack_size)) {
        PANIC("computed stack size overflow");
    }

    i64 offset = stack_allocations->total_stack_size;

    allocation->location = x64_location_address(X86_64_GPR_RBP, -offset);

    x64_stack_allocations_append(stack_allocations, allocation);
}

static void
x64_stack_allocations_erase(x64_StackAllocations *restrict stack_allocations,
                            x64_Allocation *restrict allocation) {
    u64 i = 0;
    for (; i < stack_allocations->count; ++i) {
        x64_Allocation *cursor = stack_allocations->buffer[i];
        if (cursor == allocation) { break; }
    }

    if (i == stack_allocations->count) { return; }

    for (; i < stack_allocations->count; ++i) {
        stack_allocations->buffer[i] = stack_allocations->buffer[i + 1];
    }

    stack_allocations->count -= 1;
}

static void x64_stack_allocations_release_expired_allocations(
    x64_StackAllocations *restrict stack_allocations, u64 Idx) {
    for (u64 i = 0; i < stack_allocations->count; ++i) {
        x64_Allocation *cursor = stack_allocations->buffer[i];
        if (cursor->lifetime.last_use < Idx) {
            x64_stack_allocations_erase(stack_allocations, cursor);
            --i;
        }
    }
}

static x64_Allocation *
x64_stack_allocations_of(x64_StackAllocations *restrict stack_allocations,
                         u64 ssa) {
    for (u64 i = 0; i < stack_allocations->count; ++i) {
        x64_Allocation *cursor = stack_allocations->buffer[i];
        if (cursor->ssa == ssa) { return cursor; }
    }
    EXP_UNREACHABLE();
}

static x64_AllocationBuffer x64_allocation_buffer_create() {
    x64_AllocationBuffer allocation_buffer = {
        .count = 0, .capacity = 0, .buffer = NULL};
    return allocation_buffer;
}

static void x64_allocation_buffer_destroy(
    x64_AllocationBuffer *restrict allocation_buffer) {
    assert(allocation_buffer != NULL);
    for (u64 i = 0; i < allocation_buffer->count; ++i) {
        x64_allocation_deallocate(allocation_buffer->buffer[i]);
    }

    deallocate(allocation_buffer->buffer);
    allocation_buffer->buffer   = NULL;
    allocation_buffer->count    = 0;
    allocation_buffer->capacity = 0;
}

static bool
x64_allocation_buffer_full(x64_AllocationBuffer *restrict allocation_buffer) {
    return (allocation_buffer->count + 1) >= allocation_buffer->capacity;
}

static void
x64_allocation_buffer_grow(x64_AllocationBuffer *restrict allocation_buffer) {
    Growth_u64 g =
        array_growth_u64(allocation_buffer->capacity, sizeof(x64_Allocation *));
    allocation_buffer->buffer =
        reallocate(allocation_buffer->buffer, g.alloc_size);
    allocation_buffer->capacity = g.new_capacity;
}

static x64_Allocation *
x64_allocation_buffer_append(x64_AllocationBuffer *restrict allocation_buffer,
                             u64         ssa,
                             Lifetime   *lifetime,
                             Type const *type) {
    assert(allocation_buffer != NULL);
    assert(type != NULL);

    if (x64_allocation_buffer_full(allocation_buffer)) {
        x64_allocation_buffer_grow(allocation_buffer);
    }

    x64_Allocation **allocation =
        allocation_buffer->buffer + allocation_buffer->count;
    allocation_buffer->count += 1;
    *allocation        = x64_allocation_allocate();
    (*allocation)->ssa = ssa;
    if (lifetime != NULL) {
        (*allocation)->lifetime = *lifetime;
    } else {
        (*allocation)->lifetime = lifetime_immortal();
    }
    (*allocation)->type = type;
    return *allocation;
}

x64_Allocator x64_allocator_create(Function *restrict body,
                                   Context *restrict context) {
    x64_Allocator allocator = {
        .gprp              = x64_gprp_create(),
        .stack_allocations = x64_stack_allocations_create(),
        .allocations       = x64_allocation_buffer_create(),
        .lifetimes         = lifetimes_compute(body, context),
    };
    x64_gprp_aquire(&allocator.gprp, X86_64_GPR_RSP);
    x64_gprp_aquire(&allocator.gprp, X86_64_GPR_RBP);
    return allocator;
}

void x64_allocator_destroy(x64_Allocator *restrict allocator) {
    x64_gprp_destroy(&allocator->gprp);
    x64_stack_allocations_destroy(&allocator->stack_allocations);
    x64_allocation_buffer_destroy(&allocator->allocations);
    lifetimes_destroy(&allocator->lifetimes);
}

bool x64_allocator_uses_stack(x64_Allocator *restrict allocator) {
    return allocator->stack_allocations.total_stack_size > 0;
}

i64 x64_allocator_total_stack_size(x64_Allocator *restrict allocator) {
    return allocator->stack_allocations.total_stack_size;
}

static void
x64_allocator_release_expired_lifetimes(x64_Allocator *restrict allocator,
                                        u64 Idx) {
    x64_stack_allocations_release_expired_allocations(
        &allocator->stack_allocations, Idx);
    x64_gprp_release_expired_allocations(&allocator->gprp, Idx);
}

static void x64_allocator_spill_allocation(x64_Allocator *restrict allocator,
                                           x64_Allocation *restrict allocation,
                                           x64_Bytecode *restrict x64bc) {
    assert(allocation->location.kind == LOCATION_GPR);
    x86_64_GPR gpr = allocation->location.gpr;
    x64_gprp_release(&allocator->gprp, gpr);
    x64_stack_allocations_allocate(&allocator->stack_allocations, allocation);

    x64_bytecode_append(
        x64bc, x64_mov(x64_operand_alloc(allocation), x64_operand_gpr(gpr)));
}

x64_Allocation *x64_allocator_allocation_of(x64_Allocator *restrict allocator,
                                            u64 ssa) {
    x64_Allocation *allocation = x64_gprp_allocation_of(&allocator->gprp, ssa);
    if (allocation != NULL) { return allocation; }

    return x64_stack_allocations_of(&allocator->stack_allocations, ssa);
}

void x64_allocator_release_gpr(x64_Allocator *restrict allocator,
                               x86_64_GPR gpr,
                               u64        Idx,
                               x64_Bytecode *restrict x64bc) {
    x64_Allocation *active = x64_gprp_allocation_at(&allocator->gprp, gpr);
    if ((active == NULL) || active->lifetime.last_use < Idx) {
        x64_gprp_release(&allocator->gprp, gpr);
        return;
    }

    x64_allocator_reallocate_active(allocator, active, x64bc);
}

void x64_allocator_aquire_gpr(x64_Allocator *restrict allocator,
                              x86_64_GPR gpr,
                              u64        Idx,
                              x64_Bytecode *restrict x64bc) {
    x64_Allocation *active = x64_gprp_allocation_at(&allocator->gprp, gpr);
    if (active == NULL) {
        x64_gprp_aquire(&allocator->gprp, gpr);
        return;
    }

    if (active->lifetime.last_use <= Idx) {
        x64_gprp_release(&allocator->gprp, gpr);
        x64_gprp_aquire(&allocator->gprp, gpr);
        return;
    }

    x64_allocator_reallocate_active(allocator, active, x64bc);
}

static void x64_allocator_stack_allocate(x64_Allocator *restrict allocator,
                                         x64_Allocation *restrict allocation) {
    x64_stack_allocations_allocate(&allocator->stack_allocations, allocation);
}

static void x64_allocator_register_allocate(x64_Allocator *restrict allocator,
                                            u64 Idx,
                                            x64_Allocation *restrict allocation,
                                            x64_Bytecode *restrict x64bc) {
    x64_allocator_release_expired_lifetimes(allocator, Idx);

    if (x64_gprp_allocate(&allocator->gprp, allocation)) { return; }

    // otherwise spill the oldest active allocation to the stack.
    x64_Allocation *oldest_active =
        x64_gprp_oldest_allocation(&allocator->gprp);

    if (oldest_active->lifetime.last_use > allocation->lifetime.last_use) {
        x64_allocator_spill_allocation(allocator, oldest_active, x64bc);
        x64_gprp_allocate(&allocator->gprp, allocation);
    } else {
        x64_allocator_stack_allocate(allocator, allocation);
    }
}

x64_Allocation *x64_allocator_allocate(x64_Allocator *restrict allocator,
                                       u64            Idx,
                                       LocalVariable *local,
                                       x64_Bytecode *restrict x64bc) {
    Lifetime       *lifetime = lifetimes_at(&allocator->lifetimes, local->ssa);
    x64_Allocation *allocation = x64_allocation_buffer_append(
        &allocator->allocations, local->ssa, lifetime, local->type);

    if (string_view_empty(local->name) && type_is_scalar(local->type)) {
        x64_allocator_register_allocate(allocator, Idx, allocation, x64bc);
    } else {
        x64_allocator_stack_allocate(allocator, allocation);
    }

    return allocation;
}

/**
 * @brief allocate <ssa> in the same location as <active>
 * reallocating <active> if <active> expires after <Idx>
 *
 */
x64_Allocation *
x64_allocator_allocate_from_active(x64_Allocator *restrict allocator,
                                   u64             Idx,
                                   LocalVariable  *local,
                                   x64_Allocation *active,
                                   x64_Bytecode *restrict x64bc) {
    Lifetime *lifetime = lifetimes_at(&allocator->lifetimes, local->ssa);

    if (active->lifetime.last_use <= Idx) {
        // we can reuse the existing allocation treating
        // it as the new ssa local allocation.
        active->ssa      = local->ssa;
        active->lifetime = *lifetime;
        return active;
    }

    // we have to keep the existing allocation around
    x64_Allocation *new = x64_allocator_allocate(allocator, Idx, local, x64bc);

    // initialize the new allocation
    if ((active->location.kind == LOCATION_ADDRESS) &&
        (new->location.kind == LOCATION_ADDRESS)) {
        u64 size = size_of(new->type);
        exp_assert(x86_64_gpr_valid_size(size));
        x86_64_GPR gpr =
            x64_allocator_aquire_any_gpr(allocator, size, Idx, x64bc);
        x64_bytecode_append(
            x64bc, x64_mov(x64_operand_gpr(gpr), x64_operand_alloc(active)));
        x64_bytecode_append(
            x64bc, x64_mov(x64_operand_alloc(new), x64_operand_gpr(gpr)));
    } else {
        x64_bytecode_append(
            x64bc, x64_mov(x64_operand_alloc(new), x64_operand_alloc(active)));
    }

    return new;
}

x64_Allocation *
x64_allocator_allocate_to_any_gpr(x64_Allocator *restrict allocator,
                                  LocalVariable *local,
                                  x64_Bytecode *restrict x64bc) {
    Lifetime       *lifetime = lifetimes_at(&allocator->lifetimes, local->ssa);
    x64_Allocation *allocation = x64_allocation_buffer_append(
        &allocator->allocations, local->ssa, lifetime, local->type);

    if (x64_gprp_allocate(&allocator->gprp, allocation)) { return allocation; }

    u8 gpr_index = x64_allocator_spill_oldest_active(allocator, x64bc);
    x64_gprp_allocate_to_gpr_index(&allocator->gprp, gpr_index, allocation);
    return allocation;
}

x64_Allocation *x64_allocator_allocate_to_gpr(x64_Allocator *restrict allocator,
                                              LocalVariable *local,
                                              x86_64_GPR     gpr,
                                              u64            Idx,
                                              x64_Bytecode *restrict x64bc) {
    Lifetime       *lifetime = lifetimes_at(&allocator->lifetimes, local->ssa);
    x64_Allocation *allocation = x64_allocation_buffer_append(
        &allocator->allocations, local->ssa, lifetime, local->type);

    x64_allocator_release_gpr(allocator, gpr, Idx, x64bc);
    x64_gprp_allocate_to_gpr(&allocator->gprp, gpr, allocation);
    return allocation;
}

x64_Allocation *x64_allocator_allocate_to_stack(
    x64_Allocator *restrict allocator, i64 offset, LocalVariable *local) {
    Lifetime       *lifetime = lifetimes_at(&allocator->lifetimes, local->ssa);
    x64_Allocation *allocation = x64_allocation_buffer_append(
        &allocator->allocations, local->ssa, lifetime, local->type);

    allocation->location = x64_location_address(X86_64_GPR_RBP, offset);

    x64_stack_allocations_append(&allocator->stack_allocations, allocation);
    return allocation;
}

x64_Allocation *x64_allocator_allocate_result(x64_Allocator *restrict allocator,
                                              x64_Location location,
                                              Type const  *type) {
    x64_Allocation *allocation = x64_allocation_buffer_append(
        &allocator->allocations, u64_MAX, nullptr, type);

    // The gpr allocator of this function does not actually want the
    // location marked as used, so that instructions within the function
    // can use the return GPR as a temporary, potentially allowing the
    // return value to be computed in the same GPR as the return value.
    // This is a bit of a hack, but it works for now.
    // The stack_allocator of this function does not handle allocating the
    // result. the caller allocates space for the result.
    allocation->location = location;

    return allocation;
}

void x64_allocator_reallocate_active(x64_Allocator *restrict allocator,
                                     x64_Allocation *restrict active,
                                     x64_Bytecode *restrict x64bc) {
    if (active->location.kind == LOCATION_ADDRESS) { return; }

    x86_64_GPR prev_gpr = active->location.gpr;
    if (x64_gprp_reallocate(&allocator->gprp, active)) {
        x64_bytecode_append(x64bc,
                            x64_mov(x64_operand_gpr(active->location.gpr),
                                    x64_operand_gpr(prev_gpr)));
    } else {
        x64_allocator_spill_allocation(allocator, active, x64bc);
    }
}

u8 x64_allocator_spill_oldest_active(x64_Allocator *restrict allocator,
                                     x64_Bytecode *restrict x64bc) {
    x64_Allocation *oldest = x64_gprp_oldest_allocation(&allocator->gprp);
    if (oldest == NULL) {
        u8 gpr_index;
        if (x64_gprp_any_available(&allocator->gprp, &gpr_index)) {
            return gpr_index;
        }

        EXP_UNREACHABLE();
    }
    x86_64_GPR gpr = oldest->location.gpr;
    x64_allocator_spill_allocation(allocator, oldest, x64bc);
    return x86_64_gpr_index(gpr);
}

x86_64_GPR x64_allocator_aquire_any_gpr(x64_Allocator *restrict allocator,
                                        u64 size,
                                        u64 Idx,
                                        x64_Bytecode *restrict x64bc) {
    exp_assert(x86_64_gpr_valid_size(size));
    x64_allocator_release_expired_lifetimes(allocator, Idx);

    u8 gpr = 0;
    if (x64_gprp_any_available(&allocator->gprp, &gpr)) {
        return x86_64_gpr_with_size(gpr, size);
    }

    gpr = x64_allocator_spill_oldest_active(allocator, x64bc);
    return x86_64_gpr_with_size(gpr, size);
}
