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
#include "intrinsics/size_of.h"
#include "utility/alloc.h"
#include "utility/array_growth.h"
#include "utility/minmax.h"

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

static void x64_gprp_aquire(x64_GPRP *restrict gprp, x64_GPR r) {
  SET_BIT(gprp->bitset, r);
}

static void x64_gprp_release(x64_GPRP *restrict gprp, x64_GPR r) {
  CLR_BIT(gprp->bitset, r);
  gprp->buffer[r] = NULL;
}

static bool x64_gprp_any_available(x64_GPRP *restrict gprp,
                                   x64_GPR *restrict r) {
  for (u8 i = 0; i < 16; ++i) {
    if (!CHK_BIT(gprp->bitset, i)) {
      *r = (x64_GPR)i;
      return 1;
    }
  }
  return 0;
}

static void x64_gprp_allocate_to_gpr(x64_GPRP *restrict gprp,
                                     x64_GPR gpr,
                                     x64_Allocation *restrict allocation) {
  SET_BIT(gprp->bitset, gpr);
  gprp->buffer[gpr]    = allocation;
  allocation->location = x64_location_reg(gpr);
}

static bool x64_gprp_allocate(x64_GPRP *restrict gprp,
                              x64_Allocation *restrict allocation) {
  x64_GPR gpr;
  if (x64_gprp_any_available(gprp, &gpr)) {
    x64_gprp_allocate_to_gpr(gprp, gpr, allocation);
    return 1;
  }

  return 0;
}

static bool x64_gprp_reallocate(x64_GPRP *restrict gprp,
                                x64_Allocation *restrict allocation) {
  assert(allocation->location.kind == ALLOC_GPR);
  x64_GPR gpr;
  if (x64_gprp_any_available(gprp, &gpr)) {
    x64_gprp_release(gprp, allocation->location.gpr);
    x64_gprp_allocate_to_gpr(gprp, gpr, allocation);
    return 1;
  }
  return 0;
}

static x64_Allocation *x64_gprp_allocation_at(x64_GPRP *restrict gprp,
                                              x64_GPR gpr) {
  return gprp->buffer[gpr];
}

