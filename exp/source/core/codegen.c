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
#include <stddef.h>
#include <stdlib.h>

#include "backend/x64_allocator.h"
#include "core/codegen.h"
#include "core/directives.h"
#include "utility/alloc.h"
#include "utility/array_growth.h"
#include "utility/config.h"
#include "utility/numeric_conversions.h"
#include "utility/panic.h"

static void codegen_alloc_operand(X64ActiveAllocation *restrict aa,
                                  String *restrict buffer) {
  switch (aa->allocation.kind) {
  case ALLOC_GPR: {
    string_append(buffer, SV("%"));
    string_append(buffer, gpr_to_sv(aa->allocation.gpr));
    break;
  }

  case ALLOC_STACK: {
    string_append(buffer, SV("-"));
    string_append_u64(buffer, aa->allocation.offset);
    string_append(buffer, SV("(%rbp)"));
    break;
  }

  default:
    unreachable();
  }
}

static void codegen_immediate_operand(i64 imm, String *restrict buffer) {
  string_append(buffer, SV("$"));
  string_append_i64(buffer, imm);
}

// if we are using a constant value as an operand,
// we can embed that constant as a immediate.
// #NOTE: only for now! when we support types
// larger than a single machine word this becomes
// more complex. (pretty sure it becomes a stack
// allocation.)
static void codegen_constant_operand(Value *restrict constant,
                                     String *restrict buffer) {
  i64 imm = 0;
  switch (constant->kind) {
  case VALUEKIND_UNINITIALIZED:
    PANIC("uninitialized constant");
    break;

  case VALUEKIND_NIL: {
    break;
  }

  case VALUEKIND_BOOLEAN: {
    imm = (i64)constant->boolean;
    break;
  }

  case VALUEKIND_I64: {
    imm = constant->integer;
    break;
  }

  default:
    unreachable();
  }

  codegen_immediate_operand(imm, buffer);
}

static void codegen_reallocate_gpr(X64Allocator *restrict la,
                                   X64ActiveAllocation *aa,
                                   String *restrict buffer) {
  X64GPR gpr = 0;
  if (gprp_allocate(&la->gprp, &gpr)) {
    // move allocation to new register.
    string_append(buffer, SV("\tmov %"));
    string_append(buffer, gpr_to_sv(aa->allocation.gpr));
    string_append(buffer, SV(", %"));
    string_append(buffer, gpr_to_sv(gpr));
    string_append(buffer, SV("\n"));

    gprp_release(&la->gprp, aa->allocation.gpr);
    // update the existing allocation to the new register
    aa->allocation.gpr = gpr;
    return;
  }

  // spill allocation to the stack
  u16 offset = x64allocator_bump_active_stack_size(la);
  string_append(buffer, SV("\tmov %"));
  string_append(buffer, gpr_to_sv(aa->allocation.gpr));
  string_append(buffer, SV(", -"));
  string_append_u64(buffer, offset);
  string_append(buffer, SV("(%rbp)\n"));
  // update the existing allocation to the new stack slot
  aa->allocation.kind   = ALLOC_STACK;
  aa->allocation.offset = offset;
}

static void codegen_reallocate_local_other_than(X64Allocator *restrict la,
                                                X64ActiveAllocation *aa,
                                                X64GPR avoid,
                                                String *restrict buffer) {
  X64GPR gpr = 0;
  if (gprp_allocate_other_than(&la->gprp, &gpr, avoid)) {
    // move allocation to new register.
    string_append(buffer, SV("\tmov %"));
    string_append(buffer, gpr_to_sv(aa->allocation.gpr));
    string_append(buffer, SV(", %"));
    string_append(buffer, gpr_to_sv(gpr));
    string_append(buffer, SV("\n"));
    // update the existing allocation to the new register
    aa->allocation.gpr = gpr;
    return;
  }

  // spill allocation to the stack
  u16 offset = x64allocator_bump_active_stack_size(la);
  string_append(buffer, SV("\tmov %"));
  string_append(buffer, gpr_to_sv(aa->allocation.gpr));
  string_append(buffer, SV(", -"));
  string_append_u64(buffer, offset);
  string_append(buffer, SV("(%rbp)\n"));
  // update the existing allocation to the new stack slot
  aa->allocation.kind   = ALLOC_STACK;
  aa->allocation.offset = offset;
}

/**
 * @brief release the target gpr, making sure to reallocate
 * any allocated ssa local.
 *
 * @param c
 * @param la
 * @param gpr
 * @param buffer
 */
static void codegen_release_gpr(X64Allocator *restrict la, X64GPR gpr,
                                String *restrict buffer) {
  X64ActiveAllocation *aa = x64allocator_allocation_at(la, gpr);
  if (aa == NULL) {
    return; // gpr is available.
  }
  // else there is an allocation at the given gpr
  codegen_reallocate_local_other_than(la, aa, gpr, buffer);
}

static void codegen_zero_gpr(X64GPR gpr, String *restrict buffer) {
  string_append(buffer, SV("\tmov "));
  codegen_immediate_operand((i64)0, buffer);
  string_append(buffer, SV(", %"));
  string_append(buffer, gpr_to_sv(gpr));
  string_append(buffer, SV("\n"));
}

/**
 * @brief force release some gpr, prioritizing old allocations, making sure to
 * reallocate any ssa local.
 *
 * @param la
 * @param buffer
 * @return X64GPR
 */
static X64GPR codegen_release_any_gpr(X64Allocator *restrict la, u16 Idx,
                                      String *restrict buffer) {
  // attempt to release an old active allocation in a gpr.
  // this could release nothing, a gpr, or an old stack
  // allocation.
  x64allocator_expire_old_lifetimes(la, Idx);

  X64GPR gpr = 0;
  if (gprp_any_available(&la->gprp, &gpr)) {
    return gpr;
  } else {
    // there weren't any old gpr allocations, and there
    // were no gpr's available. so we are forced to spill
    // an existing gpr.
    // #note: we choose %r15 because it is the last
    // register our algorithm would normally allocation to.
    // we could rand() this. or choose another heuristic.
    // the allocation with the longest lifetime is
    // the heuristic the linear register allocator uses.
    codegen_release_gpr(la, X64GPR_R15, buffer);
    return X64GPR_R15;
  }
}

static X64ActiveAllocation codegen_allocate_gpr(X64Allocator *restrict la,
                                                X64GPR gpr, u16 ssa,
                                                String *restrict buffer) {
  codegen_release_gpr(la, gpr, buffer);

  return x64active_allocations_add(
      &la->active, ssa, *li_at(&la->lifetimes, ssa), x64allocation_reg(gpr));
}

