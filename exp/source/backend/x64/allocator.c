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
#include "utility/minmax.h"
#include "utility/panic.h"
#include "utility/unreachable.h"

static x64_GPRegisterPool x64_gprp_create() {
    x64_GPRegisterPool gprp = {.bitset = bitset_create()};
    return gprp;
}

static void x64_gprp_aquire(x64_GPRegisterPool *gprp, x64_GPR gpr) {
    assert(gprp != nullptr);
    bitset_set_bit(&gprp->bitset, x64_gpr_index(gpr));
}

static void x64_gprp_release(x64_GPRegisterPool *gprp, x64_GPR gpr) {
    assert(gprp != nullptr);
    bitset_clear_bit(&gprp->bitset, x64_gpr_index(gpr));
}

static bool
x64_gprp_first_available(x64_GPRegisterPool *gprp, x64_GPR *gpr, u64 size) {
    assert(gprp != nullptr);
    assert(gpr != nullptr);
    for (u8 i = 0; i < 16; ++i) {
        if (!bitset_check_bit(&gprp->bitset, i)) {
            *gpr = x64_gpr_with_size(i, size);
            return true;
        }
    }
    return false;
}

static void x64_gprp_allocate_to_gpr(x64_GPRegisterPool *gprp, x64_GPR gpr) {
    assert(gprp != nullptr);
    assert(!bitset_check_bit(&gprp->bitset, x64_gpr_index(gpr)));
    bitset_set_bit(&gprp->bitset, x64_gpr_index(gpr));
}

static bool x64_gprp_allocate_to_first_available(x64_GPRegisterPool *gprp,
                                                 x64_Allocation *allocation) {
    assert(gprp != nullptr);
    assert(allocation != nullptr);
    x64_GPR gpr;
    if (x64_gprp_first_available(gprp, &gpr, size_of(allocation->type))) {
        x64_gprp_allocate_to_gpr(gprp, gpr);
        allocation->location = x64_location_gpr(gpr);
        return true;
    }

    return false;
}

static bool
x64_gprp_move_allocation_to_first_available(x64_GPRegisterPool *gprp,
                                            x64_Allocation *allocation) {
    assert(gprp != nullptr);
    assert(allocation != nullptr);
    assert(allocation->location.kind == LOCATION_GPR);
    x64_GPR gpr;
    if (x64_gprp_first_available(gprp, &gpr, size_of(allocation->type))) {
        x64_gprp_release(gprp, allocation->location.gpr);
        x64_gprp_allocate_to_gpr(gprp, gpr);
        allocation->location.gpr = gpr;
        return true;
    }
    return false;
}

static x64_StackFrame x64_stack_frame_create() {
    x64_StackFrame stack_frame = {.active_size = 0, .total_size = 0};
    return stack_frame;
}

static void x64_stack_frame_allocate_size(x64_StackFrame *stack_frame,
                                          u64 size) {
    assert(stack_frame != nullptr);
    assert(size != 0);
    assert(size <= i32_MAX);
    stack_frame->active_size += (i32)size;
    stack_frame->total_size =
        max_i32(stack_frame->total_size, stack_frame->active_size);
}

static void x64_stack_frame_allocate(x64_StackFrame *stack_allocations,
                                     x64_Allocation *allocation) {
    assert(stack_allocations != nullptr);
    assert(allocation != nullptr);
    assert(allocation->type != nullptr);
    x64_stack_frame_allocate_size(stack_allocations, size_of(allocation->type));
    allocation->location = x64_location_address(
        x64_address_create(X64_GPR_rBP, -stack_allocations->total_size));
}

[[maybe_unused]] static void
x64_stack_frame_allocate_to_address(x64_StackFrame *stack_frame,
                                    x64_Allocation *allocation,
                                    x64_Address address) {
    assert(stack_frame != nullptr);
    assert(allocation != nullptr);
    assert(allocation->type != nullptr);
    x64_stack_frame_allocate_size(stack_frame, size_of(allocation->type));
    allocation->location = x64_location_address(address);
}

