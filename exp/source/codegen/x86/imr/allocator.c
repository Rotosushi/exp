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

#include "codegen/x86/imr/allocator.h"
#include "codegen/x86/imr/registers.h"
#include "support/allocation.h"
#include "support/array_growth.h"
#include "support/assert.h"
#include "support/panic.h"
#include "support/unreachable.h"

static void x86_register_allocations_create(
    x86_RegisterAllocations *restrict register_allocations);
static void
x86_stack_allocations_create(x86_StackAllocations *restrict stack_allocations);
static void x86_allocations_create(x86_Allocations *restrict allocations);

void x86_allocator_create(x86_Allocator *restrict allocator) {
    exp_assert(allocator != NULL);
    x86_register_allocations_create(&allocator->register_allocations);
    x86_stack_allocations_create(&allocator->stack_allocations);
    x86_allocations_create(&allocator->allocations);
}

static void
x86_registers_destroy(x86_RegisterAllocations *restrict register_allocations);
static void
x86_stack_frame_destroy(x86_StackAllocations *restrict stack_allocations);
static void x86_allocations_destroy(x86_Allocations *restrict allocations);

void x86_allocator_destroy(x86_Allocator *restrict allocator) {
    exp_assert(allocator != NULL);
    x86_registers_destroy(&allocator->register_allocations);
    x86_stack_frame_destroy(&allocator->stack_allocations);
    x86_allocations_destroy(&allocator->allocations);
}

static void x86_release_expired_allocations(x86_Allocator *restrict allocator,
                                            u32 block_index);

static x86_Allocation *
x86_allocations_append(x86_Allocations *restrict allocations,
                       Local const *restrict local,
                       Context *restrict context);

static bool x86_allocate_to_next_available_register(
    x86_RegisterAllocations *restrict register_allocations,
    x86_Allocation *restrict allocation);

static void
x86_allocate_to_stack(x86_StackAllocations *restrict stack_allocations,
                      x86_Allocation *restrict allocation);

x86_Allocation *x86_allocator_allocate_local(x86_Allocator *restrict allocator,
                                             Local const *restrict local,
                                             u32 block_index,
                                             Context *restrict context) {
    exp_assert(allocator != NULL);
    exp_assert(local != NULL);
    exp_assert(context != NULL);

    x86_release_expired_allocations(allocator, block_index);

    x86_Allocation *allocation =
        x86_allocations_append(&allocator->allocations, local, context);

    // #NOTE: For now, we always allocate local variables onto
    // the stack. I don't know how exactly to express it yet,
    // but if a local variable fits into a register, we should
    // try to place it into one. As registers are faster than
    // memory. Perhaps this is done by a register promotion pass?
    if (!string_view_empty(allocation->name)) {
        x86_allocate_to_stack(&allocator->stack_allocations, allocation);
        return allocation;
    }

    u64 size = x86_layout_size_of(allocation->layout);
    if ((x86_gpr_valid_size(size)) &&
        (x86_allocate_to_next_available_register(
            &allocator->register_allocations, allocation))) {
        return allocation;
    }

    x86_allocate_to_stack(&allocator->stack_allocations, allocation);
    return allocation;
}

static void x86_release_expired_register_allocations(
    x86_RegisterAllocations *restrict register_allocations, u32 block_index);

// #NOTE:
// 1: search the existing allocations for any whose lifetime is not
//    inclusive of the given block_index.
// 2: for each expired allocation mark the stack allocation as free
static void x86_release_expired_stack_allocations(
    x86_StackAllocations *restrict stack_allocations, u32 block_index);

static void x86_release_expired_allocations(x86_Allocator *restrict allocator,
                                            u32 block_index) {
    x86_release_expired_register_allocations(&allocator->register_allocations,
                                             block_index);
    x86_release_expired_stack_allocations(&allocator->stack_allocations,
                                          block_index);
}

static x86_Allocation *
x86_allocations_append(x86_Allocations *restrict allocations,
                       Local const *restrict local,
                       Context *restrict context);

/**
 * @brief allocate the given allocation to the next available GPR
 *
 * @pre the allocation must be a valid size for a GPR.
 *
 * @return true if able to allocate
 * @return false otherwise
 */