static X64Allocation
codegen_create_temporary_from_immediate(X64Allocator *restrict la, u16 Idx,
                                        i64 imm, X64GPR avoid,
                                        String *restrict buffer) {
  // attempt to release an old active allocation in a gpr.
  // this could release nothing, a gpr, or an old stack
  // allocation.
  x64allocator_expire_old_lifetimes(la, Idx);

  X64GPR gpr = 0;
  if (gprp_any_available_other_than(&la->gprp, &gpr, avoid)) {
    string_append(buffer, SV("\tmov "));
    codegen_immediate_operand(imm, buffer);
    string_append(buffer, SV(", %"));
    string_append(buffer, gpr_to_sv(gpr));
    string_append(buffer, SV("\n"));

    return x64allocation_reg(gpr);
  } else {
    u16 offset = x64allocator_bump_active_stack_size(la);
    string_append(buffer, SV("\tmov "));
    codegen_immediate_operand(imm, buffer);
    string_append(buffer, SV(", -"));
    string_append_u64(buffer, offset);
    string_append(buffer, SV("(%rbp)\n"));

    return x64allocation_stack(offset);
  }
}

static X64Allocation
codegen_create_temporary_from_constant(X64Allocator *restrict la, u16 Idx,
                                       Value *restrict constant, X64GPR avoid,
                                       String *restrict buffer) {
  switch (constant->kind) {
  case VALUEKIND_UNINITIALIZED:
    PANIC("uninitialized value");
    break;

  case VALUEKIND_NIL:
    return codegen_create_temporary_from_immediate(la, Idx, (i64)0, avoid,
                                                   buffer);

  case VALUEKIND_BOOLEAN:
    return codegen_create_temporary_from_immediate(
        la, Idx, (i64)constant->boolean, avoid, buffer);

  case VALUEKIND_I64:
    return codegen_create_temporary_from_immediate(la, Idx, constant->integer,
                                                   avoid, buffer);

  default:
    unreachable();
  }
}

/**
 * @brief Makes sure the value of <aa> is held in <gpr> associated with the
 * X64ActiveAllocation for the given <ssa> local. keeping <aa> alive if it's
 * last use is after the current instruction <Idx>
 *
 * @note i don't know what to call this, i just know i don't like the name.
 *
 * @param la the X64Allocator
 * @param Idx the index of the current Instruction
 * @param aa the X64ActiveAllocation to copy into gpr
 * @param gpr the target register
 * @param buffer
 */
static X64ActiveAllocation
codegen_allocate_gpr_from_existing(X64Allocator *restrict la, u16 Idx,
                                   X64ActiveAllocation *restrict aa, X64GPR gpr,
                                   u16 ssa, String *restrict buffer) {
  if ((aa->allocation.kind == ALLOC_GPR) && (aa->allocation.gpr == gpr)) {
    if (aa->lifetime.last_use > Idx) {
      // we have to keep <aa> alive so we have to move it out of this register
      codegen_reallocate_gpr(la, aa, buffer);
      // however we know that the correct value is still in <gpr>,
      // so we can just start ssa's ActiveLifetime associated with <gpr>
      // as if we had allocated it there
      return x64active_allocations_add(&la->active, ssa,
                                       *li_at(&la->lifetimes, ssa),
                                       x64allocation_reg(gpr));
    }
    // else we can remove the old lifetime
    x64active_allocations_erase(&la->active, aa);
    // and add the new one
    return x64active_allocations_add(
        &la->active, ssa, *li_at(&la->lifetimes, ssa), x64allocation_reg(gpr));
  }
  // else (aa->allocation.kind != ALLOC_GPR) || (aa->allocation.gpr != gpr)
  // move the value of <aa> to the target register
  string_append(buffer, SV("\tmov "));
  codegen_alloc_operand(aa, buffer);
  string_append(buffer, SV(", %"));
  string_append(buffer, gpr_to_sv(gpr));
  string_append(buffer, SV("\n"));

  // if <aa> is old, we can remove it from active
  if (aa->lifetime.last_use <= Idx) {
    x64active_allocations_erase(&la->active, aa);
  }
  // start the lifetime of ssa associated with gpr
  return x64active_allocations_add(
      &la->active, ssa, *li_at(&la->lifetimes, ssa), x64allocation_reg(gpr));
}

static X64ActiveAllocation
codegen_allocate_gpr_from_immediate(X64Allocator *restrict la, i64 imm,
                                    X64GPR gpr, u16 ssa,
                                    String *restrict buffer) {
  codegen_release_gpr(la, gpr, buffer);
  // initialize gpr with imm
  string_append(buffer, SV("\tmov "));
  codegen_immediate_operand(imm, buffer);
  string_append(buffer, SV(", %"));
  string_append(buffer, gpr_to_sv(gpr));
  string_append(buffer, SV("\n"));

  return x64active_allocations_add(
      &la->active, ssa, *li_at(&la->lifetimes, ssa), x64allocation_reg(gpr));
}

static X64ActiveAllocation
codegen_allocate_mem_from_immediate(X64Allocator *restrict la, i64 imm, u16 ssa,
                                    String *restrict buffer) {
  u16 offset = x64allocator_bump_active_stack_size(la);

  // initialize mem with imm
  string_append(buffer, SV("\tmov "));
  codegen_immediate_operand(imm, buffer);
  string_append(buffer, SV(", -"));
  string_append_u64(buffer, offset);
  string_append(buffer, SV("(%rbp)\n"));

  return x64active_allocations_add(&la->active, ssa,
                                   *li_at(&la->lifetimes, ssa),
                                   x64allocation_stack(offset));
}

static X64ActiveAllocation
codegen_allocate_gpr_from_constant(X64Allocator *restrict la, Value *constant,
                                   X64GPR gpr, u16 ssa,
                                   String *restrict buffer) {
  switch (constant->kind) {
  case VALUEKIND_UNINITIALIZED: {
    PANIC("uninitialized value");
    break;
  }

  case VALUEKIND_NIL: {
    return codegen_allocate_gpr_from_immediate(la, (i64)0, gpr, ssa, buffer);
  }

  case VALUEKIND_BOOLEAN: {
    return codegen_allocate_gpr_from_immediate(la, (i64)constant->boolean, gpr,
                                               ssa, buffer);
  }

  case VALUEKIND_I64: {
    return codegen_allocate_gpr_from_immediate(la, (i64)constant->integer, gpr,
                                               ssa, buffer);
  }

  default:
    unreachable();
  }
}

static X64ActiveAllocation
codegen_allocate_mem_from_constant(X64Allocator *restrict la, Value *constant,
                                   u16 ssa, String *restrict buffer) {
  switch (constant->kind) {
  case VALUEKIND_UNINITIALIZED: {
    PANIC("uninitialized value");
    break;
  }

  case VALUEKIND_NIL: {
    return codegen_allocate_mem_from_immediate(la, (i64)0, ssa, buffer);
  }

  case VALUEKIND_BOOLEAN: {
    return codegen_allocate_mem_from_immediate(la, (i64)constant->boolean, ssa,
                                               buffer);
  }

  case VALUEKIND_I64: {
    return codegen_allocate_mem_from_immediate(la, (i64)constant->integer, ssa,
                                               buffer);
  }

  default:
    unreachable();
  }
}

