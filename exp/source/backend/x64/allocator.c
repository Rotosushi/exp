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

#include "backend/x64/allocator.h"
#include "backend/x64/context.h"
#include "intrinsics/size_of.h"
#include "utility/alloc.h"
#include "utility/array_growth.h"
#include "utility/panic.h"
#include "utility/unreachable.h"

static void x64_gprp_create(x64_GPRP *gprp) {
    assert(gprp != nullptr);
    gprp->bitset = bitset_create();
    gprp->buffer = callocate(16, sizeof(x64_Allocation *));
}

static void x64_gprp_destroy(x64_GPRP *gprp) {
    assert(gprp != nullptr);
    gprp->bitset = bitset_create();
    deallocate(gprp->buffer);
}

static void x64_gprp_aquire(x64_GPRP *gprp, x64_GPR gpr) {
    assert(gprp != nullptr);
    assert(gpr != X64_GPR_NONE);
    bitset_set_bit(&gprp->bitset, gpr);
}

static void x64_gprp_release(x64_GPRP *gprp, x64_GPR gpr) {
    assert(gprp != nullptr);
    assert(gpr != X64_GPR_NONE);
    bitset_clear_bit(&gprp->bitset, gpr);
    gprp->buffer[gpr] = nullptr;
}

static bool x64_gprp_any_available(x64_GPRP *gprp, x64_GPR *gpr) {
    assert(gprp != nullptr);
    assert(gpr != nullptr);
    for (u8 i = 0; i < 16; ++i) {
        if (!bitset_check_bit(&gprp->bitset, i)) {
            *gpr = (x64_GPR)i;
            return true;
        }
    }
    return false;
}

static void x64_gprp_allocate_to_gpr(x64_GPRP *gprp,
                                     x64_GPR gpr,
                                     x64_Allocation *allocation) {
    assert(gprp != nullptr);
    assert(gpr != X64_GPR_NONE);
    assert(allocation != nullptr);
    bitset_set_bit(&gprp->bitset, gpr);
    gprp->buffer[gpr]    = allocation;
    allocation->location = x64_location_gpr(gpr);
}

static bool x64_gprp_allocate(x64_GPRP *gprp, x64_Allocation *allocation) {
    assert(gprp != nullptr);
    assert(allocation != nullptr);
    x64_GPR gpr;
    if (x64_gprp_any_available(gprp, &gpr)) {
        x64_gprp_allocate_to_gpr(gprp, gpr, allocation);
        return true;
    }

    return false;
}

static bool x64_gprp_reallocate(x64_GPRP *gprp, x64_Allocation *allocation) {
    assert(gprp != nullptr);
    assert(allocation != nullptr);
    assert(allocation->location.kind == LOCATION_GPR);
    x64_GPR gpr;
    if (x64_gprp_any_available(gprp, &gpr)) {
        x64_gprp_release(gprp, allocation->location.gpr);
        x64_gprp_allocate_to_gpr(gprp, gpr, allocation);
        return true;
    }
    return false;
}

static x64_Allocation *x64_gprp_allocation_at(x64_GPRP *gprp, x64_GPR gpr) {
    assert(gprp != nullptr);
    assert(gpr != X64_GPR_NONE);
    return gprp->buffer[gpr];
}

static x64_Allocation *x64_gprp_allocation_of(x64_GPRP *gprp, u64 ssa) {
    assert(gprp != nullptr);
    for (u8 i = 0; i < 16; ++i) {
        x64_Allocation *cursor = gprp->buffer[i];
        if (cursor == nullptr) { continue; }
        if (cursor->ssa == ssa) { return cursor; }
    }

    return nullptr;
}

static x64_Allocation *x64_gprp_oldest_allocation(x64_GPRP *gprp) {
    assert(gprp != nullptr);
    x64_Allocation *oldest = nullptr;
    for (u8 i = 0; i < 16; ++i) {
        x64_Allocation *cursor = gprp->buffer[i];
        if (cursor == nullptr) { continue; }

        if ((oldest == nullptr) ||
            (oldest->lifetime.last_use < cursor->lifetime.last_use)) {
            oldest = cursor;
        }
    }
    return oldest;
}

