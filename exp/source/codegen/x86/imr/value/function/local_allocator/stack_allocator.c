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

#include "codegen/x86/imr/value/function/local_allocator/stack_allocator.h"
#include "support/allocation.h"
#include "support/array_growth.h"
#include "support/assert.h"
#include "support/unreachable.h"

static void x86_stack_space_allocation(x86_StackSpace *restrict stack_space,
                                       i32 offset,
                                       x86_Allocation *restrict allocation) {
    stack_space->kind            = X86_STACK_SPACE_KIND_ALLOCATION;
    stack_space->data.offset     = offset;
    stack_space->data.allocation = allocation;
}

static void x86_stack_space_padding(x86_StackSpace *restrict stack_space,
                                    i32 offset,
                                    i32 padding) {
    stack_space->kind         = X86_STACK_SPACE_KIND_PADDING;
    stack_space->data.offset  = offset;
    stack_space->data.padding = padding;
}

static x86_Location
x86_stack_space_location(x86_StackSpace *restrict stack_space) {
    EXP_ASSERT(stack_space->kind == X86_STACK_SPACE_KIND_ALLOCATION);
    return x86_location_address(
        X86_GPR_RBP,
        x86_allocation_ptr_kind_of(stack_space->data.allocation),
        -stack_space->data.offset);
}

void x86_stack_allocator_create(x86_StackAllocator *restrict stack_allocator) {
    stack_allocator->size     = 0;
    stack_allocator->length   = 0;
    stack_allocator->capacity = 0;
    stack_allocator->buffer   = NULL;
}

void x86_stack_allocator_destroy(x86_StackAllocator *restrict stack_allocator) {
    deallocate(stack_allocator->buffer);
    x86_stack_allocator_create(stack_allocator);
}

i32 x86_stack_allocator_stack_size(
    x86_StackAllocator *restrict stack_allocator) {
    EXP_ASSERT(stack_allocator != NULL);
    return stack_allocator->size;
}

static bool
x86_stack_allocator_full(x86_StackAllocator const *restrict stack_allocator) {
    return (stack_allocator->length + 1) >= stack_allocator->capacity;
}

static void
x86_stack_allocator_grow(x86_StackAllocator *restrict stack_allocator) {
    Growth_u32 g            = array_growth_u32(stack_allocator->capacity,
                                    sizeof(*stack_allocator->buffer));
    stack_allocator->buffer = reallocate(stack_allocator->buffer, g.alloc_size);
    stack_allocator->capacity = g.new_capacity;
}

static x86_StackSpace *
x86_stack_allocator_insert(x86_StackAllocator *restrict stack_allocator,
                           u32 position) {
    EXP_ASSERT(position <= stack_allocator->capacity);
    EXP_ASSERT(position <= stack_allocator->length);
    if (x86_stack_allocator_full(stack_allocator)) {
        x86_stack_allocator_grow(stack_allocator);
    }

    for (u32 index = stack_allocator->length; index > position; --index) {
        stack_allocator->buffer[index] = stack_allocator->buffer[index - 1];
    }

    stack_allocator->length += 1;
    return stack_allocator->buffer + position;
}

static void
x86_stack_allocator_remove(x86_StackAllocator *restrict stack_allocator,
                           u32 position) {
    EXP_ASSERT(position <= stack_allocator->capacity);
    EXP_ASSERT(position <= stack_allocator->length);

    for (u32 index = stack_allocator->length; index > position; --index) {
        stack_allocator->buffer[index] = stack_allocator->buffer[index - 1];
    }

    stack_allocator->length -= 1;
}

/*
 * #NOTE: the location of the given allocation on the stack is
 * address of the first byte of it's data. given that the stack
 * grows downwards we need to give a negative offset to the base
 * pointer. The simplest solution is to add the size of the new
 * allocation to the active stack size, and return the new active
 * stack size as the offset relative to the base pointer.
 * however, this does not account for the fact that we need to access
 * the memory properly aligned. which means that the stack frame itself
 * is layed out like a tuple, where we need to add padding between elements
 * of differing alignments.
 *
 * 1: search the existing allocations for enough free space for the new
 *    allocation.
 *    1a: There was an empty spot, we reuse it
 * 2: There was no empty spot, so we add more space onto the end of the
 *    stack, enough for the new allocation, and place it there.
 *    taking care to include padding between the previous element if
 *    needed.
 *
 * The available free space includes the padding between the
 * free spot and the next element, we can simply decrement the padding
 * or remove it entirely if need be. If we do this here, we avoid
 * shuffling all the data in the array twice, once when we would coalesce
 * the padding with the free allocation, and once when we add the padding
 * back once we reuse the spot. (obviously neither of these will always
 * happen, however if we reuse the padding now, we only have the potential
 * of shuffling the elements in the array in the case where we need all of
 * the available free space and all of the padding exactly.)
 */