static bool x86_allocate_to_next_available_register(
    x86_RegisterAllocations *restrict register_allocations,
    x86_Allocation *restrict allocation) {
    x86_RegisterPool *pool   = &register_allocations->pool;
    x86_Allocation  **buffer = register_allocations->buffer;
    u8                gpr_index;
    if (!x86_register_pool_gpr_aquire_next_available(pool, &gpr_index)) {
        return false;
    }

    u64 size = x86_layout_size_of(allocation->layout);
    exp_assert_debug(x86_gpr_valid_size(size));
    x86_GPR gpr          = x86_gpr_with_size(gpr_index, size);
    allocation->location = x86_location_gpr(gpr);
    buffer[gpr_index]    = allocation;
    return true;
}

// #NOTE: the location of the given allocation on the stack is
// address of the first byte of it's data. given that the stack
// grows downwards we need to give a negative offset to the base
// pointer. The simplest solution is to add the size of the new
// allocation to the active stack size, and return the new active
// stack size as the offset relative to the base pointer.
// however, this does not account for the fact that we need to access
// the memory properly aligned. which means that the stack frame itself
// is layed out like a tuple, where we need to add padding between elements
// of differing alignments.
//
// 1: search the existing allocations for enough free space for the new
//    allocation. The available free space includes the padding between the
//    free spot and the next element, we can simply decrement the padding
//    or remove it entirely if need be. If we do this here, we avoid
//    shuffling all the data in the array twice, once when we would coalesce
//    the padding with the free allocation, and once when we add the padding
//    back once we reuse the spot. (obviously neither of these will always
//    happen, however if we reuse the padding now, we only have the potential
//    of shuffling the elements in the array in the case where we need all of
//    the available free space and all of the padding exactly.)
//    1a: There was an empty spot, we reuse it
// 2: There was no empty spot, so we add more space onto the end of the
//    stack, enough for the new allocation, and place it there.
//    taking care to include padding between the previous element if
//    needed.

static void
x86_allocate_to_stack(x86_StackAllocations *restrict stack_allocations,
                      x86_Allocation *restrict allocation) {
    // #NOTE: We address locals relative to the stack
    // frames base pointer. The offset is the address of the
    // first byte of the stack space allocated.
    u64 size      = x86_allocation_size_of(allocation);
    u64 alignment = x86_allocation_align_of(allocation);
    // #NOTE: for now, this is simply asserted. however there is
    // a more robust and friendly way of handling this i'm sure.
    exp_assert(size < i32_MAX);

    x86_PtrKind ptr_kind = x86_allocation_ptr_kind_of(allocation);

    i32 offset;

    x86_Location address = x86_location_address(X86_GPR_RBP, ptr_kind, offset);
}

// /**
//  * @brief moves the allocation from it's current GPR to the next available
//  * (different) GPR
//  *
//  * @pre the allocation must be allocated to a GPR managed by this GPRP
//  * @pre the allocation must be a valid size for a GPR
//  *
//  * @return true if able to allocate
//  * @return false otherwise
//  */
// static bool x86_reallocate_to_any_gpr(x86_Allocator *restrict allocator,
//                                       x86_Allocation *restrict allocation) {
//     exp_assert(!allocation->location.is_address);
//     u8 current_gpr_index = x86_gpr_to_index(allocation->location.gpr);

//     u8 new_gpr_index;
//     if (x86_gprp_next_available(&allocator->gprp, &new_gpr_index)) {
//         x86_gprp_release(&allocator->gprp, allocation->location.gpr);

//         x86_allocate_to_gpr_index(allocator, allocation, new_gpr_index);
//         return true;
//     }
//     return false;
// }

// static x86_Allocation *
// x86_allocator_allocation_at_gpr(x86_Allocator *restrict allocator,
//                                 x86_GPR gpr) {
//     return allocator->gpr_allocations[x86_gpr_to_index(gpr)];
// }

// static x86_Allocation *x86_gprp_allocation_of(x86_GPRP *restrict gprp,
//                                               u64 ssa) {
//     for (u8 i = 0; i < 16; ++i) {
//         x86_Allocation *cursor = gprp->buffer[i];
//         if (cursor == NULL) { continue; }
//         if (cursor->ssa == ssa) { return cursor; }
//     }

//     return NULL;
// }

// static x86_Allocation *x86_gprp_oldest_allocation(x86_GPRP *restrict gprp) {
//     x86_Allocation *oldest = NULL;

//     for (u8 i = 0; i < 16; ++i) {
//         x86_Allocation *cursor = gprp->buffer[i];
//         if (cursor == NULL) { continue; }

//         if ((oldest == NULL) || (oldest->lifetime.end <
//         cursor->lifetime.end)) {
//             oldest = cursor;
//         }
//     }
//     return oldest;
// }