/**
 * @brief same as "codegen_allocate_gpr_from_existing"
 * except the choice of gpr is made by the callee.
 *
 * @param la
 * @param Idx
 * @param aa
 * @param ssa
 * @param buffer
 * @return X64ActiveAllocation*
 */
static X64ActiveAllocation
codegen_allocate_any_gpr_from_existing(X64Allocator *restrict la, u16 Idx,
                                       X64ActiveAllocation *restrict aa,
                                       u16 ssa, String *restrict buffer) {
  X64GPR gpr;
  if (gprp_any_available(&la->gprp, &gpr)) {
    // an available gpr is 'any' gpr
    return codegen_allocate_gpr_from_existing(la, Idx, aa, gpr, ssa, buffer);
  } else {
    // if the existing X64ActiveAllocation is old, we can reuse it's gpr
    if ((aa->lifetime.last_use <= Idx) && (aa->allocation.kind == ALLOC_GPR)) {
      return codegen_allocate_gpr_from_existing(la, Idx, aa, aa->allocation.gpr,
                                                ssa, buffer);
    } else {
      gpr = codegen_release_any_gpr(la, Idx, buffer);
      return codegen_allocate_gpr_from_existing(la, Idx, aa, gpr, ssa, buffer);
    }
  }
}

/**
 * @brief same as "codegen_allocate_gpr_from_existing"
 * except that the new allocation can end up in memory.
 *
 * @param la
 * @param Idx
 * @param aa
 * @param ssa
 * @param buffer
 * @return X64ActiveAllocation*
 */
static X64ActiveAllocation codegen_allocate_any_gpr_or_mem_from_existing(
    X64Allocator *restrict la, u16 Idx, X64ActiveAllocation *restrict aa,
    u16 ssa, String *restrict buffer) {
  if (aa->allocation.kind == ALLOC_GPR) {
    return codegen_allocate_any_gpr_from_existing(la, Idx, aa, ssa, buffer);
  }

  u16 offset = aa->allocation.offset;

  if (aa->lifetime.last_use <= Idx) {
    // we can allocate on top of the existing value
    x64active_allocations_erase(&la->active, aa);
  } else {
    // we have to use a new stack slot.
    offset = x64allocator_bump_active_stack_size(la);
  }

  // allocation the ssa
  return x64active_allocations_add(&la->active, ssa,
                                   *li_at(&la->lifetimes, ssa),
                                   x64allocation_stack(offset));
}

static X64ActiveAllocation codegen_allocate_any_gpr_or_mem_from_immediate(
    X64Allocator *restrict la, i64 imm, u16 ssa, String *restrict buffer) {
  X64GPR gpr;
  if (gprp_any_available(&la->gprp, &gpr)) {
    return codegen_allocate_gpr_from_immediate(la, imm, gpr, ssa, buffer);
  } else {
    return codegen_allocate_mem_from_immediate(la, imm, ssa, buffer);
  }
}

static X64ActiveAllocation
codegen_allocate_any_gpr_or_mem_from_constant(X64Allocator *restrict la,
                                              Value *constant, u16 ssa,
                                              String *restrict buffer) {
  X64GPR gpr;
  if (gprp_any_available(&la->gprp, &gpr)) {
    return codegen_allocate_gpr_from_constant(la, constant, gpr, ssa, buffer);
  } else {
    return codegen_allocate_mem_from_constant(la, constant, ssa, buffer);
  }
}

static void codegen_function_epilouge(String *restrict buffer) {
  string_append(buffer, SV("\tmov %rbp, %rsp\n"));
  string_append(buffer, SV("\tpop %rbp\n"));
}

// in order to map the one operand IR instruction
//   ret B
// where:
// B -- L[R] = B,    <return>
// B -- L[R] = C[B], <return>
// B -- L[R] = L[B], <return>
//
// to the zero operand ret x64 instruction
//   ret
//
// we need to ensure that the operand B is already
// in the return register. and we need to emit the
// function epilouge
static void codegen_ret(Context *restrict c, X64Allocator *restrict la,
                        Instruction I, String *restrict buffer) {
  // - find out where operand B is
  u16 B = I.B;
  // - make sure B is stored in %rax
  switch (I.B_format) {
  case OPRFMT_SSA: {
    X64ActiveAllocation *Balloc = x64allocator_allocation_of(la, B);
    if ((Balloc->allocation.kind == ALLOC_GPR) &&
        (Balloc->allocation.gpr == X64GPR_RAX)) {
      break;
    }

    // "mov <Balloc>, %rax"
    string_append(buffer, SV("\tmov "));
    codegen_alloc_operand(Balloc, buffer);
    string_append(buffer, SV(", %rax\n"));
    break;
  }

  case OPRFMT_CONSTANT: {
    // "mov $<imm>, %rax"
    string_append(buffer, SV("\tmov "));
    codegen_constant_operand(context_constants_at(c, B), buffer);
    string_append(buffer, SV(", %rax\n"));
    break;
  }

  case OPRFMT_IMMEDIATE: {
    // "mov $<imm>, %rax"
    string_append(buffer, SV("\tmov "));
    codegen_immediate_operand((i64)B, buffer);
    string_append(buffer, SV(", %rax\n"));
    break;
  }

  default:
    unreachable();
  }

  codegen_function_epilouge(buffer);
  string_append(buffer, SV("\tret\n"));
}

// in order to map the 2 operand move IR instruction
//   move A, B
// where:
//   AB -- L[A] = B
//   AB -- L[A] = C[B]
//   AB -- L[A] = L[B]
//
// to the 2 operand mov x64 instruction
//  mov B, A
// where:
//   AB -- R[A] = B
//   AB -- R[A] = R[B]
//   AB -- R[A] = M[B]
//
// we just have to allocate A and move.
static void codegen_mov(Context *restrict c, X64Allocator *restrict la,
                        Instruction I, u16 Idx, String *restrict buffer) {

  u16 A                      = I.A;
  u16 B                      = I.B;
  X64ActiveAllocation Aalloc = x64allocator_allocate(la, Idx, A);
  // - "mov <B>, <A>"
  switch (I.B_format) {
  // "mov <Balloc>, <A>"
  case OPRFMT_SSA: {
    X64ActiveAllocation Balloc = *x64allocator_allocation_of(la, B);

    // "mov -<B-offset>(%rbp), %<tmp-gpr>"
    // "mov %<tmp-gpr>, -<A-offset>(%rbp)"
    if ((Aalloc.allocation.kind == ALLOC_STACK) &&
        (Balloc.allocation.kind == ALLOC_STACK)) {
      X64GPR gpr = codegen_release_any_gpr(la, Idx, buffer);

      string_append(buffer, SV("\tmov -"));
      string_append_u64(buffer, Balloc.allocation.offset);
      string_append(buffer, SV("(%rbp), %"));
      string_append(buffer, gpr_to_sv(gpr));
      string_append(buffer, SV("\n"));

      string_append(buffer, SV("\tmov %"));
      string_append(buffer, gpr_to_sv(gpr));
      string_append(buffer, SV(", -"));
      string_append_u64(buffer, Aalloc.allocation.offset);
      string_append(buffer, SV("(%rbp)\n"));
      return;
    }

    // "mov <Balloc>, <Aalloc>"
    string_append(buffer, SV("\tmov "));
    codegen_alloc_operand(&Balloc, buffer);
    string_append(buffer, SV(", "));
    codegen_alloc_operand(&Aalloc, buffer);
    string_append(buffer, SV("\n"));
    return;
  } // !case OPRFMT_SSA:

  // "mov $<B-imm>, <A>"
  case OPRFMT_CONSTANT: {
    string_append(buffer, SV("\tmov "));
    codegen_constant_operand(context_constants_at(c, B), buffer);
    string_append(buffer, SV(", "));
    codegen_alloc_operand(&Aalloc, buffer);
    string_append(buffer, SV("\n"));
    break;
  } // case OPRFMT_CONSTANT:

  // "mov $<B-imm>, <A>"
  case OPRFMT_IMMEDIATE: {
    string_append(buffer, SV("\tmov "));
    codegen_immediate_operand((i64)B, buffer);
    string_append(buffer, SV(", "));
    codegen_alloc_operand(&Aalloc, buffer);
    string_append(buffer, SV("\n"));
    break;
  }

  default:
    unreachable();
  }
}

