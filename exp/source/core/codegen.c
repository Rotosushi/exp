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

#include "core/codegen.h"
#include "core/directives.h"
#include "utility/alloc.h"
#include "utility/array_growth.h"
#include "utility/config.h"
#include "utility/numeric_conversions.h"
#include "utility/panic.h"

/**
 * @brief General Purpose Register models which GPRs are available
 *
 */
typedef enum X64GPR {
  X64GPR_RAX,
  X64GPR_RBX,
  X64GPR_RCX,
  X64GPR_RDX,
  X64GPR_RBP,
  X64GPR_RSI,
  X64GPR_RDI,
  X64GPR_RSP,
  X64GPR_R8,
  X64GPR_R9,
  X64GPR_R10,
  X64GPR_R11,
  X64GPR_R12,
  X64GPR_R13,
  X64GPR_R14,
  X64GPR_R15,
} X64GPR;

#define SV(s) string_view_from_string(s, sizeof(s) - 1)

static StringView gpr_to_sv(X64GPR r) {
  switch (r) {
  case X64GPR_RAX:
    return SV("rax");
  case X64GPR_RBX:
    return SV("rbx");
  case X64GPR_RCX:
    return SV("rcx");
  case X64GPR_RDX:
    return SV("rdx");
  case X64GPR_RBP:
    return SV("rbp");
  case X64GPR_RSI:
    return SV("rsi");
  case X64GPR_RDI:
    return SV("rdi");
  case X64GPR_RSP:
    return SV("rsp");
  case X64GPR_R8:
    return SV("r8");
  case X64GPR_R9:
    return SV("r9");
  case X64GPR_R10:
    return SV("r10");
  case X64GPR_R11:
    return SV("r11");
  case X64GPR_R12:
    return SV("r12");
  case X64GPR_R13:
    return SV("r13");
  case X64GPR_R14:
    return SV("r14");
  case X64GPR_R15:
    return SV("r15");
  default:
    unreachable();
  }
}

#undef SV

/**
 * @brief General Purpose Register Pool keeps track of which
 * general purpose registers are currently allocated.
 *
 */
typedef struct GPRP {
  u16 bitset;
} GPRP;

static GPRP gprp_create() {
  GPRP gprp = {.bitset = 0};
  return gprp;
}

static void gprp_destroy(GPRP *restrict gprp) { gprp->bitset = 0; }

#define SET_BIT(B, r) ((B) |= (u16)(1 << r))
#define CLR_BIT(B, r) ((B) &= (u16)(~(1 << r)))
#define CHK_BIT(B, r) (((B) >> r) & 1)

/**
 * @brief force allocate the given register
 *
 * @param gprp
 * @param r
 */
static void gprp_force(GPRP *restrict gprp, X64GPR r) {
  SET_BIT(gprp->bitset, r);
}

static bool gprp_any_available(GPRP *restrict gprp, X64GPR *restrict r) {
  for (u8 i = 0; i < 16; ++i) {
    if (!CHK_BIT(gprp->bitset, i)) {
      *r = (X64GPR)i;
      return 1;
    }
  }
  return 0;
}

static bool gprp_any_available_other_than(GPRP *restrict gprp,
                                          X64GPR *restrict r, X64GPR avoid) {
  for (u8 i = 0; i < 16; ++i) {
    if (!CHK_BIT(gprp->bitset, i) && (i != avoid)) {
      *r = (X64GPR)i;
      return 1;
    }
  }
  return 0;
}

/**
 * @brief allocate the next available register
 *
 * @param[in] gprp
 * @param[out] r the allocated register
 * @return if a register could be allocated
 */
static bool gprp_allocate(GPRP *restrict gprp, X64GPR *restrict r) {
  if (gprp_any_available(gprp, r)) {
    SET_BIT(gprp->bitset, *r);
    return 1;
  }

  return 0;
}

static bool gprp_allocate_other_than(GPRP *restrict gprp, X64GPR *restrict r,
                                     X64GPR avoid) {
  if (gprp_any_available_other_than(gprp, r, avoid)) {
    SET_BIT(gprp->bitset, *r);
    return 1;
  }

  return 0;
}

static void gprp_release(GPRP *restrict gprp, X64GPR r) {
  CLR_BIT(gprp->bitset, r);
}

// static bool gprp_check(GPRP *restrict gprp, X64GPR gpr) {
//   return CHK_BIT(gprp->bitset, gpr);
// }

#undef SET_BIT
#undef CLR_BIT
#undef CHK_BIT

/**
 * @brief represents the lifetime of a local variable in the function.
 *
 * @note due to SSA form there is no such this as a "lifetime hole"
 * so all lifetime information is guaranteed to be contiguous. thus
 * it is safe to represent Lifetime as a (first_use, last_use) pair.
 * and this captures all relevant lifetime information.
 * hmm, we want to store lifetime information
 * "sorted by increasing last use"
 *
 *
 */
typedef struct Lifetime {
  u16 first_use;
  u16 last_use;
} Lifetime;

/**
 * @brief manages the lifetime information of all locals for a given
 * function.
 *
 * @note we don't need to dynamically grow this set because we already
 * know exactly how many SSA locals a function body uses.
 */
typedef struct Lifetimes {
  u16 count;
  Lifetime *buffer;
} Lifetimes;

static Lifetimes li_create(u16 count) {
  Lifetimes li = {.count = count, .buffer = callocate(count, sizeof(Lifetime))};
  return li;
}

static void li_destroy(Lifetimes *restrict li) {
  li->count = 0;
  free(li->buffer);
  li->buffer = NULL;
}

static Lifetime *li_at(Lifetimes *restrict li, u16 ssa) {
  assert(ssa < li->count);
  return li->buffer + ssa;
}