static x64_Allocation *x64_gprp_allocation_of(x64_GPRP *restrict gprp,
                                              u16 ssa) {
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
                                                 u16 Idx) {
  for (u8 i = 0; i < 16; ++i) {
    x64_Allocation *cursor = gprp->buffer[i];
    if (cursor == NULL) { continue; }

    if (cursor->lifetime.last_use <= Idx) { x64_gprp_release(gprp, i); }
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
  Growth g =
      array_growth_u16(stack_allocations->capacity, sizeof(x64_Allocation *));
  stack_allocations->buffer =
      reallocate(stack_allocations->buffer, g.alloc_size);
  stack_allocations->capacity = (u16)g.new_capacity;
}

static bool in_range(u16 offset) { return (offset <= i16_MAX); }

static void
x64_stack_allocations_allocate(x64_StackAllocations *restrict stack_allocations,
                               x64_Allocation *restrict allocation) {
  stack_allocations->active_stack_size +=
      (u16)ulmax(8UL, size_of(allocation->type));
  stack_allocations->total_stack_size =
      (u16)umax(stack_allocations->active_stack_size,
                stack_allocations->total_stack_size);

  if (stack_allocations_full(stack_allocations)) {
    stack_allocations_grow(stack_allocations);
  }

  u16 offset = stack_allocations->active_stack_size;
  assert(in_range(offset));
  allocation->location = x64_location_stack(-((i16)offset));
  stack_allocations->buffer[stack_allocations->count] = allocation;
  stack_allocations->count += 1;
}

static void
x64_stack_allocations_erase(x64_StackAllocations *restrict stack_allocations,
                            x64_Allocation *restrict allocation) {
  u16 i = 0;
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
    x64_StackAllocations *restrict stack_allocations, u16 Idx) {
  for (u16 i = 0; i < stack_allocations->count; ++i) {
    x64_Allocation *cursor = stack_allocations->buffer[i];
    if (cursor->lifetime.last_use <= Idx) {
      x64_stack_allocations_erase(stack_allocations, cursor);
      --i;
    }
  }
}

static x64_Allocation *
x64_stack_allocations_of(x64_StackAllocations *restrict stack_allocations,
                         u16 ssa) {
  for (u16 i = 0; i < stack_allocations->count; ++i) {
    x64_Allocation *cursor = stack_allocations->buffer[i];
    if (cursor->ssa == ssa) { return cursor; }
  }
  return NULL;
}

static x64_AllocationBuffer x64_allocation_buffer_create() {
  x64_AllocationBuffer allocation_buffer = {
      .count = 0, .capacity = 0, .buffer = NULL};
  return allocation_buffer;
}

static void x64_allocation_buffer_destroy(
    x64_AllocationBuffer *restrict allocation_buffer) {
  assert(allocation_buffer != NULL);
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
  Growth g =
      array_growth_u64(allocation_buffer->capacity, sizeof(x64_Allocation));
  allocation_buffer->buffer =
      reallocate(allocation_buffer->buffer, g.alloc_size);
  allocation_buffer->capacity = g.new_capacity;
}

static x64_Allocation *
x64_allocation_buffer_append(x64_AllocationBuffer *restrict allocation_buffer,
                             u16 ssa,
                             Lifetime *lifetime,
                             Type *type) {
  assert(allocation_buffer != NULL);
  assert(lifetime != NULL);
  assert(type != NULL);

  if (x64_allocation_buffer_full(allocation_buffer)) {
    x64_allocation_buffer_grow(allocation_buffer);
  }

  x64_Allocation *allocation =
      allocation_buffer->buffer + allocation_buffer->count;
  allocation_buffer->count += 1;
  allocation->ssa      = ssa;
  allocation->lifetime = *lifetime;
  allocation->type     = type;
  return allocation;
}

x64_Allocator x64_allocator_create(FunctionBody *restrict body) {
  x64_Allocator allocator = {
      .gprp              = x64_gprp_create(),
      .stack_allocations = x64_stack_allocations_create(),
      .allocations       = x64_allocation_buffer_create(),
      .lifetimes         = lifetimes_compute(body),
  };
  x64_gprp_aquire(&allocator.gprp, X64GPR_RSP);
  x64_gprp_aquire(&allocator.gprp, X64GPR_RBP);
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

u16 x64_allocator_total_stack_size(x64_Allocator *restrict allocator) {
  return allocator->stack_allocations.total_stack_size;
}

static void
x64_allocator_release_expired_lifetimes(x64_Allocator *restrict allocator,
                                        u16 Idx) {
  x64_stack_allocations_release_expired_allocations(
      &allocator->stack_allocations, Idx);
  x64_gprp_release_expired_allocations(&allocator->gprp, Idx);
}

static void x64_allocator_spill_allocation(x64_Allocator *restrict allocator,
                                           x64_Allocation *restrict allocation,
                                           x64_Bytecode *restrict x64bc) {
  assert(allocation->location.kind == ALLOC_GPR);
  x64_GPR gpr = allocation->location.gpr;
  x64_gprp_release(&allocator->gprp, gpr);
  x64_stack_allocations_allocate(&allocator->stack_allocations, allocation);

  x64_bytecode_append_mov(x64bc,
                          x64_operand_stack(allocation->location.offset),
                          x64_operand_gpr(gpr));
}

x64_Allocation *x64_allocator_allocation_of(x64_Allocator *restrict allocator,
                                            u16 ssa) {
  x64_Allocation *allocation = x64_gprp_allocation_of(&allocator->gprp, ssa);
  if (allocation != NULL) { return allocation; }

  return x64_stack_allocations_of(&allocator->stack_allocations, ssa);
}

void x64_allocator_release_gpr(x64_Allocator *restrict allocator,
                               x64_GPR gpr,
                               u16 Idx,
                               x64_Bytecode *restrict x64bc) {
  x64_Allocation *active = x64_gprp_allocation_at(&allocator->gprp, gpr);
  if ((active == NULL) || active->lifetime.last_use <= Idx) {
    x64_gprp_release(&allocator->gprp, gpr);
    return;
  }

  x64_allocator_reallocate_active(allocator, active, x64bc);
}

void x64_allocator_aquire_gpr(x64_Allocator *restrict allocator,
                              x64_GPR gpr,
                              u16 Idx,
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
                                            u16 Idx,
                                            x64_Allocation *restrict allocation,
                                            x64_Bytecode *restrict x64bc) {
  x64_allocator_release_expired_lifetimes(allocator, Idx);

  if (x64_gprp_allocate(&allocator->gprp, allocation)) { return; }

  // otherwise spill the oldest active allocation to the stack.
  x64_Allocation *oldest_active = x64_gprp_oldest_allocation(&allocator->gprp);

  if (oldest_active->lifetime.last_use > allocation->lifetime.last_use) {
    x64_allocator_spill_allocation(allocator, oldest_active, x64bc);
    x64_gprp_allocate(&allocator->gprp, allocation);
  } else {
    x64_allocator_stack_allocate(allocator, allocation);
  }
}

x64_Allocation *x64_allocator_allocate(x64_Allocator *restrict allocator,
                                       u16 Idx,
                                       LocalVariable *local,
                                       x64_Bytecode *restrict x64bc) {
  Lifetime *lifetime         = lifetimes_at(&allocator->lifetimes, local->ssa);
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
                                   u16 Idx,
                                   LocalVariable *local,
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
  if ((active->location.kind == ALLOC_STACK) &&
      (new->location.kind == ALLOC_STACK)) {
    x64_GPR gpr = x64_allocator_aquire_any_gpr(allocator, Idx, x64bc);
    x64_bytecode_append_mov(x64bc,
                            x64_operand_gpr(gpr),
                            x64_operand_stack(active->location.offset));
    x64_bytecode_append_mov(
        x64bc, x64_operand_stack(new->location.offset), x64_operand_gpr(gpr));
  } else {
    x64_bytecode_append_mov(
        x64bc, x64_operand_alloc(new), x64_operand_alloc(active));
  }

  return new;
}

x64_Allocation *x64_allocator_allocate_to_gpr(x64_Allocator *restrict allocator,
                                              x64_GPR gpr,
                                              u16 Idx,
                                              LocalVariable *local,
                                              x64_Bytecode *restrict x64bc) {
  x64_allocator_release_gpr(allocator, gpr, Idx, x64bc);

  Lifetime *lifetime         = lifetimes_at(&allocator->lifetimes, local->ssa);
  x64_Allocation *allocation = x64_allocation_buffer_append(
      &allocator->allocations, local->ssa, lifetime, local->type);

  x64_gprp_allocate_to_gpr(&allocator->gprp, gpr, allocation);
  return allocation;
}

x64_Allocation *x64_allocator_allocate_result(x64_Allocator *restrict allocator,
                                              u16 Idx,
                                              LocalVariable *local,
                                              x64_Bytecode *restrict x64bc) {
  if (type_is_scalar(local->type)) {
    return x64_allocator_allocate_to_gpr(
        allocator, X64GPR_RAX, Idx, local, x64bc);
  }

  Lifetime *lifetime         = lifetimes_at(&allocator->lifetimes, local->ssa);
  x64_Allocation *allocation = x64_allocation_buffer_append(
      &allocator->allocations, local->ssa, lifetime, local->type);

  x64_stack_allocations_allocate(&allocator->stack_allocations, allocation);
  return allocation;
}

// static x64_Location x64_argument_location(u8 index) {
//   // #TODO: for now, all possible types are scalar.
//   // so when we introduce types that cannot be passed
//   // by register we have to account for that here.
//   switch (index) {
//   case 0: return x64_location_reg(X64GPR_RDI);
//   case 1: return x64_location_reg(X64GPR_RSI);
//   case 2: return x64_location_reg(X64GPR_RDX);
//   case 3: return x64_location_reg(X64GPR_RCX);
//   case 4: return x64_location_reg(X64GPR_R8);
//   case 5: return x64_location_reg(X64GPR_R9);
//   // the rest of the arguments are passed on the stack.
//   default: {
//     return x64_location_stack(0);
//   }
//   }
// }

// we have to allocate an argument to the expected location.
// this location needs to be agreed upon by both caller and callee.
// for arguments passed in registers, this is straightforward, as
// we can simply use the same register for the same arguments.
// for this we, somewhat arbitrarily, use the system V abi.
// when we pass arguments on the stack, we have to agree a-priori
// about the stack location as well. this is somwhat more difficult.
// but how about we simply pass the first stack argument right above
// the callee's frame?
// since the call instruction implicitly pushes the rIP onto the stack,
// the first stack argument (if it is a single word large) would be at
// 16(%rbp). (where 8(%rbp) is the location of the rIP.)
// this brings up our first issue, until now the stack offsets have all
// been in the same direction. but now, we need both positive and negative
// offsets. however we have been storing our offsets within a u16, meaning
// we cannot store negative numbers. so how do we solve this?
// I think the cleanest solution would be to simply treat arguments as if
// they were regular SSA locals. which means we want to directly encode their
// stack position to simplify code which accesses stack variables.
// this means using an i16 to store stack offsets.
// we need to allocate the result of the call onto the stack as well, if the
// result cannot fit into a register. This would place the first stack argument
// directly above the result.
// the second stack argument is above the first and so on.
// when allocating the actual arguments we push each argument onto the stack
// starting from the last stack argument.
//

// x64_Allocation *
// x64_allocator_allocate_formal_argument(x64_Allocator *restrict allocator,
//                                        u16 Idx,
//                                        x64_FormalArgument *restrict argument,
//                                        x64_Bytecode *restrict x64bc) {
//   return;
// }

void x64_allocator_reallocate_active(x64_Allocator *restrict allocator,
                                     x64_Allocation *restrict active,
                                     x64_Bytecode *restrict x64bc) {
  if (active->location.kind == ALLOC_STACK) { return; }

  x64_GPR prev_gpr = active->location.gpr;
  if (x64_gprp_reallocate(&allocator->gprp, active)) {
    x64_bytecode_append_mov(x64bc,
                            x64_operand_gpr(active->location.gpr),
                            x64_operand_gpr(prev_gpr));
  } else {
    x64_allocator_spill_allocation(allocator, active, x64bc);
  }
}

x64_GPR x64_allocator_spill_oldest_active(x64_Allocator *restrict allocator,
                                          x64_Bytecode *restrict x64bc) {
  x64_Allocation *oldest = x64_gprp_oldest_allocation(&allocator->gprp);
  if (oldest != NULL) {
    x64_GPR gpr = oldest->location.gpr;
    x64_allocator_spill_allocation(allocator, oldest, x64bc);
    return gpr;
  }
  assert(0);
}

x64_GPR x64_allocator_aquire_any_gpr(x64_Allocator *restrict allocator,
                                     u16 Idx,
                                     x64_Bytecode *restrict x64bc) {
  x64_allocator_release_expired_lifetimes(allocator, Idx);

  x64_GPR gpr = 0;
  if (x64_gprp_any_available(&allocator->gprp, &gpr)) { return gpr; }

  gpr = x64_allocator_spill_oldest_active(allocator, x64bc);
  return gpr;
}