// in order to map the 2 operand move IR instruction
//   move A, B
// where:
//   AB -- L[A] = B
//   AB -- L[A] = C[B]
//   AB -- L[A] = L[B]
//
// to the 1 operand neg x64 instruction
//  neg A
// where:
//   AB -- R[A] = -A
//   AB -- R[A] = -R[A]
//   AB -- R[A] = -M[A]
//
// we have to allocate A with the contents of B
// and make sure B isn't overwritten if it is
// used later.
static void codegen_neg(X64Allocator *restrict la, Instruction I, u16 Idx,
                        String *restrict buffer) {
  u16 A = I.A;
  u16 B = I.B;

  // assert that we don't generate trivially foldable instructions.
  assert(I.B_format == OPRFMT_SSA);

  // first, make sure that the operand A has the right value
  // and that we don't overwrite B if it is still used after
  // this instruction.
  X64ActiveAllocation Balloc = *x64allocator_allocation_of(la, B);
  X64ActiveAllocation Aalloc = codegen_allocate_any_gpr_or_mem_from_existing(
      la, Idx, &Balloc, A, buffer);

  // "neg <Aalloc>"
  string_append(buffer, SV("\tneg "));
  codegen_alloc_operand(&Aalloc, buffer);
  string_append(buffer, SV("\n"));
}

// in order to map the 3 operand IR instruction
//   add A, B, C
// where:
//   ABC -- L[A] = L[B] + L[C]
//   ABC -- L[A] = L[B] + C[C]
//   ABC -- L[A] = L[B] + C
//   ABC -- L[A] = C[B] + L[C]
//   ABC -- L[A] = C[B] + C[C]
//   ABC -- L[A] = C[B] + C
//   ABC -- L[A] = B    + L[C]
//   ABC -- L[A] = B    + C[C]
//   ABC -- L[A] = B    + C
//
// to the two operand x64 instruction
//   add A, B
// where:
//   AB -- R[A] = R[A] + R[B]
//
// we have to ensure that the previous value of
// A isn't overwritten if it is needed after this
// expression.
static void codegen_add(Context *restrict c, X64Allocator *restrict la,
                        Instruction I, u16 Idx, String *restrict buffer) {
  u16 A = I.A;
  u16 B = I.B;
  u16 C = I.C;

  X64ActiveAllocation Aalloc = {};
  switch (I.B_format) {
  case OPRFMT_SSA: {
    // make sure the an operand is present in the destination register.
    // and create the allocation of A at the destrination register.
    X64ActiveAllocation Balloc = *x64allocator_allocation_of(la, B);
    switch (I.C_format) {
      // B and C are SSA
    case OPRFMT_SSA: {
      X64ActiveAllocation Calloc = *x64allocator_allocation_of(la, C);

      if (Balloc.allocation.kind == ALLOC_GPR) {
        // since we know B is in a register, we can use
        // that register as the destination operand for the
        // addition. we can use C's location as the source
        // operand if it is either reg or mem.
        Aalloc = codegen_allocate_gpr_from_existing(
            la, Idx, &Balloc, Balloc.allocation.gpr, A, buffer);
        // "add <Calloc>, <Aalloc>"
        string_append(buffer, SV("\tadd "));
        codegen_alloc_operand(&Calloc, buffer);
        string_append(buffer, SV(", "));
        codegen_alloc_operand(&Aalloc, buffer);
        string_append(buffer, SV("\n"));
      } else if (Calloc.allocation.kind == ALLOC_GPR) {
        // since we know C is in a register, we can use
        // that register as the destination operand for
        // the addition. we can use B as the source operand
        // from it's memory location.
        Aalloc = codegen_allocate_gpr_from_existing(
            la, Idx, &Calloc, Calloc.allocation.gpr, A, buffer);
        // "add <Balloc>, <Aalloc>"
        string_append(buffer, SV("\tadd "));
        codegen_alloc_operand(&Balloc, buffer);
        string_append(buffer, SV(", "));
        codegen_alloc_operand(&Aalloc, buffer);
        string_append(buffer, SV("\n"));
      } else {
        // B and C are both memory operands, so we have to move
        // one to a register before we can add. we arbitrarily
        // choose B. (since addition is associative is doesn't
        // matter which one we select. could rand() it) and
        // we arbitrarily choose to move the destination operand
        // to a register. we could rand() that too.
        // and we choose the first available register, or
        // spill an existing allocation if none are available.
        X64GPR gpr = codegen_release_any_gpr(la, Idx, buffer);
        Aalloc = codegen_allocate_gpr_from_existing(la, Idx, &Balloc, gpr, A,
                                                    buffer);
        // since we know Aalloc now holds B and is in the destination:
        // "add <Aalloc>, <Calloc>"
        string_append(buffer, SV("\tadd "));
        codegen_alloc_operand(&Calloc, buffer);
        string_append(buffer, SV(", "));
        codegen_alloc_operand(&Aalloc, buffer);
        string_append(buffer, SV("\n"));
      }
      break;
    } // !case OPRFMT_SSA:

    // B is SSA, C is a constant
    case OPRFMT_CONSTANT: {
      Value *Cconst = context_constants_at(c, C);
      // we can just embed C directly into the instruction.
      // first we have to start A's lifetime. using the value of B
      Aalloc =
          codegen_allocate_any_gpr_from_existing(la, Idx, &Balloc, A, buffer);
      // "add <Aalloc>, <Cimm>"
      string_append(buffer, SV("\tadd "));
      codegen_constant_operand(Cconst, buffer);
      string_append(buffer, SV(", "));
      codegen_alloc_operand(&Aalloc, buffer);
      string_append(buffer, SV("\n"));
      break;
    }

    // B is SSA, C is immediate
    case OPRFMT_IMMEDIATE: {
      // we can just embed C directly into the instruction.
      // first we have to start A's lifetime. using the value of B
      Aalloc =
          codegen_allocate_any_gpr_from_existing(la, Idx, &Balloc, A, buffer);
      // "add <Aalloc>, <Cimm>"
      string_append(buffer, SV("\tadd "));
      codegen_immediate_operand((i64)C, buffer);
      string_append(buffer, SV(", "));
      codegen_alloc_operand(&Aalloc, buffer);
      string_append(buffer, SV("\n"));
      break;
    }

    default:
      unreachable();
    }

    break;
  }

  // B is constant
  case OPRFMT_CONSTANT: {
    // assert that we don't generate trivially foldable instructions.
    assert(I.C_format == OPRFMT_SSA);
    X64ActiveAllocation Calloc = *x64allocator_allocation_of(la, C);
    // we can just embed B directly into the instruction.
    // first we have to start A's lifetime. using the value of C
    Aalloc =
        codegen_allocate_any_gpr_from_existing(la, Idx, &Calloc, A, buffer);
    string_append(buffer, SV("\tadd "));
    codegen_constant_operand(context_constants_at(c, B), buffer);
    string_append(buffer, SV(", "));
    codegen_alloc_operand(&Aalloc, buffer);
    string_append(buffer, SV("\n"));
    break;
  }

  // B is immediate
  case OPRFMT_IMMEDIATE: {
    // assert that we don't generate trivially foldable instructions.
    assert(I.C_format == OPRFMT_SSA);
    X64ActiveAllocation Calloc = *x64allocator_allocation_of(la, C);
    // we can just embed B directly into the instruction.
    // first we have to start A's lifetime. using the value of C
    Aalloc =
        codegen_allocate_any_gpr_from_existing(la, Idx, &Calloc, A, buffer);
    string_append(buffer, SV("\tadd "));
    codegen_immediate_operand((i64)B, buffer);
    string_append(buffer, SV(", "));
    codegen_alloc_operand(&Aalloc, buffer);
    string_append(buffer, SV("\n"));
    break;
  }

  default:
    unreachable();
  }
}