// static void x86_gprp_release_expired_allocations(x86_GPRP *restrict gprp,
//                                                  u64 Idx) {
//     for (u8 i = 0; i < 16; ++i) {
//         x86_Allocation *cursor = gprp->buffer[i];
//         if (cursor == NULL) { continue; }

//         if (cursor->lifetime.end < Idx) { x86_gprp_release_index(gprp, i); }
//     }
// }

// static x86_StackAllocations x86_stack_allocations_create() {
//     x86_StackAllocations stack_allocations = {
//         .active_stack_size = 0,
//         .total_stack_size  = 0,
//         .count             = 0,
//         .capacity          = 0,
//         .buffer            = NULL,
//     };
//     return stack_allocations;
// }

// static void x86_stack_allocations_destroy(
//     x86_StackAllocations *restrict stack_allocations) {
//     if (stack_allocations->buffer != NULL) {
//         deallocate(stack_allocations->buffer);
//     }

//     stack_allocations->count             = 0;
//     stack_allocations->capacity          = 0;
//     stack_allocations->active_stack_size = 0;
//     stack_allocations->total_stack_size  = 0;
// }

// static bool
// x86_stack_allocations_full(x86_StackAllocations *restrict stack_allocations)
// {
//     return (stack_allocations->count + 1) >= stack_allocations->capacity;
// }

// static void
// x86_stack_allocations_grow(x86_StackAllocations *restrict stack_allocations)
// {
//     Growth_u64 g =
//         array_growth_u64(stack_allocations->capacity, sizeof(x86_Allocation
//         *));
//     stack_allocations->buffer =
//         reallocate(stack_allocations->buffer, g.alloc_size);
//     stack_allocations->capacity = g.new_capacity;
// }

// static void
// x86_stack_allocations_append(x86_StackAllocations *restrict
// stack_allocations,
//                              x86_Allocation *restrict allocation) {
//     if (x86_stack_allocations_full(stack_allocations)) {
//         x86_stack_allocations_grow(stack_allocations);
//     }

//     stack_allocations->buffer[stack_allocations->count] = allocation;
//     stack_allocations->count += 1;
// }

// static void
// x86_stack_allocations_allocate(x86_StackAllocations *restrict
// stack_allocations,
//                                x86_Allocation *restrict allocation) {
//     if (__builtin_add_overflow(stack_allocations->total_stack_size,
//                                size_of(allocation->type),
//                                &stack_allocations->total_stack_size)) {
//         PANIC("computed stack size overflow");
//     }

//     i64 offset = stack_allocations->total_stack_size;

//     allocation->location = x86_location_address(X86_GPR_RBP, -offset);

//     x86_stack_allocations_append(stack_allocations, allocation);
// }

// static void
// x86_stack_allocations_erase(x86_StackAllocations *restrict stack_allocations,
//                             x86_Allocation *restrict allocation) {
//     u64 i = 0;
//     for (; i < stack_allocations->count; ++i) {
//         x86_Allocation *cursor = stack_allocations->buffer[i];
//         if (cursor == allocation) { break; }
//     }

//     if (i == stack_allocations->count) { return; }

//     for (; i < stack_allocations->count; ++i) {
//         stack_allocations->buffer[i] = stack_allocations->buffer[i + 1];
//     }

//     stack_allocations->count -= 1;
// }

// static void x86_stack_allocations_release_expired_allocations(
//     x86_StackAllocations *restrict stack_allocations, u64 Idx) {
//     for (u64 i = 0; i < stack_allocations->count; ++i) {
//         x86_Allocation *cursor = stack_allocations->buffer[i];
//         if (cursor->lifetime.end < Idx) {
//             x86_stack_allocations_erase(stack_allocations, cursor);
//             --i;
//         }
//     }
// }

// static x86_Allocation *
// x86_stack_allocations_of(x86_StackAllocations *restrict stack_allocations,
//                          u64 ssa) {
//     for (u64 i = 0; i < stack_allocations->count; ++i) {
//         x86_Allocation *cursor = stack_allocations->buffer[i];
//         if (cursor->ssa == ssa) { return cursor; }
//     }
//     EXP_UNREACHABLE();
// }

// static x86_AllocationBuffer x86_allocation_buffer_create() {
//     x86_AllocationBuffer allocation_buffer = {
//         .count = 0, .capacity = 0, .buffer = NULL};
//     return allocation_buffer;
// }