// walk the bytecode representing the function body.
// if an instruction assigns a value to a SSA local
// that is the first use of the SSA local.
// The last use is the last instruction which uses
// a SSA local as an operand.
//
// if we walk the bytecode in reverse, then we know
// the last use is the first use we encounter, and
// the first use is the instruction which defines
// the local (has the local in operand A)
static Lifetimes li_compute(FunctionBody *restrict body) {
  Bytecode *bc = &body->bc;
  Lifetimes li = li_create(body->ssa_count);

  for (u16 i = bc->length; i > 0; --i) {
    u16 inst      = i - 1;
    Instruction I = bc->buffer[inst];
    switch (INST_FORMAT(I)) {
    case IFMT_B: {
      if (INST_B_FORMAT(I) == OPRFMT_SSA) {
        Lifetime *Bl = li_at(&li, INST_B(I));
        if (inst > Bl->last_use) {
          Bl->last_use = inst;
        }
      }
      break;
    }

    case IFMT_AB: {
      u16 A         = INST_A(I);
      Lifetime *Al  = li_at(&li, A);
      Al->first_use = inst;

      if (INST_B_FORMAT(I) == OPRFMT_SSA) {
        Lifetime *Bl = li_at(&li, INST_B(I));
        if (inst > Bl->last_use) {
          Bl->last_use = inst;
        }
      }
      break;
    }

    case IFMT_ABC: {
      u16 A         = INST_A(I);
      Lifetime *Al  = li_at(&li, A);
      Al->first_use = inst;

      if (INST_B_FORMAT(I) == OPRFMT_SSA) {
        Lifetime *Bl = li_at(&li, INST_B(I));
        if (inst > Bl->last_use) {
          Bl->last_use = inst;
        }
      }

      if (INST_C_FORMAT(I) == OPRFMT_SSA) {
        Lifetime *Cl = li_at(&li, INST_C(I));
        if (inst > Cl->last_use) {
          Cl->last_use = inst;
        }
      }
      break;
    }

    default:
      unreachable();
    }
  }

  return li;
}

typedef enum AllocationKind {
  ALLOC_GPR,
  ALLOC_STACK,
} AllocationKind;

typedef struct Allocation {
  AllocationKind kind;
  union {
    X64GPR gpr;
    u16 offset;
  };
} Allocation;

static Allocation alloc_reg(X64GPR gpr) {
  Allocation a = {.kind = ALLOC_GPR, .gpr = gpr};
  return a;
}

static Allocation alloc_stack(u16 offset) {
  Allocation a = {.kind = ALLOC_STACK, .offset = offset};
  return a;
}

typedef struct ActiveAllocation {
  u16 ssa;
  Lifetime lifetime;
  Allocation allocation;
} ActiveAllocation;

typedef struct Active {
  u16 stack_size;
  u16 size;
  u16 capacity;
  ActiveAllocation *buffer;
} Active;

static Active active_create() {
  Active a = {.stack_size = 0, .size = 0, .capacity = 0, .buffer = NULL};
  return a;
}

static void active_destroy(Active *restrict a) {
  a->stack_size = 0;
  a->size       = 0;
  a->capacity   = 0;
  free(a->buffer);
  a->buffer = NULL;
}

static bool active_full(Active *restrict a) {
  return (a->size + 1) >= a->capacity;
}

static void active_grow(Active *restrict a) {
  Growth g    = array_growth_u16(a->capacity, sizeof(ActiveAllocation));
  a->buffer   = reallocate(a->buffer, g.alloc_size);
  a->capacity = (u16)g.new_capacity;
}

static ActiveAllocation active_add(Active *restrict active, u16 ssa,
                                   Lifetime lifetime, Allocation allocation) {
  if (active_full(active)) {
    active_grow(active);
  }

  // find the lifetime that ends later than the given lifetime
  // and insert before it.
  u16 i = 0;
  for (; i < active->size; ++i) {
    ActiveAllocation *al = active->buffer + i;
    if (al->lifetime.last_use > lifetime.last_use) {
      break;
    }
  }

  // shift all lifetimes after idx forward one location
  for (u16 j = active->size; j > i; --j) {
    active->buffer[j] = active->buffer[j - 1];
  }

  ActiveAllocation *aa = active->buffer + i;
  active->size += 1;

  *aa = (ActiveAllocation){
      .ssa = ssa, .lifetime = lifetime, .allocation = allocation};
  return *aa;
}

static void active_erase(Active *restrict a, ActiveAllocation *restrict aa) {
  u16 i = 0;
  for (; i < a->size; ++i) {
    ActiveAllocation *al = a->buffer + i;
    if (al->ssa == aa->ssa) {
      break;
    }
  }

  // move all lifetimes after i backwards one location
  for (u16 j = i; j < a->size; ++j) {
    a->buffer[j] = a->buffer[j + 1];
  }

  a->size -= 1;
}

// static void active_erase_ssa(Active *restrict a, u16 ssa) {
//   // find the index, i, of the ssa's lifetime
//   u16 i = 0;
//
// }

/**
 * @brief manages where SSA locals are allocated
 *
 * #TODO LocalAllocations can be refactored into a
 * simpler structure only containing (count, buffer)
 * and the structure containing all of these elements
 * can be renamed RegisterAllocator or something similar.
 *
 */
typedef struct LocalAllocator {
  GPRP gprp;
  Lifetimes lifetimes;
  Active active;
  u16 stack_size;
} LocalAllocator;

static LocalAllocator la_create(FunctionBody *restrict body) {
  LocalAllocator la = {.gprp       = gprp_create(),
                       .lifetimes  = li_compute(body),
                       .active     = active_create(),
                       .stack_size = 0};
  // reserve the stack pointer RSP
  // and the frame pointer RBP
  // such that locals do not get allocated to them.
  gprp_force(&la.gprp, X64GPR_RSP);
  gprp_force(&la.gprp, X64GPR_RBP);
  return la;
}

static void la_destroy(LocalAllocator *restrict la) {
  gprp_destroy(&la->gprp);
  li_destroy(&la->lifetimes);
  active_destroy(&la->active);
  la->stack_size = 0;
}

static u16 la_bump_active_stack_size(LocalAllocator *restrict la) {
  // #TODO: as a simplification we just use a full word to store each
  // local on the stack.
  la->active.stack_size += 8;

  if (la->stack_size < la->active.stack_size) {
    la->stack_size = la->active.stack_size;
  }
  return la->active.stack_size;
}

static void la_reduce_active_stack_size(LocalAllocator *restrict la) {
  // #TODO similarly to the total stack size we don't take into
  // account the size of the elements stored on the stack.
  la->active.stack_size -= 8;
}