static void x64_gprp_release_expired_allocations(x64_GPRP *gprp, u64 Idx) {
    for (u8 i = 0; i < 16; ++i) {
        x64_Allocation *cursor = gprp->buffer[i];
        if (cursor == nullptr) { continue; }

        if (cursor->lifetime.last_use <= Idx) { x64_gprp_release(gprp, i); }
    }
}

static void
x64_stack_allocations_initialize(x64_StackAllocations *stack_allocations) {
    assert(stack_allocations != nullptr);
    stack_allocations->active_stack_size = 0;
    stack_allocations->total_stack_size  = 0;
    stack_allocations->count             = 0;
    stack_allocations->capacity          = 0;
    stack_allocations->buffer            = nullptr;
}

static void
x64_stack_allocations_destroy(x64_StackAllocations *stack_allocations) {
    assert(stack_allocations != nullptr);
    if (stack_allocations->buffer != nullptr) {
        deallocate(stack_allocations->buffer);
    }

    stack_allocations->count             = 0;
    stack_allocations->capacity          = 0;
    stack_allocations->active_stack_size = 0;
    stack_allocations->total_stack_size  = 0;
}

static bool stack_allocations_full(x64_StackAllocations *stack_allocations) {
    assert(stack_allocations != nullptr);
    return (stack_allocations->count + 1) >= stack_allocations->capacity;
}

static void stack_allocations_grow(x64_StackAllocations *stack_allocations) {
    assert(stack_allocations != nullptr);
    Growth64 g =
        array_growth_u64(stack_allocations->capacity, sizeof(x64_Allocation *));
    stack_allocations->buffer =
        reallocate(stack_allocations->buffer, g.alloc_size);
    stack_allocations->capacity = g.new_capacity;
}

static void
x64_stack_allocations_append(x64_StackAllocations *stack_allocations,
                             x64_Allocation *allocation) {
    assert(stack_allocations != nullptr);
    assert(allocation != nullptr);
    if (stack_allocations_full(stack_allocations)) {
        stack_allocations_grow(stack_allocations);
    }

    stack_allocations->buffer[stack_allocations->count] = allocation;
    stack_allocations->count += 1;
}

static void
x64_stack_allocations_allocate(x64_StackAllocations *stack_allocations,
                               x64_Allocation *allocation,
                               x64_Context *x64_context) {
    assert(stack_allocations != nullptr);
    assert(allocation != nullptr);
    if (__builtin_add_overflow(stack_allocations->total_stack_size,
                               size_of(allocation->type),
                               &stack_allocations->total_stack_size)) {
        PANIC("computed stack size overflow");
    }

    i64 offset = stack_allocations->total_stack_size;

    u16 address = x64_context_addresses_insert(
        x64_context, x64_address_create(X64_GPR_RBP, X64_GPR_NONE, 1, -offset));

    allocation->location = x64_location_address(address);

    x64_stack_allocations_append(stack_allocations, allocation);
}

static void x64_stack_allocations_erase(x64_StackAllocations *stack_allocations,
                                        x64_Allocation *allocation) {
    assert(stack_allocations != nullptr);
    assert(allocation != nullptr);
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
    x64_StackAllocations *stack_allocations, u64 Idx) {
    assert(stack_allocations != nullptr);
    for (u64 i = 0; i < stack_allocations->count; ++i) {
        x64_Allocation *cursor = stack_allocations->buffer[i];
        if (cursor->lifetime.last_use < Idx) {
            x64_stack_allocations_erase(stack_allocations, cursor);
            --i;
        }
    }
}

static x64_Allocation *
x64_stack_allocations_of(x64_StackAllocations *stack_allocations, u64 ssa) {
    assert(stack_allocations != nullptr);
    for (u64 i = 0; i < stack_allocations->count; ++i) {
        x64_Allocation *cursor = stack_allocations->buffer[i];
        if (cursor->ssa == ssa) { return cursor; }
    }
    // #NOTE: if an allocation is not present in a GPR, then it must
    //  be present in the stack. otherwise we have created an ill
    //  formed program.
    EXP_UNREACHABLE();
}