// in order to map the 3 operand IR instruction
//   sub A, B, C
// where:
//   ABC -- L[A] = L[B] - L[C]
//   ABC -- L[A] = L[B] - C[C]
//   ABC -- L[A] = L[B] - C
//   ABC -- L[A] = C[B] - L[C]
//   ABC -- L[A] = C[B] - C[C]
//   ABC -- L[A] = C[B] - C
//   ABC -- L[A] = B    - L[C]
//   ABC -- L[A] = B    - C[C]
//   ABC -- L[A] = B    - C
//
// to the two operand x64 instruction
//   sub A, B
// where:
//   AB -- R[A] = R[A] - R[B]
//
// we have to ensure that the previous value of
// A isn't overwritten if it is needed after this
// expression.
static void codegen_sub(Context *restrict c, X64Allocator *restrict la,
                        Instruction I, u16 Idx, String *restrict buffer) {
  u16 A = I.A;
  u16 B = I.B;
  u16 C = I.C;

  X64ActiveAllocation Aalloc = {};
  switch (I.B_format) {
  case OPRFMT_SSA: {
    X64ActiveAllocation Balloc = *x64allocator_allocation_of(la, B);
    switch (I.C_format) {
    case OPRFMT_SSA: {
      X64ActiveAllocation Calloc = *x64allocator_allocation_of(la, C);

      if ((Balloc.allocation.kind == ALLOC_GPR) ||
          (Calloc.allocation.kind == ALLOC_GPR)) {
        // there is a valid opcode for "sub <gpr>, <mem/gpr>"
        // since subtraction is non-associative we always
        // order the operands as "sub <C>, <B>". and
        // we also always allocate the result <A> from <B>
        Aalloc = codegen_allocate_gpr_from_existing(
            la, Idx, &Balloc, Balloc.allocation.gpr, A, buffer);
        // "sub <Calloc>, <Aalloc>"
        string_append(buffer, SV("\tsub "));
        codegen_alloc_operand(&Calloc, buffer);
        string_append(buffer, SV(", "));
        codegen_alloc_operand(&Aalloc, buffer);
        string_append(buffer, SV("\n"));
      } else {
        // B and C are both memory operands, so we have to move
        // one to a register before we can sub.
        X64GPR gpr = codegen_release_any_gpr(la, Idx, buffer);
        Aalloc = codegen_allocate_gpr_from_existing(la, Idx, &Balloc, gpr, A,
                                                    buffer);
        // since we know Aalloc now holds B and is in the destination:
        // "sub <Calloc>, <Aalloc>"
        string_append(buffer, SV("\tsub "));
        codegen_alloc_operand(&Calloc, buffer);
        string_append(buffer, SV(", "));
        codegen_alloc_operand(&Aalloc, buffer);
        string_append(buffer, SV("\n"));
      }
      break;
    }

    case OPRFMT_CONSTANT: {
      Value *Cconst = context_constants_at(c, C);

      Aalloc =
          codegen_allocate_any_gpr_from_existing(la, Idx, &Balloc, A, buffer);
      string_append(buffer, SV("\tsub "));
      codegen_constant_operand(Cconst, buffer);
      string_append(buffer, SV(", "));
      codegen_alloc_operand(&Aalloc, buffer);
      string_append(buffer, SV("\n"));
      break;
    }

    case OPRFMT_IMMEDIATE: {
      Aalloc =
          codegen_allocate_any_gpr_from_existing(la, Idx, &Balloc, A, buffer);
      string_append(buffer, SV("\tsub "));
      codegen_immediate_operand((u16)C, buffer);
      string_append(buffer, SV(", "));
      codegen_alloc_operand(&Aalloc, buffer);
      string_append(buffer, SV("\n"));
      break;
    }

    default:
      unreachable();
    }
    break;
  }

  // #note subtraction is not associative, so it matters
  // in which order we perform it. So we have to implement
  // B - C to maintain the semantics of the source code.
  // additionally, there is no x64 instruction to
  // subtract a register from an immediate.
  // this means we have to allocate B into a register
  // before we can subtract C from it. this goes along with
  // the usual, we want to start the lifetime of A using that
  // register or stack slot as it's allocation.
  case OPRFMT_CONSTANT: {
    assert(I.C_format == OPRFMT_SSA);
    X64ActiveAllocation Calloc = *x64allocator_allocation_of(la, C);
    Value *Bconst              = context_constants_at(c, B);
    X64ActiveAllocation Aalloc =
        codegen_allocate_any_gpr_or_mem_from_constant(la, Bconst, A, buffer);

    string_append(buffer, SV("\tsub "));
    codegen_alloc_operand(&Calloc, buffer);
    string_append(buffer, SV(", "));
    codegen_alloc_operand(&Aalloc, buffer);
    string_append(buffer, SV("\n"));
    break;
  }

  case OPRFMT_IMMEDIATE: {
    assert(I.C_format == OPRFMT_SSA);
    X64ActiveAllocation Calloc = *x64allocator_allocation_of(la, C);
    X64ActiveAllocation Aalloc =
        codegen_allocate_any_gpr_or_mem_from_immediate(la, (i64)B, A, buffer);

    string_append(buffer, SV("\tsub "));
    codegen_alloc_operand(&Calloc, buffer);
    string_append(buffer, SV(", "));
    codegen_alloc_operand(&Aalloc, buffer);
    string_append(buffer, SV("\n"));
    break;
  }

  default:
    unreachable();
  }
}