static void x64_stack_frame_deallocate(x64_StackFrame *stack_frame,
                                       x64_Allocation *allocation) {
    assert(stack_frame != nullptr);
    assert(allocation != nullptr);
    u64 size = size_of(allocation->type);
    assert(size != 0);
    assert(size <= i32_MAX);
    stack_frame->active_size -= (i32)size;
}

static void x64_allocations_initialize(x64_Allocations *allocations) {
    assert(allocations != nullptr);
    allocations->capacity = 0;
    allocations->buffer   = nullptr;
}

static void x64_allocations_destroy(x64_Allocations *allocations) {
    assert(allocations != nullptr);
    for (u64 i = 0; i < allocations->capacity; ++i) {
        if (allocations->buffer[i] == nullptr) { continue; }
        x64_allocation_deallocate(allocations->buffer[i]);
    }

    deallocate(allocations->buffer);
    allocations->buffer   = nullptr;
    allocations->capacity = 0;
}

static void x64_allocations_grow(x64_Allocations *allocation_buffer) {
    assert(allocation_buffer != nullptr);
    Growth64 g =
        array_growth_u64(allocation_buffer->capacity, sizeof(x64_Allocation *));
    allocation_buffer->buffer =
        reallocate(allocation_buffer->buffer, g.alloc_size);
    allocation_buffer->capacity = g.new_capacity;
}

static x64_Allocation *
x64_allocations_allocate(x64_Allocations *allocation_buffer,
                         u64 ssa,
                         Lifetime lifetime,
                         Type const *type) {
    assert(allocation_buffer != nullptr);
    assert(type != nullptr);

    if (allocation_buffer->capacity <= ssa) {
        x64_allocations_grow(allocation_buffer);
    }

    x64_Allocation **allocation = allocation_buffer->buffer + ssa;
    if (*allocation != nullptr) { return *allocation; }
    *allocation             = x64_allocation_allocate();
    (*allocation)->ssa      = ssa;
    (*allocation)->lifetime = lifetime;
    (*allocation)->type     = type;
    return *allocation;
}

static x64_Allocation *x64_allocations_at(x64_Allocations *allocation_buffer,
                                          u64 ssa) {
    assert(allocation_buffer != nullptr);
    assert(allocation_buffer->capacity > ssa);
    return allocation_buffer->buffer[ssa];
}

void x64_allocator_initialize(x64_LocalRegisterAllocator *allocator,
                              FunctionBody *body,
                              x64_Context *x64_context) {
    assert(allocator != nullptr);
    assert(body != nullptr);
    assert(x64_context != nullptr);
    allocator->context     = x64_context;
    allocator->gprp        = x64_gprp_create();
    allocator->stack_frame = x64_stack_frame_create();
    x64_allocations_initialize(&allocator->allocations);
    lifetimes_initialize(&allocator->lifetimes, body, x64_context);

    // #NOTE: pre-color %rsp and %rbp for stack frame management.
    x64_gprp_aquire(&allocator->gprp, X64_GPR_rSP);
    x64_gprp_aquire(&allocator->gprp, X64_GPR_rBP);
}

void x64_allocator_terminate(x64_LocalRegisterAllocator *allocator) {

    allocator->stack_frame = x64_stack_frame_create();
    x64_allocations_destroy(&allocator->allocations);
    lifetimes_destroy(&allocator->lifetimes);
}

bool x64_allocator_uses_stack(x64_LocalRegisterAllocator *allocator) {
    assert(allocator != nullptr);
    return allocator->stack_frame.total_size > 0;
}

i32 x64_allocator_total_stack_size(x64_LocalRegisterAllocator *allocator) {
    assert(allocator != nullptr);
    return allocator->stack_frame.total_size;
}

void x64_allocator_update_lifetime(x64_LocalRegisterAllocator *allocator,
                                   u32 ssa,
                                   Lifetime lifetime) {
    assert(allocator != nullptr);
    lifetimes_update(&allocator->lifetimes, ssa, lifetime);
}