void x86_stack_allocator_allocate_to_next_available(
    x86_StackAllocator *restrict stack_allocator,
    x86_Allocation *restrict allocation) {
    EXP_ASSERT(stack_allocator != NULL);
    EXP_ASSERT(allocation != NULL);

    u64 allocation_size = x86_allocation_size_of(allocation);
    EXP_ASSERT(allocation_size < i32_MAX);
    i32 size = (i32)allocation_size;

    for (u32 index = 0; index < stack_allocator->length; ++index) {
        x86_StackSpace *space = stack_allocator->buffer + index;
        switch (space->kind) {
        case X86_STACK_SPACE_KIND_ALLOCATION: {
            continue;
        }

        case X86_STACK_SPACE_KIND_PADDING: {
            // we can exactly reuse the space, simply convert it from padding
            // to an allocation.
            if (size == space->data.padding) {
                x86_stack_space_allocation(
                    space, space->data.offset, allocation);
                allocation->location = x86_stack_space_location(space);
                return;
            }

            if (size < space->data.padding) {
                // we can allocate at the current location of the padding,
                // we just have to add in a new logical stack space
                // and adjust the value of the padding
                x86_StackSpace *new =
                    x86_stack_allocator_insert(stack_allocator, index);
                x86_stack_space_allocation(new, space->data.offset, allocation);
                allocation->location = x86_stack_space_location(new);

                space->data.offset += size;
                space->data.padding -= size;
                return;
            }

            // size > space->data.padding
            // we cannot reuse this stack space.
            continue;
        }

        default: EXP_UNREACHABLE();
        }
    }

    // we know we need to add more space to the stack itself now.
    // and we know it needs to be a whole new stack space.
    if (x86_stack_allocator_full(stack_allocator)) {
        x86_stack_allocator_grow(stack_allocator);
    }

    stack_allocator->size += size;
    x86_StackSpace *new =
        x86_stack_allocator_insert(stack_allocator, stack_allocator->length);
    x86_stack_space_allocation(new, stack_allocator->size, allocation);
    allocation->location = x86_stack_space_location(new);
    return;
}

// #NOTE:
// 1: search the existing allocations for any whose lifetime is not
//    inclusive of the given block_index.
// 2: for each expired allocation mark the stack allocation as free
void x86_stack_allocator_release_expired(
    x86_StackAllocator *restrict stack_allocator, u32 block_index) {
    EXP_ASSERT(stack_allocator != NULL);
    for (u32 index = 0; index < stack_allocator->length; ++index) {
        x86_StackSpace *space = stack_allocator->buffer + index;
        if (space->kind != X86_STACK_SPACE_KIND_ALLOCATION) { continue; }
        x86_Allocation *allocation = space->data.allocation;
        if (x86_allocation_alive(allocation, block_index)) { continue; }

        // we need to expire this allocation. which means turning it into
        // padding. in order to keep the number of adjacent padding elements
        // down to one. we will combine this new padding element and the next
        // one, iff the next space is padding. Since we only ever insert one
        // element of padding at a time, and only between two allocations, this
        // ensures that we only ever have one element of padding at any logical
        // stack location. so we never need to handle more than one within the
        // allocate function.

        u64 allocation_size = x86_allocation_size_of(allocation);
        EXP_ASSERT(allocation_size < i32_MAX);
        i32 size = (i32)allocation_size;

        // if there is no next element, we don't need to combine.
        if (index == (stack_allocator->length - 1)) {
            x86_allocation_expire(allocation);
            x86_stack_space_padding(space, space->data.offset, size);
            continue;
        }

        x86_StackSpace *next = stack_allocator->buffer + index + 1;
        // if the next element is not padding we don't need to combine.
        if (next->kind == X86_STACK_SPACE_KIND_ALLOCATION) {
            x86_allocation_expire(allocation);
            x86_stack_space_padding(space, space->data.offset, size);
            continue;
        }

        // next is padding, and space is going to become padding
        i32 padding = size + next->data.padding;
        x86_stack_space_padding(space, space->data.offset, padding);
        x86_stack_allocator_remove(stack_allocator, index + 1);
        // since we just removed an element, we want to reduce our index down
        // by one, so that it still references the padding element we just
        // created. Thus when the next iteration of the loop happens and we add
        // one to the index we are pointing at the next element.
        index -= 1;
    }
}