// in order to map the 3 operand IR instruction
//   mul A, B, C
// where:
//   ABC -- L[A] = L[B] * L[C]
//   ABC -- L[A] = L[B] * C[C]
//   ABC -- L[A] = L[B] * C
//   ABC -- L[A] = C[B] * L[C]
//   ABC -- L[A] = C[B] * C[C]
//   ABC -- L[A] = C[B] * C
//   ABC -- L[A] = B    * L[C]
//   ABC -- L[A] = B    * C[C]
//   ABC -- L[A] = B    * C
//
// to the 1 operand x64 instruction
//   mul B
// where:
//   B -- RDX:RAX = RAX + R[B]
//
// we have to ensure that the previous value of
// RAX and RDX isn't overwritten if either is needed
// after this expression. and we have to ensure
// that at least one of the operands can be
// allocated to RAX before the instruction.
// we must also note that the result, A,
// is allocated to RAX afterwords.
static void codegen_mul(Context *restrict c, X64Allocator *restrict la,
                        Instruction I, u16 Idx, String *restrict buffer) {
  u16 A                                       = I.A;
  u16 B                                       = I.B;
  u16 C                                       = I.C;
  [[maybe_unused]] X64ActiveAllocation Aalloc = {};
  switch (I.B_format) {
  case OPRFMT_SSA: {
    X64ActiveAllocation Balloc = *x64allocator_allocation_of(la, B);
    // make sure the value of B is held in %rax, for the imul instruction.
    Aalloc = codegen_allocate_gpr_from_existing(la, Idx, &Balloc, X64GPR_RAX, A,
                                                buffer);
    // make sure we are not overwriting an existing X64ActiveAllocation in %rdx
    // when we evaluate the imul instruction.
    codegen_release_gpr(la, X64GPR_RDX, buffer);
    switch (I.C_format) {
    case OPRFMT_SSA: {
      X64ActiveAllocation Calloc = *x64allocator_allocation_of(la, C);
      // "imul <Calloc>"
      string_append(buffer, SV("\timul "));
      codegen_alloc_operand(&Calloc, buffer);
      string_append(buffer, SV("\n"));
      break;
    }

    case OPRFMT_CONSTANT: {
      // so, we can't have an immediate as the operand to imul
      // or idiv. however, we do know for sure that %rdx is free,
      // so we can use it to store the temporary.
      Value *Cconst = context_constants_at(c, C);
      string_append(buffer, SV("\tmov "));
      codegen_constant_operand(Cconst, buffer);
      string_append(buffer, SV(", %"));
      string_append(buffer, gpr_to_sv(X64GPR_RDX));
      string_append(buffer, SV("\n"));

      string_append(buffer, SV("\timul %"));
      string_append(buffer, gpr_to_sv(X64GPR_RDX));
      string_append(buffer, SV("\n"));
      break;
    }

    case OPRFMT_IMMEDIATE: {
      string_append(buffer, SV("\tmov "));
      codegen_immediate_operand((i64)C, buffer);
      string_append(buffer, SV(", %"));
      string_append(buffer, gpr_to_sv(X64GPR_RDX));
      string_append(buffer, SV("\n"));

      string_append(buffer, SV("\timul %"));
      string_append(buffer, gpr_to_sv(X64GPR_RDX));
      string_append(buffer, SV("\n"));
      break;
    }

    default:
      unreachable();
    }
    break;
  }

  case OPRFMT_CONSTANT: {
    assert(I.C_format == OPRFMT_SSA);
    X64ActiveAllocation Calloc = *x64allocator_allocation_of(la, C);
    Value *Bconst              = context_constants_at(c, B);
    // initialize A's X64ActiveAllocation with the value of B at %rax
    Aalloc =
        codegen_allocate_gpr_from_constant(la, Bconst, X64GPR_RAX, A, buffer);
    // make sure we are not overwriting an existing X64ActiveAllocation in %rdx
    // when we evaluate the imul instruction.
    codegen_release_gpr(la, X64GPR_RDX, buffer);
    string_append(buffer, SV("\timul "));
    codegen_alloc_operand(&Calloc, buffer);
    string_append(buffer, SV("\n"));
    break;
  }

  case OPRFMT_IMMEDIATE: {
    assert(I.C_format == OPRFMT_SSA);
    X64ActiveAllocation Calloc = *x64allocator_allocation_of(la, C);
    Aalloc =
        codegen_allocate_gpr_from_immediate(la, (i64)B, X64GPR_RAX, A, buffer);
    // make sure we are not overwriting an existing X64ActiveAllocation in %rdx
    // when we evaluate the imul instruction.
    codegen_release_gpr(la, X64GPR_RDX, buffer);
    string_append(buffer, SV("\timul "));
    codegen_alloc_operand(&Calloc, buffer);
    string_append(buffer, SV("\n"));
    break;
  }

  default:
    unreachable();
  }
}