static void
x64_allocation_buffer_initialize(x64_AllocationBuffer *allocation_buffer) {
    assert(allocation_buffer != nullptr);
    allocation_buffer->count    = 0;
    allocation_buffer->capacity = 0;
    allocation_buffer->buffer   = nullptr;
}

static void
x64_allocation_buffer_destroy(x64_AllocationBuffer *allocation_buffer) {
    assert(allocation_buffer != nullptr);
    for (u64 i = 0; i < allocation_buffer->count; ++i) {
        x64_allocation_deallocate(allocation_buffer->buffer[i]);
    }

    deallocate(allocation_buffer->buffer);
    allocation_buffer->buffer   = nullptr;
    allocation_buffer->count    = 0;
    allocation_buffer->capacity = 0;
}

static bool
x64_allocation_buffer_full(x64_AllocationBuffer *allocation_buffer) {
    assert(allocation_buffer != nullptr);
    return (allocation_buffer->count + 1) >= allocation_buffer->capacity;
}

static void
x64_allocation_buffer_grow(x64_AllocationBuffer *allocation_buffer) {
    assert(allocation_buffer != nullptr);
    Growth64 g =
        array_growth_u64(allocation_buffer->capacity, sizeof(x64_Allocation *));
    allocation_buffer->buffer =
        reallocate(allocation_buffer->buffer, g.alloc_size);
    allocation_buffer->capacity = g.new_capacity;
}

static x64_Allocation *
x64_allocation_buffer_append(x64_AllocationBuffer *allocation_buffer,
                             u64 ssa,
                             Lifetime lifetime,
                             Type const *type) {
    assert(allocation_buffer != nullptr);
    assert(type != nullptr);

    if (x64_allocation_buffer_full(allocation_buffer)) {
        x64_allocation_buffer_grow(allocation_buffer);
    }

    x64_Allocation **allocation =
        allocation_buffer->buffer + allocation_buffer->count;
    allocation_buffer->count += 1;
    *allocation             = x64_allocation_allocate();
    (*allocation)->ssa      = ssa;
    (*allocation)->lifetime = lifetime;
    (*allocation)->type     = type;
    return *allocation;
}

void x64_allocator_initialize(x64_Allocator *allocator,
                              FunctionBody *body,
                              x64_Context *x64_context) {
    assert(allocator != nullptr);
    assert(body != nullptr);
    assert(x64_context != nullptr);
    allocator->context = x64_context;
    x64_gprp_create(&allocator->gprp);
    x64_stack_allocations_initialize(&allocator->stack_allocations);
    x64_allocation_buffer_initialize(&allocator->allocations);
    lifetimes_initialize(&allocator->lifetimes, body, x64_context);

    x64_gprp_aquire(&allocator->gprp, X64_GPR_RSP);
    x64_gprp_aquire(&allocator->gprp, X64_GPR_RBP);
}

void x64_allocator_terminate(x64_Allocator *allocator) {
    x64_gprp_destroy(&allocator->gprp);
    x64_stack_allocations_destroy(&allocator->stack_allocations);
    x64_allocation_buffer_destroy(&allocator->allocations);
    lifetimes_destroy(&allocator->lifetimes);
}

bool x64_allocator_uses_stack(x64_Allocator *allocator) {
    assert(allocator != nullptr);
    return allocator->stack_allocations.total_stack_size > 0;
}

i64 x64_allocator_total_stack_size(x64_Allocator *allocator) {
    assert(allocator != nullptr);
    return allocator->stack_allocations.total_stack_size;
}

void x64_allocator_update_lifetime(x64_Allocator *allocator,
                                   u64 ssa,
                                   Lifetime lifetime) {
    assert(allocator != nullptr);
    lifetimes_update(&allocator->lifetimes, ssa, lifetime);
}

static void x64_allocator_release_expired_lifetimes(x64_Allocator *allocator,
                                                    u64 Idx) {
    x64_stack_allocations_release_expired_allocations(
        &allocator->stack_allocations, Idx);
    x64_gprp_release_expired_allocations(&allocator->gprp, Idx);
}