// static void x86_allocation_buffer_destroy(
//     x86_AllocationBuffer *restrict allocation_buffer) {
//     assert(allocation_buffer != NULL);
//     for (u64 i = 0; i < allocation_buffer->count; ++i) {
//         x86_allocation_deallocate(allocation_buffer->buffer[i]);
//     }

//     deallocate(allocation_buffer->buffer);
//     allocation_buffer->buffer   = NULL;
//     allocation_buffer->count    = 0;
//     allocation_buffer->capacity = 0;
// }

// static bool
// x86_allocation_buffer_full(x86_AllocationBuffer *restrict allocation_buffer)
// {
//     return (allocation_buffer->count + 1) >= allocation_buffer->capacity;
// }

// static void
// x86_allocation_buffer_grow(x86_AllocationBuffer *restrict allocation_buffer)
// {
//     Growth_u64 g =
//         array_growth_u64(allocation_buffer->capacity, sizeof(x86_Allocation
//         *));
//     allocation_buffer->buffer =
//         reallocate(allocation_buffer->buffer, g.alloc_size);
//     allocation_buffer->capacity = g.new_capacity;
// }

// static x86_Allocation *
// x86_allocation_buffer_append(x86_AllocationBuffer *restrict
// allocation_buffer,
//                              Local *restrict local) {
//     assert(allocation_buffer != NULL);
//     assert(local != NULL);

//     if (x86_allocation_buffer_full(allocation_buffer)) {
//         x86_allocation_buffer_grow(allocation_buffer);
//     }

//     x86_Allocation **allocation =
//         allocation_buffer->buffer + allocation_buffer->count;
//     allocation_buffer->count += 1;
//     *allocation             = x86_allocation_allocate();
//     (*allocation)->ssa      = local->ssa;
//     (*allocation)->lifetime = local->lifetime;
//     (*allocation)->type     = local->type;
//     return *allocation;
// }

// void x86_allocator_create(x86_Allocator *restrict allocator) {
//     exp_assert(allocator != NULL);
//     allocator->gprp              = x86_gprp_create();
//     allocator->stack_allocations = x86_stack_allocations_create();
//     allocator->allocations       = x86_allocation_buffer_create();
//     x86_gprp_aquire(&allocator->gprp, X86_GPR_RSP);
//     x86_gprp_aquire(&allocator->gprp, X86_GPR_RBP);
// }

// void x86_allocator_destroy(x86_Allocator *restrict allocator) {
//     x86_gprp_destroy(&allocator->gprp);
//     x86_stack_allocations_destroy(&allocator->stack_allocations);
//     x86_allocation_buffer_destroy(&allocator->allocations);
// }

// bool x86_allocator_uses_stack(x86_Allocator *restrict allocator) {
//     return allocator->stack_allocations.total_stack_size > 0;
// }

// i64 x86_allocator_total_stack_size(x86_Allocator *restrict allocator) {
//     return allocator->stack_allocations.total_stack_size;
// }

// static void
// x86_allocator_release_expired_lifetimes(x86_Allocator *restrict allocator,
//                                         u64 Idx) {
//     x86_stack_allocations_release_expired_allocations(
//         &allocator->stack_allocations, Idx);
//     x86_gprp_release_expired_allocations(&allocator->gprp, Idx);
// }

// static void x86_allocator_spill_allocation(x86_Allocator *restrict allocator,
//                                            x86_Allocation *restrict
//                                            allocation, x86_Bytecode *restrict
//                                            x64bc) {
//     assert(allocation->location.kind == X86_LOCATION_GPR);
//     x86_GPR gpr = allocation->location.gpr;
//     x86_gprp_release(&allocator->gprp, gpr);
//     x86_stack_allocations_allocate(&allocator->stack_allocations,
//     allocation);

//     x86_bytecode_append(
//         x64bc, x86_mov(x86_operand_alloc(allocation), x86_operand_gpr(gpr)));
// }

// x86_Allocation *x86_allocator_allocation_of(x86_Allocator *restrict
// allocator,
//                                             u64 ssa) {
//     x86_Allocation *allocation = x86_gprp_allocation_of(&allocator->gprp,
//     ssa); if (allocation != NULL) { return allocation; }

//     return x86_stack_allocations_of(&allocator->stack_allocations, ssa);
// }