// in order to map the 3 operand IR instruction
//   div A, B, C
// where:
//   ABC -- L[A] = L[B] / L[C]
//   ABC -- L[A] = L[B] / C[C]
//   ABC -- L[A] = L[B] / C
//   ABC -- L[A] = C[B] / L[C]
//   ABC -- L[A] = C[B] / C[C]
//   ABC -- L[A] = C[B] / C
//   ABC -- L[A] = B    / L[C]
//   ABC -- L[A] = B    / C[C]
//   ABC -- L[A] = B    / C
//
// to the 1 operand x64 instruction
//   div B
// where:
//   B -- RAX = RAX / R[B], RDX = RAX % R[B]
//
// we have to ensure that the previous value of
// RAX and RDX isn't overwritten if either is needed
// after this expression. And we have to ensure
// that the dividend B's value is held in %rax
// before the instruction.
// we must also note that the result, A, is
// allocated to RAX afterwords.
static void codegen_div(Context *restrict c, X64Allocator *restrict la,
                        Instruction I, u16 Idx, String *restrict buffer) {
  u16 A                                       = I.A;
  u16 B                                       = I.B;
  u16 C                                       = I.C;
  [[maybe_unused]] X64ActiveAllocation Aalloc = {};
  switch (I.B_format) {
  case OPRFMT_SSA: {
    X64ActiveAllocation Balloc = *x64allocator_allocation_of(la, B);
    // make sure the value of B is held in %rax, for the idiv instruction.
    Aalloc = codegen_allocate_gpr_from_existing(la, Idx, &Balloc, X64GPR_RAX, A,
                                                buffer);
    // make sure we are not overwriting an existing X64ActiveAllocation in %rdx
    // when we evaluate the idiv instruction.
    codegen_release_gpr(la, X64GPR_RDX, buffer);
    codegen_zero_gpr(X64GPR_RDX, buffer);
    switch (I.C_format) {
    case OPRFMT_SSA: {
      X64ActiveAllocation Calloc = *x64allocator_allocation_of(la, C);

      string_append(buffer, SV("\tidiv "));
      codegen_alloc_operand(&Calloc, buffer);
      string_append(buffer, SV("\n"));
      break;
    }

    case OPRFMT_CONSTANT: {
      // so, we can't have an immediate as the operand to imul
      // or idiv. and %rdx is used as the upper bits of a the
      // dividend. so we have to use another temporary.
      Value *Cconst     = context_constants_at(c, C);
      X64Allocation tmp = codegen_create_temporary_from_constant(
          la, Idx, Cconst, X64GPR_RDX, buffer);

      string_append(buffer, SV("\tidiv "));
      if (tmp.kind == ALLOC_GPR) {
        string_append(buffer, SV("%"));
        string_append(buffer, gpr_to_sv(tmp.gpr));
      } else {
        string_append(buffer, SV("-"));
        string_append_u64(buffer, tmp.offset);
        string_append(buffer, SV("(%rbp)"));
      }
      string_append(buffer, SV("\n"));
      break;
    }

    case OPRFMT_IMMEDIATE: {
      X64Allocation tmp = codegen_create_temporary_from_immediate(
          la, Idx, (i64)C, X64GPR_RDX, buffer);

      string_append(buffer, SV("\tidiv "));
      if (tmp.kind == ALLOC_GPR) {
        string_append(buffer, SV("%"));
        string_append(buffer, gpr_to_sv(tmp.gpr));
      } else {
        string_append(buffer, SV("-"));
        string_append_u64(buffer, tmp.offset);
        string_append(buffer, SV("(%rbp)"));
      }
      string_append(buffer, SV("\n"));
      break;
    }

    default:
      unreachable();
    }
    break;
  }

  case OPRFMT_CONSTANT: {
    assert(I.C_format == OPRFMT_SSA);
    X64ActiveAllocation *Calloc = x64allocator_allocation_of(la, C);
    if ((Calloc->allocation.kind == ALLOC_GPR) &&
        (Calloc->allocation.gpr == X64GPR_RDX)) {
      codegen_reallocate_local_other_than(la, Calloc, X64GPR_RDX, buffer);
    }

    Value *Bconst = context_constants_at(c, B);
    // initialize A's X64ActiveAllocation with the value of B at %rax
    Aalloc =
        codegen_allocate_gpr_from_constant(la, Bconst, X64GPR_RAX, A, buffer);
    // make sure we are not overwriting an existing X64ActiveAllocation in %rdx
    // when we evaluate the idiv instruction. and that rdx holds no erraneous
    // data
    codegen_release_gpr(la, X64GPR_RDX, buffer);
    codegen_zero_gpr(X64GPR_RDX, buffer);

    string_append(buffer, SV("\tidiv "));
    codegen_alloc_operand(Calloc, buffer);
    string_append(buffer, SV("\n"));
    break;
  }

  case OPRFMT_IMMEDIATE: {
    assert(I.C_format == OPRFMT_SSA);
    X64ActiveAllocation *Calloc = x64allocator_allocation_of(la, C);
    if ((Calloc->allocation.kind == ALLOC_GPR) &&
        (Calloc->allocation.gpr == X64GPR_RDX)) {
      codegen_reallocate_local_other_than(la, Calloc, X64GPR_RDX, buffer);
    }

    Aalloc =
        codegen_allocate_gpr_from_immediate(la, (i64)B, X64GPR_RAX, A, buffer);
    // make sure we are not overwriting an existing X64ActiveAllocation in %rdx
    // when we evaluate the idiv instruction.
    codegen_release_gpr(la, X64GPR_RDX, buffer);
    codegen_zero_gpr(X64GPR_RDX, buffer);

    string_append(buffer, SV("\tidiv "));
    codegen_alloc_operand(Calloc, buffer);
    string_append(buffer, SV("\n"));
    break;
  }

  default:
    unreachable();
  }
}