static void x64_allocator_spill_allocation(x64_Allocator *allocator,
                                           x64_Allocation *allocation) {
    assert(allocation->location.kind == LOCATION_GPR);
    x64_GPR gpr = allocation->location.gpr;
    x64_gprp_release(&allocator->gprp, gpr);
    x64_stack_allocations_allocate(
        &allocator->stack_allocations, allocation, allocator->context);
    assert(allocation->location.kind == LOCATION_ADDRESS);

    x64_bytecode_append(
        x64_context_current_x64_bc(allocator->context),
        x64_mov(x64_operand_address(allocation->location.address),
                x64_operand_gpr(gpr)));
}

x64_Allocation *x64_allocator_allocation_of(x64_Allocator *allocator, u64 ssa) {
    x64_Allocation *allocation = x64_gprp_allocation_of(&allocator->gprp, ssa);
    if (allocation != nullptr) { return allocation; }

    return x64_stack_allocations_of(&allocator->stack_allocations, ssa);
}

void x64_allocator_reallocate_active(x64_Allocator *allocator,
                                     x64_Allocation *active) {
    if (active->location.kind == LOCATION_ADDRESS) { return; }

    x64_GPR prev_gpr = active->location.gpr;
    if (x64_gprp_reallocate(&allocator->gprp, active)) {
        x64_bytecode_append(x64_context_current_x64_bc(allocator->context),
                            x64_mov(x64_operand_gpr(active->location.gpr),
                                    x64_operand_gpr(prev_gpr)));
    } else {
        x64_allocator_spill_allocation(allocator, active);
    }
}

void x64_allocator_release_gpr(x64_Allocator *allocator, x64_GPR gpr, u64 Idx) {
    x64_Allocation *active = x64_gprp_allocation_at(&allocator->gprp, gpr);
    if ((active == nullptr) || active->lifetime.last_use < Idx) {
        x64_gprp_release(&allocator->gprp, gpr);
        return;
    }

    x64_allocator_reallocate_active(allocator, active);
}

void x64_allocator_aquire_gpr(x64_Allocator *allocator, x64_GPR gpr, u64 Idx) {
    x64_Allocation *active = x64_gprp_allocation_at(&allocator->gprp, gpr);
    if (active == nullptr) {
        x64_gprp_aquire(&allocator->gprp, gpr);
        return;
    }

    if (active->lifetime.last_use <= Idx) {
        x64_gprp_release(&allocator->gprp, gpr);
        x64_gprp_aquire(&allocator->gprp, gpr);
        return;
    }

    x64_allocator_reallocate_active(allocator, active);
}

x64_Allocation *x64_allocator_allocate_to_gpr(x64_Allocator *allocator,
                                              x64_GPR gpr,
                                              u64 Idx,
                                              LocalVariable *local) {
    x64_allocator_release_gpr(allocator, gpr, Idx);

    Lifetime *lifetime = lifetimes_at(&allocator->lifetimes, local->ssa);
    x64_Allocation *allocation = x64_allocation_buffer_append(
        &allocator->allocations, local->ssa, *lifetime, local->type);

    x64_gprp_allocate_to_gpr(&allocator->gprp, gpr, allocation);
    return allocation;
}

x64_Allocation *x64_allocator_allocate_to_stack(x64_Allocator *allocator,
                                                i64 offset,
                                                LocalVariable *local) {
    Lifetime *lifetime = lifetimes_at(&allocator->lifetimes, local->ssa);
    x64_Allocation *allocation = x64_allocation_buffer_append(
        &allocator->allocations, local->ssa, *lifetime, local->type);

    u16 address = x64_context_addresses_insert(
        allocator->context,
        x64_address_create(X64_GPR_RBP, X64_GPR_NONE, 1, offset));
    allocation->location = x64_location_address(address);

    x64_stack_allocations_append(&allocator->stack_allocations, allocation);
    return allocation;
}