// static void la_force_allocate(LocalAllocations *restrict la, u16 ssa,
//                               X64GPR gpr) {
//   gprp_force(&la->gprp, gpr);
//   la->buffer[ssa] = alloc_reg(gpr);
// }

static ActiveAllocation *la_allocation_of(LocalAllocator *restrict la,
                                          u16 ssa) {
  Active *active = &la->active;
  for (u16 i = 0; i < active->size; ++i) {
    ActiveAllocation *aa = active->buffer + i;
    if (aa->ssa == ssa) {
      return aa;
    }
  }
  return NULL;
}

static ActiveAllocation *la_allocation_at(LocalAllocator *restrict la,
                                          X64GPR gpr) {
  Active *active = &la->active;
  for (u16 i = 0; i < active->size; ++i) {
    ActiveAllocation *aa = active->buffer + i;
    if ((aa->allocation.kind == ALLOC_GPR) && (aa->allocation.gpr == gpr)) {
      return aa;
    }
  }
  return NULL;
}

/**
 * @brief look for any active lifetimes whose last use is
 * smaller than the current instruction index. i.e. their lifetime
 * has ended.
 *
 *
 * @param la
 * @param Idx
 */
static void la_expire_old_lifetimes(LocalAllocator *restrict la, u16 Idx) {
  Active *a  = &la->active;
  GPRP *gprp = &la->gprp;
  u16 end    = a->size;
  for (u16 i = 0; i < end; ++i) {
    // copy the active lifetime
    ActiveAllocation *al = a->buffer + i;

    // since we store active lifetimes in order
    // of increasing last_use, if we find an
    // active lifetime which ends later than
    // the current instruction (Idx), we know that the rest
    // of the active lifetimes also end later.
    // thus we can exit early.
    if (al->lifetime.last_use >= Idx) {
      return;
    }

    if (al->allocation.kind == ALLOC_GPR) {
      gprp_release(gprp, al->allocation.gpr);
    } else {
      la_reduce_active_stack_size(la);
    }

    active_erase(a, al);

    // since we remove an element from active lifetimes
    // we also have to update the end point, so we don't
    // read garbage. I think this is okay because
    // A - we don't try to access a lifetime after it is
    //  removed
    // B - we recompute the pointer on each iteration
    // C - ActiveLifetimes keeps all elements stored
    //  contiguously.
    // D - since we decrement the end by one when we remove
    //  one element, end stays lock-step with the size of
    //  ActiveLifetimes.
    // E - removing elements will never trigger a reallocation
    //  of the memory allocated for the buffer of active lifetimes.
    end -= 1;
    // since we removed an element, the next active lifetime
    // will be at the current index. so we update our index
    // here to account for that.
    i -= 1;
  }
}

/**
 * @brief allocate the given SSA local
 *
 * @param la
 * @param Idx the index of the current instruction
 * @param ssa the 'label' of the ssa local
 * @return Allocation*
 */
static ActiveAllocation la_allocate(LocalAllocator *restrict la, u16 Idx,
                                    u16 ssa) {
  X64GPR gpr         = 0;
  Lifetime *lifetime = li_at(&la->lifetimes, ssa);

  la_expire_old_lifetimes(la, Idx);

  if (gprp_allocate(&la->gprp, &gpr)) {
    return active_add(&la->active, ssa, *lifetime, alloc_reg(gpr));
  }

  // otherwise spill to the stack.
  return active_add(&la->active, ssa, *lifetime,
                    alloc_stack(la_bump_active_stack_size(la)));
}

static void codegen_alloc_operand(ActiveAllocation *restrict aa,
                                  String *restrict buffer) {
  switch (aa->allocation.kind) {
  case ALLOC_GPR: {
    string_append(buffer, "%");
    string_append_sv(buffer, gpr_to_sv(aa->allocation.gpr));
    break;
  }

  case ALLOC_STACK: {
    string_append(buffer, "-");
    string_append_u64(buffer, aa->allocation.offset);
    string_append(buffer, "(%rbp)");
    break;
  }

  default:
    unreachable();
  }
}