static void
x64_allocator_release_expired_lifetimes(x64_LocalRegisterAllocator *allocator,
                                        u64 block_index) {
    assert(allocator != nullptr);
    x64_Allocations *allocations = &allocator->allocations;
    for (u64 i = 0; i < allocations->capacity; ++i) {
        x64_Allocation *allocation = allocations->buffer[i];
        if (allocation == nullptr) { continue; }
        if (!x64_allocation_alive(allocation, block_index)) {
            x64_allocation_deallocate(allocation);
            allocations->buffer[i] = nullptr;
        }
    }
}

static void
x64_allocator_spill_allocation(x64_LocalRegisterAllocator *allocator,
                               x64_Allocation *allocation) {
    assert(allocation->location.kind == LOCATION_GPR);
    x64_GPR prev_gpr = allocation->location.gpr;
    x64_gprp_release(&allocator->gprp, prev_gpr);
    x64_stack_frame_allocate(&allocator->stack_frame, allocation);
    assert(allocation->location.kind == LOCATION_ADDRESS);

    x64_block_append(x64_context_current_x64_block(allocator->context),
                     x64_mov(x64_operand_address(allocation->location.address),
                             x64_operand_gpr(prev_gpr)));
}

x64_Allocation *
x64_allocator_allocation_at(x64_LocalRegisterAllocator *allocator, u64 ssa) {
    return x64_allocations_at(&allocator->allocations, ssa);
}

x64_Allocation *x64_allocator_oldest_register_allocation(
    x64_LocalRegisterAllocator *allocator) {
    assert(allocator != nullptr);
    x64_Allocation *oldest       = nullptr;
    x64_Allocations *allocations = &allocator->allocations;
    for (u64 i = 0; i < allocations->capacity; ++i) {
        x64_Allocation *allocation = allocations->buffer[i];
        if (allocation == nullptr) { continue; }
        if (allocation->location.kind != LOCATION_GPR) { continue; }
        if ((oldest == nullptr) ||
            (oldest->lifetime.last_use < allocation->lifetime.last_use)) {
            oldest = allocation;
        }
    }
    return oldest;
}

x64_Allocation *
x64_allocator_allocation_at_gpr(x64_LocalRegisterAllocator *allocator,
                                x64_GPR gpr) {
    // #NOTE: since we no longer store references to the active allocations
    //  within the GPRegisterPool we are forced to search for the allocation
    //  in the list of all allocations. Since we store allocations in the
    //  allocation's buffer using their u32 label as the index,
    //  we could jump directly to the allocation,
    //  iff we knew the u32 label of the given gpr allocation.
    assert(allocator != nullptr);
    x64_Allocations *allocations = &allocator->allocations;
    for (u64 i = 0; i < allocations->capacity; ++i) {
        x64_Allocation *allocation = allocations->buffer[i];
        if (allocation == nullptr) { continue; }
        if (allocation->location.kind != LOCATION_GPR) { continue; }
        if (x64_gpr_overlap(allocation->location.gpr, gpr)) {
            return allocation;
        }
    }

    return nullptr;
}

void x64_allocator_deallocate(x64_LocalRegisterAllocator *allocator,
                              x64_Allocation *active) {
    assert(allocator != nullptr);
    assert(active != nullptr);
    x64_Allocations *allocations = &allocator->allocations;
    x64_Allocation **allocation  = allocations->buffer + active->ssa;
    if (*allocation == nullptr) { return; }
    x64_allocation_deallocate(*allocation);
    *allocation = nullptr;
}