x64_Allocation *x64_allocator_allocate_result(x64_Allocator *allocator,
                                              x64_Location location,
                                              Type const *type) {
    x64_Allocation *allocation = x64_allocation_buffer_append(
        &allocator->allocations, u64_MAX, lifetime_immortal(), type);

    allocation->location = location;

    return allocation;
}

static x64_GPR x64_allocator_spill_oldest_active(x64_Allocator *allocator) {
    x64_Allocation *oldest = x64_gprp_oldest_allocation(&allocator->gprp);
    if (oldest != nullptr) {
        x64_GPR gpr = oldest->location.gpr;
        x64_allocator_spill_allocation(allocator, oldest);
        return gpr;
    }
    EXP_UNREACHABLE();
}

x64_GPR x64_allocator_aquire_any_gpr(x64_Allocator *allocator, u64 Idx) {
    x64_allocator_release_expired_lifetimes(allocator, Idx);

    x64_GPR gpr = 0;
    if (x64_gprp_any_available(&allocator->gprp, &gpr)) { return gpr; }

    gpr = x64_allocator_spill_oldest_active(allocator);
    return gpr;
}

static void x64_allocator_stack_allocate(x64_Allocator *allocator,
                                         x64_Allocation *allocation) {
    x64_stack_allocations_allocate(
        &allocator->stack_allocations, allocation, allocator->context);
}

static void x64_allocator_register_allocate(x64_Allocator *allocator,
                                            u64 Idx,
                                            x64_Allocation *allocation) {
    x64_allocator_release_expired_lifetimes(allocator, Idx);

    if (x64_gprp_allocate(&allocator->gprp, allocation)) { return; }

    // otherwise spill the oldest active allocation to the stack.
    x64_Allocation *oldest_active =
        x64_gprp_oldest_allocation(&allocator->gprp);

    if (oldest_active->lifetime.last_use > allocation->lifetime.last_use) {
        x64_allocator_spill_allocation(allocator, oldest_active);
        x64_gprp_allocate(&allocator->gprp, allocation);
    } else {
        x64_allocator_stack_allocate(allocator, allocation);
    }
}

x64_Allocation *x64_allocator_allocate(x64_Allocator *allocator,
                                       u64 Idx,
                                       LocalVariable *local) {
    Lifetime *lifetime = lifetimes_at(&allocator->lifetimes, local->ssa);
    x64_Allocation *allocation = x64_allocation_buffer_append(
        &allocator->allocations, local->ssa, *lifetime, local->type);

    if (string_view_empty(local->name) && type_is_scalar(local->type)) {
        x64_allocator_register_allocate(allocator, Idx, allocation);
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
x64_Allocation *x64_allocator_allocate_from_active(x64_Allocator *allocator,
                                                   u64 Idx,
                                                   LocalVariable *local,
                                                   x64_Allocation *active) {
    Lifetime *lifetime = lifetimes_at(&allocator->lifetimes, local->ssa);

    if (active->lifetime.last_use <= Idx) {
        // we can reuse the existing allocation treating
        // it as the new ssa local allocation.
        active->ssa      = local->ssa;
        active->lifetime = *lifetime;
        return active;
    }

    // we have to keep the existing allocation around
    x64_Allocation *new        = x64_allocator_allocate(allocator, Idx, local);
    x64_Bytecode *x64_bytecode = x64_context_current_x64_bc(allocator->context);

    // initialize the new allocation
    if ((active->location.kind == LOCATION_ADDRESS) &&
        (new->location.kind == LOCATION_ADDRESS)) {
        x64_GPR gpr = x64_allocator_aquire_any_gpr(allocator, Idx);
        x64_bytecode_append(
            x64_bytecode,
            x64_mov(x64_operand_gpr(gpr),
                    x64_operand_address(active->location.address)));
        x64_bytecode_append(x64_bytecode,
                            x64_mov(x64_operand_address(new->location.address),
                                    x64_operand_gpr(gpr)));
    } else {
        x64_bytecode_append(x64_bytecode,
                            x64_mov(x64_operand_location(new->location),
                                    x64_operand_location(active->location)));
    }

    return new;
}