// void x86_allocator_release_gpr(x86_Allocator *restrict allocator,
//                                x86_GPR gpr,
//                                u64     Idx,
//                                x86_Bytecode *restrict x64bc) {
//     x86_Allocation *active =
//         x86_allocator_allocation_at_gpr(&allocator->gprp, gpr);
//     if ((active == NULL) || active->lifetime.end < Idx) {
//         x86_gprp_release(&allocator->gprp, gpr);
//         return;
//     }

//     x86_allocator_reallocate_active(allocator, active, x64bc);
// }

// void x86_allocator_aquire_gpr(x86_Allocator *restrict allocator,
//                               x86_GPR gpr,
//                               u64     Idx,
//                               x86_Bytecode *restrict x64bc) {
//     x86_Allocation *active =
//         x86_allocator_allocation_at_gpr(&allocator->gprp, gpr);
//     if (active == NULL) {
//         x86_gprp_aquire(&allocator->gprp, gpr);
//         return;
//     }

//     if (active->lifetime.end <= Idx) {
//         x86_gprp_release(&allocator->gprp, gpr);
//         x86_gprp_aquire(&allocator->gprp, gpr);
//         return;
//     }

//     x86_allocator_reallocate_active(allocator, active, x64bc);
// }

// static void x86_allocator_stack_allocate(x86_Allocator *restrict allocator,
//                                          x86_Allocation *restrict allocation)
//                                          {
//     x86_stack_allocations_allocate(&allocator->stack_allocations,
//     allocation);
// }

// static void x86_allocator_register_allocate(x86_Allocator *restrict
// allocator,
//                                             u64 Idx,
//                                             x86_Allocation *restrict
//                                             allocation, x86_Bytecode
//                                             *restrict x64bc) {
//     x86_allocator_release_expired_lifetimes(allocator, Idx);

//     if (x86_allocate_to_any_gpr(&allocator->gprp, allocation)) { return; }

//     // otherwise spill the oldest active allocation to the stack.
//     x86_Allocation *oldest_active =
//         x86_gprp_oldest_allocation(&allocator->gprp);

//     if (oldest_active->lifetime.end > allocation->lifetime.end) {
//         x86_allocator_spill_allocation(allocator, oldest_active, x64bc);
//         x86_allocate_to_any_gpr(&allocator->gprp, allocation);
//     } else {
//         x86_allocator_stack_allocate(allocator, allocation);
//     }
// }

// x86_Allocation *x86_allocator_allocate(x86_Allocator *restrict allocator,
//                                        u64    Idx,
//                                        Local *local,
//                                        x86_Bytecode *restrict x64bc) {
//     x86_Allocation *allocation =
//         x86_allocation_buffer_append(&allocator->allocations, local);

//     if (string_view_empty(local->name) && type_is_scalar(local->type)) {
//         x86_allocator_register_allocate(allocator, Idx, allocation, x64bc);
//     } else {
//         x86_allocator_stack_allocate(allocator, allocation);
//     }

//     return allocation;
// }

// /**
//  * @brief allocate <ssa> in the same location as <active>
//  * reallocating <active> if <active> expires after <Idx>
//  *
//  */
// x86_Allocation *
// x86_allocator_allocate_from_active(x86_Allocator *restrict allocator,
//                                    u64             Idx,
//                                    Local          *local,
//                                    x86_Allocation *active,
//                                    x86_Bytecode *restrict x64bc) {

//     if (active->lifetime.end <= Idx) {
//         // we can reuse the existing allocation treating
//         // it as the new ssa local allocation.
//         active->ssa      = local->ssa;
//         active->lifetime = local->lifetime;
//         return active;
//     }

//     // we have to keep the existing allocation around
//     x86_Allocation *new = x86_allocator_allocate(allocator, Idx, local,
//     x64bc);

//     // initialize the new allocation
//     if ((active->location.kind == X86_LOCATION_ADDRESS) &&
//         (new->location.kind == X86_LOCATION_ADDRESS)) {
//         u64 size = size_of(new->type);
//         exp_assert(x86_gpr_valid_size(size));
//         x86_GPR gpr = x86_allocator_aquire_any_gpr(allocator, size, Idx,
//         x64bc); x86_bytecode_append(
//             x64bc, x86_mov(x86_operand_gpr(gpr), x86_operand_alloc(active)));
//         x86_bytecode_append(
//             x64bc, x86_mov(x86_operand_alloc(new), x86_operand_gpr(gpr)));
//     } else {
//         x86_bytecode_append(
//             x64bc, x86_mov(x86_operand_alloc(new),
//             x86_operand_alloc(active)));
//     }

//     return new;
// }