// in order to map the 3 operand IR instruction
//   mod A, B, C
// where:
//   ABC -- L[A] = L[B] % L[C]
//   ABC -- L[A] = L[B] % C[C]
//   ABC -- L[A] = L[B] % C
//   ABC -- L[A] = C[B] % L[C]
//   ABC -- L[A] = C[B] % C[C]
//   ABC -- L[A] = C[B] % C
//   ABC -- L[A] = B    % L[C]
//   ABC -- L[A] = B    % C[C]
//   ABC -- L[A] = B    % C
//
// to the 1 operand x64 instruction
//   div B
// where:
//   B -- RAX = RAX / R[B], RDX = RAX % R[B]
//
// we have to ensure that the previous value of
// RAX and RDX isn't overwritten if either is needed
// after this expression. And we have to ensure
// that exactly the dividend, operand B, can be
// allocated to RAX before the instruction.
// we must also note that the result, A, is
// allocated to RDX afterwords.
static void codegen_mod(Context *restrict c, X64Allocator *restrict la,
                        Instruction I, u16 Idx, String *restrict buffer) {
  u16 A                                       = I.A;
  u16 B                                       = I.B;
  u16 C                                       = I.C;
  [[maybe_unused]] X64ActiveAllocation Aalloc = {};
  switch (I.B_format) {
  case OPRFMT_SSA: {
    // we need to ensure that B is held in %rax.
    // but unlike nearly every other binop we don't
    // allocate A where B is going, we instead want
    // A to be allocated in %rdx. C needs to go in
    // a register or in memory as it cannot be an immediate.
    X64ActiveAllocation Balloc = *x64allocator_allocation_of(la, B);
    if ((Balloc.allocation.kind != ALLOC_GPR) ||
        (Balloc.allocation.gpr != X64GPR_RAX)) {
      string_append(buffer, SV("\tmov "));
      codegen_alloc_operand(&Balloc, buffer);
      string_append(buffer, SV(", %"));
      string_append(buffer, gpr_to_sv(X64GPR_RAX));
      string_append(buffer, SV("\n"));

      if (Balloc.lifetime.last_use <= Idx) {
        x64active_allocations_erase(&la->active, &Balloc);
      }
    }

    codegen_release_gpr(la, X64GPR_RDX, buffer);
    codegen_zero_gpr(X64GPR_RDX, buffer);
    Aalloc = codegen_allocate_gpr(la, X64GPR_RDX, A, buffer);

    switch (I.C_format) {
    case OPRFMT_SSA: {
      X64ActiveAllocation Calloc = *x64allocator_allocation_of(la, C);

      string_append(buffer, SV("\tidiv "));
      codegen_alloc_operand(&Calloc, buffer);
      string_append(buffer, SV("\n"));
      break;
    }

    case OPRFMT_CONSTANT: {
      Value *Cconst     = context_constants_at(c, C);
      X64Allocation tmp = codegen_create_temporary_from_constant(
          la, Idx, Cconst, X64GPR_RAX, buffer);

      string_append(buffer, SV("\tidiv "));
      if (tmp.kind == ALLOC_GPR) {
        string_append(buffer, SV("%"));
        string_append(buffer, gpr_to_sv(tmp.gpr));
      } else {
        string_append(buffer, SV("-"));
        string_append_u64(buffer, tmp.offset);
        string_append(buffer, SV("(%rbp)"));
      }
      string_append(buffer, SV("\n"));
      break;
    }

    case OPRFMT_IMMEDIATE: {
      X64Allocation tmp = codegen_create_temporary_from_immediate(
          la, Idx, (i64)C, X64GPR_RAX, buffer);

      string_append(buffer, SV("\tidiv "));
      if (tmp.kind == ALLOC_GPR) {
        string_append(buffer, SV("%"));
        string_append(buffer, gpr_to_sv(tmp.gpr));
      } else {
        string_append(buffer, SV("-"));
        string_append_u64(buffer, tmp.offset);
        string_append(buffer, SV("(%rbp)"));
      }
      string_append(buffer, SV("\n"));
      break;
    }

    default:
      unreachable();
    }
    break;
  }

  case OPRFMT_CONSTANT: {
    assert(I.C_format == OPRFMT_SSA);
    X64ActiveAllocation Calloc = *x64allocator_allocation_of(la, C);
    Value *Bconst              = context_constants_at(c, B);

    codegen_release_gpr(la, X64GPR_RDX, buffer);
    codegen_zero_gpr(X64GPR_RDX, buffer);
    Aalloc = codegen_allocate_gpr(la, X64GPR_RDX, A, buffer);

    string_append(buffer, SV("\tmov "));
    codegen_constant_operand(Bconst, buffer);
    string_append(buffer, SV(", %"));
    string_append(buffer, gpr_to_sv(X64GPR_RAX));
    string_append(buffer, SV("\n"));

    string_append(buffer, SV("\tidiv "));
    codegen_alloc_operand(&Calloc, buffer);
    string_append(buffer, SV("\n"));
    break;
  }

  case OPRFMT_IMMEDIATE: {
    assert(I.C_format == OPRFMT_SSA);
    X64ActiveAllocation Calloc = *x64allocator_allocation_of(la, C);

    codegen_release_gpr(la, X64GPR_RDX, buffer);
    codegen_zero_gpr(X64GPR_RDX, buffer);
    Aalloc = codegen_allocate_gpr(la, X64GPR_RDX, A, buffer);

    string_append(buffer, SV("\tmov "));
    codegen_immediate_operand((i64)B, buffer);
    string_append(buffer, SV(", %"));
    string_append(buffer, gpr_to_sv(X64GPR_RAX));
    string_append(buffer, SV("\n"));

    string_append(buffer, SV("\tidiv "));
    codegen_alloc_operand(&Calloc, buffer);
    string_append(buffer, SV("\n"));
    break;
  }

  default:
    unreachable();
  }
}

static void codegen_bytecode(Context *restrict c, X64Allocator *restrict la,
                             Bytecode *restrict bc, String *restrict buffer) {
  Instruction *ip = bc->buffer;
  for (u16 idx = 0; idx < bc->length; ++idx) {
    Instruction I = ip[idx];
    switch (I.opcode) {

    case OPC_RET: {
      codegen_ret(c, la, I, buffer);
      break;
    }

    case OPC_MOVE: {
      codegen_mov(c, la, I, idx, buffer);
      break;
    }

    case OPC_NEG: {
      codegen_neg(la, I, idx, buffer);
      break;
    }

    case OPC_ADD: {
      codegen_add(c, la, I, idx, buffer);
      break;
    }

    case OPC_SUB: {
      codegen_sub(c, la, I, idx, buffer);
      break;
    }

    case OPC_MUL: {
      codegen_mul(c, la, I, idx, buffer);
      break;
    }

    case OPC_DIV: {
      codegen_div(c, la, I, idx, buffer);
      break;
    }

    case OPC_MOD: {
      codegen_mod(c, la, I, idx, buffer);
      break;
    }

    default:
      unreachable();
    }
  }
}

static void codegen_function_prolouge(X64Allocator *restrict la,
                                      StringView name,
                                      String *restrict buffer) {
  directive_text(buffer);
  directive_globl(name, buffer);
  directive_type(name, STT_FUNC, buffer);
  directive_label(name, buffer);

  string_append(buffer, SV("\tpush %rbp\n"));
  string_append(buffer, SV("\tmov %rsp, %rbp\n"));
  if (la->stack_size != 0) {
    string_append(buffer, SV("\tsub $"));
    string_append_u64(buffer, la->stack_size);
    string_append(buffer, SV(", %rsp\n"));
  }
}

static void codegen_function(Context *restrict c, String *restrict buffer,
                             StringView name, FunctionBody *restrict body) {
  Bytecode *bc       = &body->bc;
  X64Allocator la    = x64allocator_create(body);
  String body_buffer = string_create();

  codegen_bytecode(c, &la, bc, &body_buffer);

  codegen_function_prolouge(&la, name, buffer);

  string_append_string(buffer, &body_buffer);

  directive_size_label_relative(name, buffer);

  x64allocator_destroy(&la);
  string_destroy(&body_buffer);
}

static void codegen_ste(Context *restrict c, String *restrict buffer,
                        SymbolTableElement *restrict ste) {
  StringView name = ste->name;
  switch (ste->kind) {
  case STE_UNDEFINED: {
    PANIC("undefined global symbol");
    break;
  }

  case STE_FUNCTION: {
    FunctionBody *body = &ste->function_body;
    codegen_function(c, buffer, name, body);
    break;
  }

  default:
    unreachable();
  }
}

// static StringView cpu_type = {.length = sizeof("znver3") - 1, .ptr =
// "znver3"};

static StringView version = {.length = sizeof(EXP_VERSION_STRING) - 1,
                             EXP_VERSION_STRING};

static void emit_file_prolouge(Context *restrict context,
                               String *restrict buffer) {
  directive_file(context_source_path(context), buffer);
  // directive_arch(cpu_type, buffer);
  string_append(buffer, SV("\n"));
}

static void emit_file_epilouge(String *restrict buffer) {
  directive_ident(version, buffer);
  directive_noexecstack(buffer);
}

i32 codegen(Context *restrict context) {
  String buffer = string_create();

  emit_file_prolouge(context, &buffer);

  SymbolTableIterator iter = context_global_symbol_iterator(context);
  while (!symbol_table_iterator_done(&iter)) {
    codegen_ste(context, &buffer, iter.element);
    string_append(&buffer, SV("\n"));

    symbol_table_iterator_next(&iter);
  }

  emit_file_epilouge(&buffer);

  StringView path = context_assembly_path(context);
  FILE *file      = file_open(path.ptr, "w");
  file_write(string_to_cstring(&buffer), file);
  file_close(file);

  string_destroy(&buffer);
  return EXIT_SUCCESS;
}