void x64_allocator_reallocate_active(x64_LocalRegisterAllocator *allocator,
                                     x64_Allocation *active,
                                     u64 block_index) {
    // #NOTE: if the active allocation is allocated to the stack,
    //  there is no need to reallocate it. So we simply check if
    //  we can deallocate it.
    if (active->location.kind == LOCATION_ADDRESS) {
        if (x64_allocation_alive(active, block_index)) { return; }
        x64_stack_frame_deallocate(&allocator->stack_frame, active);
        x64_allocator_deallocate(allocator, active);
        return;
    }

    // #NOTE: if the active allocation is allocated to a GPRegister
    //  then we try and move it to the next available GPRegister
    //  otherwise we spill it to the stack.
    x64_GPR prev_gpr = active->location.gpr;
    if (x64_gprp_move_allocation_to_first_available(&allocator->gprp, active)) {
        x64_block_append(x64_context_current_x64_block(allocator->context),
                         x64_mov(x64_operand_gpr(active->location.gpr),
                                 x64_operand_gpr(prev_gpr)));
    } else {
        x64_allocator_spill_allocation(allocator, active);
    }
}

void x64_allocator_release_gpr(x64_LocalRegisterAllocator *allocator,
                               x64_GPR gpr,
                               u64 block_index) {
    x64_Allocation *active = x64_allocator_allocation_at_gpr(allocator, gpr);
    if (active == nullptr) {
        x64_gprp_release(&allocator->gprp, gpr);
        return;
    } else if (active->lifetime.last_use < block_index) {
        x64_allocator_deallocate(allocator, active);
    }

    x64_allocator_reallocate_active(allocator, active, block_index);
}

void x64_allocator_aquire_gpr(x64_LocalRegisterAllocator *allocator,
                              x64_GPR gpr,
                              u64 block_index) {
    x64_Allocation *active = x64_allocator_allocation_at_gpr(allocator, gpr);
    if (active == nullptr) {
        x64_gprp_aquire(&allocator->gprp, gpr);
        return;
    }

    if (active->lifetime.last_use < block_index) {
        x64_allocator_deallocate(allocator, active);
        return;
    }

    x64_allocator_reallocate_active(allocator, active, block_index);
}

x64_Allocation *
x64_allocator_allocate_to_gpr(x64_LocalRegisterAllocator *allocator,
                              LocalVariable *local,
                              x64_GPR gpr,
                              u64 block_index) {
    x64_allocator_release_gpr(allocator, gpr, block_index);

    Lifetime *lifetime = lifetimes_at(&allocator->lifetimes, local->ssa);
    x64_Allocation *allocation = x64_allocations_allocate(
        &allocator->allocations, local->ssa, *lifetime, local->type);

    x64_gprp_aquire(&allocator->gprp, gpr);
    return allocation;
}

x64_Allocation *
x64_allocator_allocate_at_address(x64_LocalRegisterAllocator *allocator,
                                  LocalVariable *local,
                                  x64_Address address) {
    Lifetime *lifetime = lifetimes_at(&allocator->lifetimes, local->ssa);
    x64_Allocation *allocation = x64_allocations_allocate(
        &allocator->allocations, local->ssa, *lifetime, local->type);
    allocation->location = x64_location_address(address);
    // x64_stack_frame_allocate_to_address(
    //     &allocator->stack_frame, allocation, address);
    return allocation;
}

x64_Allocation *
x64_allocator_allocate_result(x64_LocalRegisterAllocator *allocator,
                              x64_Location location,
                              Type const *type) {
    x64_Allocation *allocation = x64_allocations_allocate(
        &allocator->allocations, 0, lifetime_immortal(), type);

    allocation->location = location;

    return allocation;
}

x64_GPR x64_allocator_aquire_any_gpr(x64_LocalRegisterAllocator *allocator,
                                     u64 block_index,
                                     u64 size) {
    x64_allocator_release_expired_lifetimes(allocator, block_index);

    x64_GPR gpr = 0;
    if (x64_gprp_first_available(&allocator->gprp, &gpr, size)) {
        x64_gprp_aquire(&allocator->gprp, gpr);
        return gpr;
    }

    x64_Allocation *oldest =
        x64_allocator_oldest_register_allocation(allocator);
    x64_allocator_spill_allocation(allocator, oldest);

    if (x64_gprp_first_available(&allocator->gprp, &gpr, size)) {
        x64_gprp_aquire(&allocator->gprp, gpr);
        return gpr;
    }
    EXP_UNREACHABLE();
}