// u8 x86_allocator_spill_oldest_active(x86_Allocator *restrict allocator,
//                                      x86_Bytecode *restrict x64bc) {
//     x86_Allocation *oldest = x86_gprp_oldest_allocation(&allocator->gprp);
//     if (oldest == NULL) {
//         u8 gpr_index;
//         if (x86_gprp_any_available(&allocator->gprp, &gpr_index)) {
//             return gpr_index;
//         }

//         EXP_UNREACHABLE();
//     }
//     x86_GPR gpr = oldest->location.gpr;
//     x86_allocator_spill_allocation(allocator, oldest, x64bc);
//     return x86_gpr_to_index(gpr);
// }

// x86_Allocation *
// x86_allocator_allocate_to_any_gpr(x86_Allocator *restrict allocator,
//                                   Local *local,
//                                   x86_Bytecode *restrict x64bc) {
//     x86_Allocation *allocation =
//         x86_allocation_buffer_append(&allocator->allocations, local);

//     if (x86_allocate_to_any_gpr(&allocator->gprp, allocation)) {
//         return allocation;
//     }

//     u8 gpr_index = x86_allocator_spill_oldest_active(allocator, x64bc);
//     x86_allocate_to_gpr_index(&allocator->gprp, gpr_index, allocation);
//     return allocation;
// }

// x86_Allocation *x86_allocator_allocate_to_gpr(x86_Allocator *restrict
// allocator,
//                                               Local  *local,
//                                               x86_GPR gpr,
//                                               u64     Idx,
//                                               x86_Bytecode *restrict x64bc) {
//     x86_Allocation *allocation =
//         x86_allocation_buffer_append(&allocator->allocations, local);

//     x86_allocator_release_gpr(allocator, gpr, Idx, x64bc);
//     x86_allocate_to_gpr(&allocator->gprp, gpr, allocation);
//     return allocation;
// }

// x86_Allocation *x86_allocator_allocate_to_stack(
//     x86_Allocator *restrict allocator, i64 offset, Local *local) {
//     x86_Allocation *allocation =
//         x86_allocation_buffer_append(&allocator->allocations, local);

//     allocation->location = x86_location_address(X86_GPR_RBP, offset);

//     x86_stack_allocations_append(&allocator->stack_allocations, allocation);
//     return allocation;
// }

// x86_Allocation *x86_allocator_allocate_result(x86_Allocator *restrict
// allocator,
//                                               x86_Location location,
//                                               Type const  *type) {
//     Local fake = {
//         .ssa = u32_MAX, .lifetime = {0, u32_MAX},
//              .type = type
//     };
//     x86_Allocation *allocation =
//         x86_allocation_buffer_append(&allocator->allocations, &fake);

//     // The gpr allocator of this function does not actually want the
//     // location marked as used, so that instructions within the function
//     // can use the return GPR as a temporary, potentially allowing the
//     // return value to be computed in the same GPR as the return value.
//     // This is a bit of a hack, but it works for now.
//     // The stack_allocator of this function does not handle allocating the
//     // result. the caller allocates space for the result.
//     allocation->location = location;

//     return allocation;
// }

// void x86_allocator_reallocate_active(x86_Allocator *restrict allocator,
//                                      x86_Allocation *restrict active,
//                                      x86_Bytecode *restrict x64bc) {
//     if (active->location.kind == X86_LOCATION_ADDRESS) { return; }

//     x86_GPR prev_gpr = active->location.gpr;
//     if (x86_reallocate_to_any_gpr(&allocator->gprp, active)) {
//         x86_bytecode_append(x64bc,
//                             x86_mov(x86_operand_gpr(active->location.gpr),
//                                     x86_operand_gpr(prev_gpr)));
//     } else {
//         x86_allocator_spill_allocation(allocator, active, x64bc);
//     }
// }

// x86_GPR x86_allocator_aquire_any_gpr(x86_Allocator *restrict allocator,
//                                      u64 size,
//                                      u64 Idx,
//                                      x86_Bytecode *restrict x64bc) {
//     exp_assert(x86_gpr_valid_size(size));
//     x86_allocator_release_expired_lifetimes(allocator, Idx);

//     u8 gpr = 0;
//     if (x86_gprp_any_available(&allocator->gprp, &gpr)) {
//         return x86_gpr_with_size(gpr, size);
//     }

//     gpr = x86_allocator_spill_oldest_active(allocator, x64bc);
//     return x86_gpr_with_size(gpr, size);
// }