static void codegen_immediate_operand(i64 imm, String *restrict buffer) {
  string_append(buffer, "$");
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

static void codegen_reallocate_local(LocalAllocator *restrict la,
                                     ActiveAllocation *aa,
                                     String *restrict buffer) {
  X64GPR gpr = 0;
  if (gprp_allocate(&la->gprp, &gpr)) {
    // move allocation to new register.
    string_append(buffer, "\tmov %");
    string_append_sv(buffer, gpr_to_sv(aa->allocation.gpr));
    string_append(buffer, ", %");
    string_append_sv(buffer, gpr_to_sv(gpr));
    string_append(buffer, "\n");
    // update the existing allocation to the new register
    aa->allocation.gpr = gpr;
    return;
  }

  // spill allocation to the stack
  u16 offset = la_bump_active_stack_size(la);
  string_append(buffer, "\tmov %");
  string_append_sv(buffer, gpr_to_sv(aa->allocation.gpr));
  string_append(buffer, ", -");
  string_append_u64(buffer, offset);
  string_append(buffer, "(%rbp)\n");
  // update the existing allocation to the new stack slot
  aa->allocation.kind   = ALLOC_STACK;
  aa->allocation.offset = offset;
}

static void codegen_reallocate_local_other_than(LocalAllocator *restrict la,
                                                ActiveAllocation *aa,
                                                X64GPR avoid,
                                                String *restrict buffer) {
  X64GPR gpr = 0;
  if (gprp_allocate_other_than(&la->gprp, &gpr, avoid)) {
    // move allocation to new register.
    string_append(buffer, "\tmov %");
    string_append_sv(buffer, gpr_to_sv(aa->allocation.gpr));
    string_append(buffer, ", %");
    string_append_sv(buffer, gpr_to_sv(gpr));
    string_append(buffer, "\n");
    // update the existing allocation to the new register
    aa->allocation.gpr = gpr;
    return;
  }

  // spill allocation to the stack
  u16 offset = la_bump_active_stack_size(la);
  string_append(buffer, "\tmov %");
  string_append_sv(buffer, gpr_to_sv(aa->allocation.gpr));
  string_append(buffer, ", -");
  string_append_u64(buffer, offset);
  string_append(buffer, "(%rbp)\n");
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
static void codegen_release_gpr(LocalAllocator *restrict la, X64GPR gpr,
                                String *restrict buffer) {
  ActiveAllocation *aa = la_allocation_at(la, gpr);
  if (aa == NULL) {
    return; // gpr is available.
  }
  // else there is an allocation at the given gpr

  codegen_reallocate_local(la, aa, buffer);
}

static void codegen_zero_gpr(X64GPR gpr, String *restrict buffer) {
  string_append(buffer, "\tmov ");
  codegen_immediate_operand((i64)0, buffer);
  string_append(buffer, ", %");
  string_append_sv(buffer, gpr_to_sv(gpr));
  string_append(buffer, "\n");
}

/**
 * @brief force release some gpr, prioritizing old allocations, making sure to
 * reallocate any ssa local.
 *
 * @param la
 * @param buffer
 * @return X64GPR
 */
static X64GPR codegen_release_any_gpr(LocalAllocator *restrict la, u16 Idx,
                                      String *restrict buffer) {
  // attempt to release an old active allocation in a gpr.
  // this could release nothing, a gpr, or an old stack
  // allocation.
  la_expire_old_lifetimes(la, Idx);

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

static Allocation
codegen_create_temporary_from_immediate(LocalAllocator *restrict la, u16 Idx,
                                        i64 imm, X64GPR avoid,
                                        String *restrict buffer) {
  // attempt to release an old active allocation in a gpr.
  // this could release nothing, a gpr, or an old stack
  // allocation.
  la_expire_old_lifetimes(la, Idx);

  X64GPR gpr = 0;
  if (gprp_any_available_other_than(&la->gprp, &gpr, avoid)) {
    string_append(buffer, "\tmov ");
    codegen_immediate_operand(imm, buffer);
    string_append(buffer, ", %");
    string_append_sv(buffer, gpr_to_sv(gpr));
    string_append(buffer, "\n");

    return alloc_reg(gpr);
  } else {
    u16 offset = la_bump_active_stack_size(la);
    string_append(buffer, "\tmov ");
    codegen_immediate_operand(imm, buffer);
    string_append(buffer, ", -");
    string_append_u64(buffer, offset);
    string_append(buffer, "(%rbp)\n");

    return alloc_stack(offset);
  }
}

static Allocation
codegen_create_temporary_from_constant(LocalAllocator *restrict la, u16 Idx,
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
 * ActiveAllocation for the given <ssa> local. keeping <aa> alive if it's last
 * use is after the current instruction <Idx>
 *
 * @note i don't know what to call this, i just know i don't like the name.
 *
 * @param la the LocalAllocator
 * @param Idx the index of the current Instruction
 * @param aa the ActiveAllocation to copy into gpr
 * @param gpr the target register
 * @param buffer
 */
static ActiveAllocation
codegen_allocate_gpr_from_existing(LocalAllocator *restrict la, u16 Idx,
                                   ActiveAllocation *restrict aa, X64GPR gpr,
                                   u16 ssa, String *restrict buffer) {
  if ((aa->allocation.kind == ALLOC_GPR) && (aa->allocation.gpr == gpr)) {
    if (aa->lifetime.last_use > Idx) {
      // we have to keep <aa> alive so we have to move it out of this register
      codegen_reallocate_local(la, aa, buffer);
      // however we know that the correct value is still in <gpr>,
      // so we can just start ssa's ActiveLifetime associated with <gpr>
      // as if we had allocated it there
      return active_add(&la->active, ssa, *li_at(&la->lifetimes, ssa),
                        alloc_reg(gpr));
    }
    // else we can remove the old lifetime
    active_erase(&la->active, aa);
    // and add the new one
    return active_add(&la->active, ssa, *li_at(&la->lifetimes, ssa),
                      alloc_reg(gpr));
  }
  // else (aa->allocation.kind != ALLOC_GPR) || (aa->allocation.gpr != gpr)
  // move the value of <aa> to the target register
  string_append(buffer, "\tmov ");
  codegen_alloc_operand(aa, buffer);
  string_append(buffer, ", %");
  string_append_sv(buffer, gpr_to_sv(gpr));
  string_append(buffer, "\n");

  // if <aa> is old, we can remove it from active
  if (aa->lifetime.last_use <= Idx) {
    active_erase(&la->active, aa);
  }
  // start the lifetime of ssa associated with gpr
  return active_add(&la->active, ssa, *li_at(&la->lifetimes, ssa),
                    alloc_reg(gpr));
}

static ActiveAllocation
codegen_allocate_gpr_from_immediate(LocalAllocator *restrict la, i64 imm,
                                    X64GPR gpr, u16 ssa,
                                    String *restrict buffer) {
  codegen_release_gpr(la, gpr, buffer);
  // initialize gpr with imm
  string_append(buffer, "\tmov ");
  codegen_immediate_operand(imm, buffer);
  string_append(buffer, ", %");
  string_append_sv(buffer, gpr_to_sv(gpr));
  string_append(buffer, "\n");

  return active_add(&la->active, ssa, *li_at(&la->lifetimes, ssa),
                    alloc_reg(gpr));
}

static ActiveAllocation
codegen_allocate_mem_from_immediate(LocalAllocator *restrict la, i64 imm,
                                    u16 ssa, String *restrict buffer) {
  u16 offset = la_bump_active_stack_size(la);

  // initialize mem with imm
  string_append(buffer, "\tmov ");
  codegen_immediate_operand(imm, buffer);
  string_append(buffer, ", -");
  string_append_u64(buffer, offset);
  string_append(buffer, "(%rbp)\n");

  return active_add(&la->active, ssa, *li_at(&la->lifetimes, ssa),
                    alloc_stack(offset));
}

static ActiveAllocation
codegen_allocate_gpr_from_constant(LocalAllocator *restrict la, Value *constant,
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

static ActiveAllocation
codegen_allocate_mem_from_constant(LocalAllocator *restrict la, Value *constant,
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
 * @return ActiveAllocation*
 */
static ActiveAllocation
codegen_allocate_any_gpr_from_existing(LocalAllocator *restrict la, u16 Idx,
                                       ActiveAllocation *restrict aa, u16 ssa,
                                       String *restrict buffer) {
  X64GPR gpr;
  if (gprp_any_available(&la->gprp, &gpr)) {
    // an available gpr is 'any' gpr
    return codegen_allocate_gpr_from_existing(la, Idx, aa, gpr, ssa, buffer);
  } else {
    // if the existing ActiveAllocation is old, we can reuse it's gpr
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
 * @return ActiveAllocation*
 */
static ActiveAllocation codegen_allocate_any_gpr_or_mem_from_existing(
    LocalAllocator *restrict la, u16 Idx, ActiveAllocation *restrict aa,
    u16 ssa, String *restrict buffer) {
  if (aa->allocation.kind == ALLOC_GPR) {
    return codegen_allocate_any_gpr_from_existing(la, Idx, aa, ssa, buffer);
  }

  u16 offset = aa->allocation.offset;

  if (aa->lifetime.last_use <= Idx) {
    // we can allocate on top of the existing value
    active_erase(&la->active, aa);
  } else {
    // we have to use a new stack slot.
    offset = la_bump_active_stack_size(la);
  }

  // allocation the ssa
  return active_add(&la->active, ssa, *li_at(&la->lifetimes, ssa),
                    alloc_stack(offset));
}

static ActiveAllocation codegen_allocate_any_gpr_or_mem_from_immediate(
    LocalAllocator *restrict la, i64 imm, u16 ssa, String *restrict buffer) {
  X64GPR gpr;
  if (gprp_any_available(&la->gprp, &gpr)) {
    return codegen_allocate_gpr_from_immediate(la, imm, gpr, ssa, buffer);
  } else {
    return codegen_allocate_mem_from_immediate(la, imm, ssa, buffer);
  }
}

static ActiveAllocation
codegen_allocate_any_gpr_or_mem_from_constant(LocalAllocator *restrict la,
                                              Value *constant, u16 ssa,
                                              String *restrict buffer) {
  X64GPR gpr;
  if (gprp_any_available(&la->gprp, &gpr)) {
    return codegen_allocate_gpr_from_constant(la, constant, gpr, ssa, buffer);
  } else {
    return codegen_allocate_mem_from_constant(la, constant, ssa, buffer);
  }
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
static void codegen_ret(Context *restrict c, LocalAllocator *restrict la,
                        Instruction I, String *restrict buffer) {
  // - find out where operand B is
  u16 B = INST_B(I);
  // - make sure B is stored in %rax
  switch (INST_B_FORMAT(I)) {
  case OPRFMT_SSA: {
    ActiveAllocation *Balloc = la_allocation_of(la, B);
    if ((Balloc->allocation.kind == ALLOC_GPR) &&
        (Balloc->allocation.gpr == X64GPR_RAX)) {
      break;
    }

    // "mov <Balloc>, %rax"
    string_append(buffer, "\tmov ");
    codegen_alloc_operand(Balloc, buffer);
    string_append(buffer, ", %rax\n");
    break;
  }

  case OPRFMT_CONSTANT: {
    // "mov $<imm>, %rax"
    string_append(buffer, "\tmov ");
    codegen_constant_operand(context_constants_at(c, B), buffer);
    string_append(buffer, ", %rax\n");
    break;
  }

  case OPRFMT_IMMEDIATE: {
    // "mov $<imm>, %rax"
    string_append(buffer, "\tmov ");
    codegen_immediate_operand((i64)B, buffer);
    string_append(buffer, ", %rax\n");
    break;
  }

  default:
    unreachable();
  }

  // - emit the function epilouge
  string_append(buffer, "\tmov %rbp, %rsp\n");
  string_append(buffer, "\tpop %rbp\n");
  string_append(buffer, "\tret\n");
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
static void codegen_mov(Context *restrict c, LocalAllocator *restrict la,
                        Instruction I, u16 Idx, String *restrict buffer) {

  u16 A                   = INST_A(I);
  u16 B                   = INST_B(I);
  ActiveAllocation Aalloc = la_allocate(la, Idx, A);
  // - "mov <B>, <A>"
  switch (INST_B_FORMAT(I)) {
  // "mov <Balloc>, <A>"
  case OPRFMT_SSA: {
    ActiveAllocation Balloc = *la_allocation_of(la, B);

    // "mov -<B-offset>(%rbp), %<tmp-gpr>"
    // "mov %<tmp-gpr>, -<A-offset>(%rbp)"
    if ((Aalloc.allocation.kind == ALLOC_STACK) &&
        (Balloc.allocation.kind == ALLOC_STACK)) {
      X64GPR gpr = codegen_release_any_gpr(la, Idx, buffer);

      string_append(buffer, "\tmov -");
      string_append_u64(buffer, Balloc.allocation.offset);
      string_append(buffer, "(%rbp), %");
      string_append_sv(buffer, gpr_to_sv(gpr));
      string_append(buffer, "\n");

      string_append(buffer, "\tmov %");
      string_append_sv(buffer, gpr_to_sv(gpr));
      string_append(buffer, ", -");
      string_append_u64(buffer, Aalloc.allocation.offset);
      string_append(buffer, "(%rbp)\n");
      return;
    }

    // "mov <Balloc>, <Aalloc>"
    string_append(buffer, "\tmov ");
    codegen_alloc_operand(&Balloc, buffer);
    string_append(buffer, ", ");
    codegen_alloc_operand(&Aalloc, buffer);
    string_append(buffer, "\n");
    return;
  } // !case OPRFMT_SSA:

  // "mov $<B-imm>, <A>"
  case OPRFMT_CONSTANT: {
    string_append(buffer, "\tmov ");
    codegen_constant_operand(context_constants_at(c, B), buffer);
    string_append(buffer, ", ");
    codegen_alloc_operand(&Aalloc, buffer);
    string_append(buffer, "\n");
    break;
  } // case OPRFMT_CONSTANT:

  // "mov $<B-imm>, <A>"
  case OPRFMT_IMMEDIATE: {
    string_append(buffer, "\tmov ");
    codegen_immediate_operand((i64)B, buffer);
    string_append(buffer, ", ");
    codegen_alloc_operand(&Aalloc, buffer);
    string_append(buffer, "\n");
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
static void codegen_neg(LocalAllocator *restrict la, Instruction I, u16 Idx,
                        String *restrict buffer) {
  u16 A = INST_A(I);
  u16 B = INST_B(I);

  // assert that we don't generate trivially foldable instructions.
  assert(INST_B_FORMAT(I) == OPRFMT_SSA);

  // first, make sure that the operand A has the right value
  // and that we don't overwrite B if it is still used after
  // this instruction.
  ActiveAllocation Balloc = *la_allocation_of(la, B);
  ActiveAllocation Aalloc = codegen_allocate_any_gpr_or_mem_from_existing(
      la, Idx, &Balloc, A, buffer);

  // "neg <Aalloc>"
  string_append(buffer, "\tneg ");
  codegen_alloc_operand(&Aalloc, buffer);
  string_append(buffer, "\n");
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
static void codegen_add(Context *restrict c, LocalAllocator *restrict la,
                        Instruction I, u16 Idx, String *restrict buffer) {
  u16 A = INST_A(I);
  u16 B = INST_B(I);
  u16 C = INST_C(I);

  ActiveAllocation Aalloc = {};
  switch (INST_B_FORMAT(I)) {
  case OPRFMT_SSA: {
    // make sure the an operand is present in the destination register.
    // and create the allocation of A at the destrination register.
    ActiveAllocation Balloc = *la_allocation_of(la, B);
    switch (INST_C_FORMAT(I)) {
      // B and C are SSA
    case OPRFMT_SSA: {
      ActiveAllocation Calloc = *la_allocation_of(la, C);

      if (Balloc.allocation.kind == ALLOC_GPR) {
        // since we know B is in a register, we can use
        // that register as the destination operand for the
        // addition. we can use C's location as the source
        // operand if it is either reg or mem.
        Aalloc = codegen_allocate_gpr_from_existing(
            la, Idx, &Balloc, Balloc.allocation.gpr, A, buffer);
        // "add <Calloc>, <Aalloc>"
        string_append(buffer, "\tadd ");
        codegen_alloc_operand(&Calloc, buffer);
        string_append(buffer, ", ");
        codegen_alloc_operand(&Aalloc, buffer);
        string_append(buffer, "\n");
      } else if (Calloc.allocation.kind == ALLOC_GPR) {
        // since we know C is in a register, we can use
        // that register as the destination operand for
        // the addition. we can use B as the source operand
        // from it's memory location.
        Aalloc = codegen_allocate_gpr_from_existing(
            la, Idx, &Calloc, Calloc.allocation.gpr, A, buffer);
        // "add <Balloc>, <Aalloc>"
        string_append(buffer, "\tadd ");
        codegen_alloc_operand(&Balloc, buffer);
        string_append(buffer, ", ");
        codegen_alloc_operand(&Aalloc, buffer);
        string_append(buffer, "\n");
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
        string_append(buffer, "\tadd ");
        codegen_alloc_operand(&Calloc, buffer);
        string_append(buffer, ", ");
        codegen_alloc_operand(&Aalloc, buffer);
        string_append(buffer, "\n");
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
      string_append(buffer, "\tadd ");
      codegen_constant_operand(Cconst, buffer);
      string_append(buffer, ", ");
      codegen_alloc_operand(&Aalloc, buffer);
      string_append(buffer, "\n");
      break;
    }

    // B is SSA, C is immediate
    case OPRFMT_IMMEDIATE: {
      // we can just embed C directly into the instruction.
      // first we have to start A's lifetime. using the value of B
      Aalloc =
          codegen_allocate_any_gpr_from_existing(la, Idx, &Balloc, A, buffer);
      // "add <Aalloc>, <Cimm>"
      string_append(buffer, "\tadd ");
      codegen_immediate_operand((i64)C, buffer);
      string_append(buffer, ", ");
      codegen_alloc_operand(&Aalloc, buffer);
      string_append(buffer, "\n");
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
    assert(INST_C_FORMAT(I) == OPRFMT_SSA);
    ActiveAllocation Calloc = *la_allocation_of(la, C);
    // we can just embed B directly into the instruction.
    // first we have to start A's lifetime. using the value of C
    Aalloc =
        codegen_allocate_any_gpr_from_existing(la, Idx, &Calloc, A, buffer);
    string_append(buffer, "\tadd ");
    codegen_constant_operand(context_constants_at(c, B), buffer);
    string_append(buffer, ", ");
    codegen_alloc_operand(&Aalloc, buffer);
    string_append(buffer, "\n");
    break;
  }

  // B is immediate
  case OPRFMT_IMMEDIATE: {
    // assert that we don't generate trivially foldable instructions.
    assert(INST_C_FORMAT(I) == OPRFMT_SSA);
    ActiveAllocation Calloc = *la_allocation_of(la, C);
    // we can just embed B directly into the instruction.
    // first we have to start A's lifetime. using the value of C
    Aalloc =
        codegen_allocate_any_gpr_from_existing(la, Idx, &Calloc, A, buffer);
    string_append(buffer, "\tadd ");
    codegen_immediate_operand((i64)B, buffer);
    string_append(buffer, ", ");
    codegen_alloc_operand(&Aalloc, buffer);
    string_append(buffer, "\n");
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
static void codegen_sub(Context *restrict c, LocalAllocator *restrict la,
                        Instruction I, u16 Idx, String *restrict buffer) {
  u16 A = INST_A(I);
  u16 B = INST_B(I);
  u16 C = INST_C(I);

  ActiveAllocation Aalloc = {};
  switch (INST_B_FORMAT(I)) {
  case OPRFMT_SSA: {
    ActiveAllocation Balloc = *la_allocation_of(la, B);
    switch (INST_C_FORMAT(I)) {
    case OPRFMT_SSA: {
      ActiveAllocation Calloc = *la_allocation_of(la, C);
      Aalloc =
          codegen_allocate_any_gpr_from_existing(la, Idx, &Balloc, A, buffer);
      // recall that in AT&T syntax the destination is on the right.
      // so "<A> - <C>" gets lowered to  "sub <C>, <A>"
      string_append(buffer, "\tsub ");
      codegen_alloc_operand(&Calloc, buffer);
      string_append(buffer, ", ");
      codegen_alloc_operand(&Aalloc, buffer);
      string_append(buffer, "\n");
      break;
    }

    case OPRFMT_CONSTANT: {
      Value *Cconst = context_constants_at(c, C);

      Aalloc =
          codegen_allocate_any_gpr_from_existing(la, Idx, &Balloc, A, buffer);
      string_append(buffer, "\tsub ");
      codegen_constant_operand(Cconst, buffer);
      string_append(buffer, ", ");
      codegen_alloc_operand(&Aalloc, buffer);
      string_append(buffer, "\n");
      break;
    }

    case OPRFMT_IMMEDIATE: {
      Aalloc =
          codegen_allocate_any_gpr_from_existing(la, Idx, &Balloc, A, buffer);
      string_append(buffer, "\tsub ");
      codegen_immediate_operand((u16)C, buffer);
      string_append(buffer, ", ");
      codegen_alloc_operand(&Aalloc, buffer);
      string_append(buffer, "\n");
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
    assert(INST_C_FORMAT(I) == OPRFMT_SSA);
    ActiveAllocation Calloc = *la_allocation_of(la, C);
    Value *Bconst           = context_constants_at(c, B);
    ActiveAllocation Aalloc =
        codegen_allocate_any_gpr_or_mem_from_constant(la, Bconst, A, buffer);

    string_append(buffer, "\tsub ");
    codegen_alloc_operand(&Calloc, buffer);
    string_append(buffer, ", ");
    codegen_alloc_operand(&Aalloc, buffer);
    string_append(buffer, "\n");
    break;
  }

  case OPRFMT_IMMEDIATE: {
    assert(INST_C_FORMAT(I) == OPRFMT_SSA);
    ActiveAllocation Calloc = *la_allocation_of(la, C);
    ActiveAllocation Aalloc =
        codegen_allocate_any_gpr_or_mem_from_immediate(la, (i64)B, A, buffer);

    string_append(buffer, "\tsub ");
    codegen_alloc_operand(&Calloc, buffer);
    string_append(buffer, ", ");
    codegen_alloc_operand(&Aalloc, buffer);
    string_append(buffer, "\n");
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
static void codegen_mul(Context *restrict c, LocalAllocator *restrict la,
                        Instruction I, u16 Idx, String *restrict buffer) {
  u16 A                                    = INST_A(I);
  u16 B                                    = INST_B(I);
  u16 C                                    = INST_C(I);
  [[maybe_unused]] ActiveAllocation Aalloc = {};
  switch (INST_B_FORMAT(I)) {
  case OPRFMT_SSA: {
    ActiveAllocation Balloc = *la_allocation_of(la, B);
    // make sure the value of B is held in %rax, for the imul instruction.
    Aalloc = codegen_allocate_gpr_from_existing(la, Idx, &Balloc, X64GPR_RAX, A,
                                                buffer);
    // make sure we are not overwriting an existing ActiveAllocation in %rdx
    // when we evaluate the imul instruction.
    codegen_release_gpr(la, X64GPR_RDX, buffer);
    switch (INST_C_FORMAT(I)) {
    case OPRFMT_SSA: {
      ActiveAllocation Calloc = *la_allocation_of(la, C);
      // "imul <Calloc>"
      string_append(buffer, "\timul ");
      codegen_alloc_operand(&Calloc, buffer);
      string_append(buffer, "\n");
      break;
    }

    case OPRFMT_CONSTANT: {
      // so, we can't have an immediate as the operand to imul
      // or idiv. however, we do know for sure that %rdx is free,
      // so we can use it to store the temporary.
      Value *Cconst = context_constants_at(c, C);
      string_append(buffer, "\tmov ");
      codegen_constant_operand(Cconst, buffer);
      string_append(buffer, ", %");
      string_append_sv(buffer, gpr_to_sv(X64GPR_RDX));
      string_append(buffer, "\n");

      string_append(buffer, "\timul %");
      string_append_sv(buffer, gpr_to_sv(X64GPR_RDX));
      string_append(buffer, "\n");
      break;
    }

    case OPRFMT_IMMEDIATE: {
      string_append(buffer, "\tmov ");
      codegen_immediate_operand((i64)C, buffer);
      string_append(buffer, ", %");
      string_append_sv(buffer, gpr_to_sv(X64GPR_RDX));
      string_append(buffer, "\n");

      string_append(buffer, "\timul %");
      string_append_sv(buffer, gpr_to_sv(X64GPR_RDX));
      string_append(buffer, "\n");
      break;
    }

    default:
      unreachable();
    }
    break;
  }

  case OPRFMT_CONSTANT: {
    assert(INST_C_FORMAT(I) == OPRFMT_SSA);
    ActiveAllocation Calloc = *la_allocation_of(la, C);
    Value *Bconst           = context_constants_at(c, B);
    // initialize A's ActiveAllocation with the value of B at %rax
    Aalloc =
        codegen_allocate_gpr_from_constant(la, Bconst, X64GPR_RAX, A, buffer);
    // make sure we are not overwriting an existing ActiveAllocation in %rdx
    // when we evaluate the imul instruction.
    codegen_release_gpr(la, X64GPR_RDX, buffer);
    string_append(buffer, "\timul ");
    codegen_alloc_operand(&Calloc, buffer);
    string_append(buffer, "\n");
    break;
  }

  case OPRFMT_IMMEDIATE: {
    assert(INST_C_FORMAT(I) == OPRFMT_SSA);
    ActiveAllocation Calloc = *la_allocation_of(la, C);
    Aalloc =
        codegen_allocate_gpr_from_immediate(la, (i64)B, X64GPR_RAX, A, buffer);
    // make sure we are not overwriting an existing ActiveAllocation in %rdx
    // when we evaluate the imul instruction.
    codegen_release_gpr(la, X64GPR_RDX, buffer);
    string_append(buffer, "\timul ");
    codegen_alloc_operand(&Calloc, buffer);
    string_append(buffer, "\n");
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
static void codegen_div(Context *restrict c, LocalAllocator *restrict la,
                        Instruction I, u16 Idx, String *restrict buffer) {
  u16 A                                    = INST_A(I);
  u16 B                                    = INST_B(I);
  u16 C                                    = INST_C(I);
  [[maybe_unused]] ActiveAllocation Aalloc = {};
  switch (INST_B_FORMAT(I)) {
  case OPRFMT_SSA: {
    ActiveAllocation Balloc = *la_allocation_of(la, B);
    // make sure the value of B is held in %rax, for the idiv instruction.
    Aalloc = codegen_allocate_gpr_from_existing(la, Idx, &Balloc, X64GPR_RAX, A,
                                                buffer);
    // make sure we are not overwriting an existing ActiveAllocation in %rdx
    // when we evaluate the idiv instruction.
    codegen_release_gpr(la, X64GPR_RDX, buffer);
    codegen_zero_gpr(X64GPR_RDX, buffer);
    switch (INST_C_FORMAT(I)) {
    case OPRFMT_SSA: {
      ActiveAllocation Calloc = *la_allocation_of(la, C);

      string_append(buffer, "\tidiv ");
      codegen_alloc_operand(&Calloc, buffer);
      string_append(buffer, "\n");
      break;
    }

    case OPRFMT_CONSTANT: {
      // so, we can't have an immediate as the operand to imul
      // or idiv. and %rdx is used as the upper bits of a the
      // dividend. so we have to use another temporary.
      Value *Cconst  = context_constants_at(c, C);
      Allocation tmp = codegen_create_temporary_from_constant(
          la, Idx, Cconst, X64GPR_RDX, buffer);

      string_append(buffer, "\tidiv ");
      if (tmp.kind == ALLOC_GPR) {
        string_append(buffer, "%");
        string_append_sv(buffer, gpr_to_sv(tmp.gpr));
      } else {
        string_append(buffer, "-");
        string_append_u64(buffer, tmp.offset);
        string_append(buffer, "(%rbp)");
      }
      string_append(buffer, "\n");
      break;
    }

    case OPRFMT_IMMEDIATE: {
      Allocation tmp = codegen_create_temporary_from_immediate(
          la, Idx, (i64)C, X64GPR_RDX, buffer);

      string_append(buffer, "\tidiv ");
      if (tmp.kind == ALLOC_GPR) {
        string_append(buffer, "%");
        string_append_sv(buffer, gpr_to_sv(tmp.gpr));
      } else {
        string_append(buffer, "-");
        string_append_u64(buffer, tmp.offset);
        string_append(buffer, "(%rbp)");
      }
      string_append(buffer, "\n");
      break;
    }

    default:
      unreachable();
    }
    break;
  }

  case OPRFMT_CONSTANT: {
    assert(INST_C_FORMAT(I) == OPRFMT_SSA);
    ActiveAllocation *Calloc = la_allocation_of(la, C);
    if ((Calloc->allocation.kind == ALLOC_GPR) &&
        (Calloc->allocation.gpr == X64GPR_RDX)) {
      codegen_reallocate_local_other_than(la, Calloc, X64GPR_RDX, buffer);
    }

    Value *Bconst = context_constants_at(c, B);
    // initialize A's ActiveAllocation with the value of B at %rax
    Aalloc =
        codegen_allocate_gpr_from_constant(la, Bconst, X64GPR_RAX, A, buffer);
    // make sure we are not overwriting an existing ActiveAllocation in %rdx
    // when we evaluate the idiv instruction. and that rdx holds no erraneous
    // data
    codegen_release_gpr(la, X64GPR_RDX, buffer);
    codegen_zero_gpr(X64GPR_RDX, buffer);

    string_append(buffer, "\tidiv ");
    codegen_alloc_operand(Calloc, buffer);
    string_append(buffer, "\n");
    break;
  }

  case OPRFMT_IMMEDIATE: {
    assert(INST_C_FORMAT(I) == OPRFMT_SSA);
    ActiveAllocation *Calloc = la_allocation_of(la, C);
    if ((Calloc->allocation.kind == ALLOC_GPR) &&
        (Calloc->allocation.gpr == X64GPR_RDX)) {
      codegen_reallocate_local_other_than(la, Calloc, X64GPR_RDX, buffer);
    }

    Aalloc =
        codegen_allocate_gpr_from_immediate(la, (i64)B, X64GPR_RAX, A, buffer);
    // make sure we are not overwriting an existing ActiveAllocation in %rdx
    // when we evaluate the idiv instruction.
    codegen_release_gpr(la, X64GPR_RDX, buffer);
    codegen_zero_gpr(X64GPR_RDX, buffer);

    string_append(buffer, "\tidiv ");
    codegen_alloc_operand(Calloc, buffer);
    string_append(buffer, "\n");
    break;
  }

  default:
    unreachable();
  }
}

static void codegen_bytecode(Context *restrict c, LocalAllocator *restrict la,
                             Bytecode *restrict bc, String *restrict buffer) {
  Instruction *ip = bc->buffer;
  for (u16 idx = 0; idx < bc->length; ++idx) {
    Instruction I = ip[idx];
    switch (INST_OP(I)) {

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
    case OPC_MOD: {

      break;
    }

    default:
      unreachable();
    }
  }
}

static void codegen_function(Context *restrict c, String *restrict buffer,
                             StringView name, FunctionBody *restrict body) {
  Bytecode *bc       = &body->bc;
  LocalAllocator la  = la_create(body);
  String body_buffer = string_create();

  codegen_bytecode(c, &la, bc, &body_buffer);

  directive_text(buffer);
  directive_globl(name, buffer);
  directive_type(name, STT_FUNC, buffer);
  directive_label(name, buffer);

  string_append(buffer, "\tpush %rbp\n");
  string_append(buffer, "\tmov %rsp, %rbp\n");
  if (la.stack_size != 0) {
    string_append(buffer, "\tsub $");
    string_append_u64(buffer, la.stack_size);
    string_append(buffer, ", %rsp\n");
  }

  string_append(buffer, body_buffer.buffer);

  directive_size_label_relative(name, buffer);

  la_destroy(&la);
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
  string_append(buffer, "\n");
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
    string_append(&buffer, "\n");

    symbol_table_iterator_next(&iter);
  }

  emit_file_epilouge(&buffer);

  StringView path = context_assembly_path(context);
  FILE *file      = file_open(path.ptr, "w");
  file_write(buffer.buffer, file);
  file_close(file);

  string_destroy(&buffer);
  return EXIT_SUCCESS;
}