static void x64_allocator_stack_allocate(x64_LocalRegisterAllocator *allocator,
                                         x64_Allocation *allocation) {
    x64_stack_frame_allocate(&allocator->stack_frame, allocation);
}

static void
x64_allocator_register_allocate(x64_LocalRegisterAllocator *allocator,
                                x64_Allocation *allocation,
                                u64 block_index) {
    x64_allocator_release_expired_lifetimes(allocator, block_index);

    if (x64_gprp_allocate_to_first_available(&allocator->gprp, allocation)) {
        return;
    }

    // otherwise spill the oldest active register allocation to the stack.
    x64_Allocation *oldest_active =
        x64_allocator_oldest_register_allocation(allocator);

    if (oldest_active->lifetime.last_use > allocation->lifetime.last_use) {
        x64_allocator_spill_allocation(allocator, oldest_active);
        x64_gprp_allocate_to_first_available(&allocator->gprp, allocation);
    } else {
        x64_allocator_stack_allocate(allocator, allocation);
    }
}

x64_Allocation *x64_allocator_allocate(x64_LocalRegisterAllocator *allocator,
                                       LocalVariable *local,
                                       u64 block_index) {
    Lifetime *lifetime = lifetimes_at(&allocator->lifetimes, local->ssa);
    x64_Allocation *allocation = x64_allocations_allocate(
        &allocator->allocations, local->ssa, *lifetime, local->type);

    if (string_view_empty(local->name) && type_is_scalar(local->type)) {
        x64_allocator_register_allocate(allocator, allocation, block_index);
    } else {
        x64_allocator_stack_allocate(allocator, allocation);
    }

    return allocation;
}

x64_Allocation *
x64_allocator_allocate_from_active(x64_LocalRegisterAllocator *allocator,
                                   u64 block_index,
                                   LocalVariable *local,
                                   x64_Allocation *active) {
    Lifetime *lifetime = lifetimes_at(&allocator->lifetimes, local->ssa);

    if (!x64_allocation_alive(active, block_index)) {
        // we can reuse the existing allocation treating
        // it as the new ssa local allocation.
        active->ssa      = local->ssa;
        active->lifetime = *lifetime;
        return active;
    }

    // we have to keep the existing allocation around
    x64_Allocation *new = x64_allocator_allocate(allocator, local, block_index);
    x64_Block *x64_bytecode = x64_context_current_x64_block(allocator->context);

    // initialize the new allocation
    if ((active->location.kind == LOCATION_ADDRESS) &&
        (new->location.kind == LOCATION_ADDRESS)) {
        x64_GPR gpr = x64_allocator_aquire_any_gpr(
            allocator, block_index, size_of(new->type));
        x64_block_append(
            x64_bytecode,
            x64_mov(x64_operand_gpr(gpr),
                    x64_operand_address(active->location.address)));
        x64_block_append(x64_bytecode,
                         x64_mov(x64_operand_address(new->location.address),
                                 x64_operand_gpr(gpr)));
    } else {
        x64_block_append(x64_bytecode,
                         x64_mov(x64_operand_location(new->location),
                                 x64_operand_location(active->location)));
    }

    return new;
}

x64_Allocation *
x64_allocator_allocate_to_address(x64_LocalRegisterAllocator *allocator,
                                  LocalVariable *local,
                                  x64_Address address) {
    assert(allocator != nullptr);
    assert(local != nullptr);
    Lifetime lifetime = *lifetimes_at(&allocator->lifetimes, local->ssa);
    x64_Allocation *allocation = x64_allocations_allocate(
        &allocator->allocations, local->ssa, lifetime, local->type);
    allocation->location = x64_location_address(address);
    return allocation;
}
