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

// static void print_gpr(X64GPR r, FILE *restrict file) {
//   file_write("%", file);
//   print_string_view(gpr_to_sv(r), file);
// }

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

/**
 * @brief allocate the next available register
 *
 * @param[in] gprp
 * @param[out] r the allocated register
 * @return if a register could be allocated
 */
static bool gprp_allocate(GPRP *restrict gprp, X64GPR *restrict r) {
  assert(r != NULL);
  for (u8 i = 0; i < 16; ++i) {
    if (!CHK_BIT(gprp->bitset, i)) {
      SET_BIT(gprp->bitset, i);
      *r = (X64GPR)i;
      return 1;
    }
  }
  return 0;
}

static void gprp_release(GPRP *restrict gprp, X64GPR r) {
  CLR_BIT(gprp->bitset, r);
}

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
  Lifetimes li = li_create(body->local_count);

  for (u16 i = bc->length; i > 0; --i) {
    u16 inst      = i - 1;
    Instruction I = bc->buffer[inst];
    switch (INST_FORMAT(I)) {
    case IFMT_B: {
      Lifetime *Bl = li_at(&li, INST_B(I));
      if ((INST_B_FORMAT(I) == OPRFMT_SSA) && (inst > Bl->last_use)) {
        Bl->last_use = inst;
      }
      break;
    }

    case IFMT_AB: {
      u16 A         = INST_A(I);
      Lifetime *Al  = li_at(&li, A);
      Al->first_use = inst;

      u16 B        = INST_B(I);
      Lifetime *Bl = li_at(&li, B);
      if ((INST_B_FORMAT(I) == OPRFMT_SSA) && (inst > Bl->last_use)) {
        Bl->last_use = inst;
      }
      break;
    }

    case IFMT_ABC: {
      u16 A         = INST_A(I);
      Lifetime *Al  = li_at(&li, A);
      Al->first_use = inst;

      u16 B        = INST_B(I);
      Lifetime *Bl = li_at(&li, B);
      if ((INST_B_FORMAT(I) == OPRFMT_SSA) && (inst > Bl->last_use)) {
        Bl->last_use = inst;
      }

      u16 C        = INST_C(I);
      Lifetime *Cl = li_at(&li, C);
      if ((INST_C_FORMAT(I) == OPRFMT_SSA) && (inst > Cl->last_use)) {
        Cl->last_use = inst;
      }
      break;
    }

    default:
      unreachable();
    }
  }

  return li;
}

typedef struct ActiveLifetime {
  u16 ssa;
  Lifetime lifetime;
} ActiveLifetime;

typedef struct Active {
  u16 stack_size;
  u16 size;
  u16 capacity;
  ActiveLifetime *buffer;
} Active;

static Active a_create() {
  Active a = {.stack_size = 0, .size = 0, .capacity = 0, .buffer = NULL};
  return a;
}

static void a_destroy(Active *restrict a) {
  a->stack_size = 0;
  a->size       = 0;
  a->capacity   = 0;
  free(a->buffer);
  a->buffer = NULL;
}

static bool a_full(Active *restrict a) { return (a->size + 1) >= a->capacity; }

static void a_grow(Active *restrict a) {
  Growth g    = array_growth_u16(a->capacity, sizeof(ActiveLifetime));
  a->buffer   = reallocate(a->buffer, g.alloc_size);
  a->capacity = (u16)g.new_capacity;
}

static void a_add(Active *restrict a, u16 ssa, Lifetime l) {
  if (a_full(a)) {
    a_grow(a);
  }

  // find the lifetime that ends later than the given lifetime
  // and insert before it.
  u16 i = 0;
  for (; i < a->size; ++i) {
    ActiveLifetime *al = a->buffer + i;
    if (al->lifetime.last_use > l.last_use) {
      break;
    }
  }

  // shift all lifetimes after idx forward one location
  for (u16 j = a->size; j > i; --j) {
    a->buffer[j] = a->buffer[j - 1];
  }

  a->buffer[i] = (ActiveLifetime){.ssa = ssa, .lifetime = l};
  a->size += 1;
}

static void a_remove(Active *restrict a, u16 ssa) {
  // find the index, i, of the ssa's lifetime
  u16 i = 0;
  for (; i < a->size; ++i) {
    ActiveLifetime *al = a->buffer + i;
    if (al->ssa == ssa) {
      break;
    }
  }

  // move all lifetimes after i backwards one location
  for (u16 j = i; j < a->size; ++j) {
    a->buffer[j] = a->buffer[j + 1];
  }

  a->size -= 1;
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

/**
 * @brief manages where SSA locals are allocated
 *
 * #TODO LocalAllocations can be refactored into a
 * simpler structure only containing (count, buffer)
 * and the structure containing all of these elements
 * can be renamed RegisterAllocator or something similar.
 *
 */
typedef struct LocalAllocations {
  GPRP gprp;
  Lifetimes lifetimes;
  Active active;
  u16 stack_size;
  u16 count;
  Allocation *buffer;
} LocalAllocations;

static LocalAllocations la_create(FunctionBody *restrict body) {
  LocalAllocations la = {.gprp       = gprp_create(),
                         .lifetimes  = li_compute(body),
                         .active     = a_create(),
                         .stack_size = 0,
                         .count      = body->local_count,
                         .buffer =
                             callocate(body->local_count, sizeof(Allocation))};
  // reserve the stack pointer RSP
  // and the frame pointer RBP
  // such that locals do not get allocated to them.
  gprp_force(&la.gprp, X64GPR_RSP);
  gprp_force(&la.gprp, X64GPR_RBP);
  return la;
}

static void la_destroy(LocalAllocations *restrict la) {
  gprp_destroy(&la->gprp);
  li_destroy(&la->lifetimes);
  a_destroy(&la->active);
  la->stack_size = 0;
  la->count      = 0;
  free(la->buffer);
  la->buffer = NULL;
}

// static void la_force_allocate(LocalAllocations *restrict la, u16 ssa,
//                               X64GPR gpr) {
//   gprp_force(&la->gprp, gpr);
//   la->buffer[ssa] = alloc_reg(gpr);
// }

static Allocation *la_at(LocalAllocations *restrict la, u16 ssa) {
  return la->buffer + ssa;
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
static void la_expire_old_lifetimes(LocalAllocations *restrict la, u16 Idx) {
  Active *a  = &la->active;
  GPRP *gprp = &la->gprp;
  u16 end    = a->size;
  for (u16 i = 0; i < end; ++i) {
    // copy the active lifetime
    ActiveLifetime *al = a->buffer + i;

    // since we store active lifetimes in order
    // of increasing last_use, if we find an
    // active lifetime which ends later than
    // the current instruction (Idx), we know that the rest
    // of the active lifetimes also end later.
    // thus we can exit early.
    if (al->lifetime.last_use >= Idx) {
      return;
    }

    Allocation *ssa_alloc = la_at(la, al->ssa);
    if (ssa_alloc->kind == ALLOC_GPR) {
      gprp_release(gprp, ssa_alloc->gpr);
    } else {
      // #NOTE, active lifetimes also counts the active stack size.
      // so we update that here if we are releasing a spilled local.
      // so this is like a stack_release(...) akin to the above
      // gprp_release
      //
      // #TODO similarly to the total stack size we don't take into
      // account the size of the elements stored on the stack right
      // now, since everything fits in a machine word, we are using
      // a stack slot size of 8 for each spilled local.
      a->stack_size -= 8;
    }

    a_remove(a, al->ssa);

    // since we remove an element from active lifetimes
    // we also have to update the end point, so we don't
    // read garbage. I think this is okay because
    // A - we don't try to access a lifetime after it is
    //  removed
    // B - we recompute the pointer on each iteration,
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
static Allocation *la_allocate(LocalAllocations *restrict la, u16 Idx,
                               u16 ssa) {
  assert(ssa < la->count);

  X64GPR gpr    = 0;
  Allocation *a = la->buffer + ssa;
  Lifetime *l   = li_at(&la->lifetimes, ssa);

  la_expire_old_lifetimes(la, Idx);

  if (gprp_allocate(&la->gprp, &gpr)) {
    *a = alloc_reg(gpr);
    a_add(&la->active, ssa, *l);
  } else {
    // otherwise spill to the stack.
    //
    // #TODO as a simplification, every spilled local
    // takes up a single word. when we get more
    // granular types, and types larger than a single
    // word, the amount that we grow the stack each
    // time becomes dependent upon the sizeof the local.
    //
    // #NOTE we store the stack size as an unsigned integer,
    // but since the stack grows down we always want to
    // subtract the current size from the stack pointer %rsp
    //
    // #NOTE we can use the stack space already allocated
    // for a new allocation in the same way we reuse registers
    // iff the stack allocation expired. So this also needs to be
    // taken into account.
    la->active.stack_size += 8;
    // we have to update the total stack size only if
    // the active stack size grows beyond the total stack size.
    if (la->active.stack_size > la->stack_size) {
      la->stack_size = la->active.stack_size;
    }

    // #NOTE we are using the currently active
    // stack size as the offset from the frame
    // pointer to construct the address of the
    // stack slot where the allocation resides.
    *a = alloc_stack(la->active.stack_size);
    a_add(&la->active, ssa, *l);
  }
  return a;
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
static i32 codegen_ret(Context *restrict c, LocalAllocations *restrict la,
                       Instruction I, String *restrict buffer) {
  // - find out where operand B is
  u16 B = INST_B(I);
  // - make sure B is stored in %rax
  switch (INST_B_FORMAT(I)) {
    // we are returning a local value. So we need to
    // find out where that local value lives.
  case OPRFMT_SSA: {
    Allocation *Balloc = la_at(la, B);
    switch (Balloc->kind) {
    // if the local value is allocated in a GPR
    // we have to ensure that it is in the
    // return register %rax before we can emit
    // the return.
    case ALLOC_GPR: {
      if (Balloc->gpr != X64GPR_RAX) {
        // mov from the allocated gpr to %rax
        // (note that by definition "whatever value
        // was allocated in %rax"'s lifetime is ending,
        // so it is safe to overwrite here.)
        // "mov %<gpr>, %rax"
        string_append(buffer, "\tmov %");
        string_append_sv(buffer, gpr_to_sv(Balloc->gpr));
        string_append(buffer, ", %rax\n");
      }
      // else B is already in %rax
      break;
    }

    // the local is stack allocated, so we can simply load it to %rax
    case ALLOC_STACK: {
      // "mov -<offset>(%rbp), %rax"
      string_append(buffer, "\tmov -");
      string_append_u64(buffer, Balloc->offset);
      string_append(buffer, "(%rbp), %rax\n");

      break;
    }

    // there is nowhere else for the local to be allocated
    default:
      unreachable();
    }
    break;
  }

  // if we are returning some constant value
  // we can embed that constant as a immediate.
  // #NOTE: only for now! when we support types
  // larger than a single machine word this becomes
  // more complex. (pretty sure it becomes a stack
  // allocation.)
  case OPRFMT_CONSTANT: {
    Value *constant = context_constants_at(c, B);
    i64 imm         = 0;
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

    // "mov $<imm>, %rax"
    string_append(buffer, "\tmov $");
    string_append_i64(buffer, imm);
    string_append(buffer, ", %rax\n");
    break;
  }

  // if the operand is itself an immediate value
  // we can simply embed it into the mov instruction
  // directly.
  case OPRFMT_IMMEDIATE: {
    // "mov $<imm>, %rax"
    string_append(buffer, "\tmov $");
    string_append_u64(buffer, B);
    string_append(buffer, ", %rax\n");
    break;
  }

  // there are no other operand kinds
  default:
    unreachable();
  }

  // - emit the function epilouge
  string_append(buffer, "\tmov %rbp, %rsp\n");
  string_append(buffer, "\tpop %rbp");

  // - emit ret
  string_append(buffer, "\tret\n");

  return EXIT_SUCCESS;
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
// we just have to allocate A somewhere.
static i32 codegen_mov(Context *restrict c, LocalAllocations *restrict la,
                       Instruction I, u16 Idx, String *restrict buffer) {

  u16 A              = INST_A(I);
  u16 B              = INST_B(I);
  Allocation *Aalloc = la_allocate(la, Idx, A);
  // - "mov <B>, <A>"
  switch (INST_B_FORMAT(I)) {
  // "mov <Balloc>, <A>"
  case OPRFMT_SSA: {
    Allocation *Balloc = la_at(la, B);

    switch (Aalloc->kind) {
    // "mov <B>, %<A-gpr>"
    case ALLOC_GPR: {
      switch (Balloc->kind) {
      // "mov %<B-gpr>, %<A-gpr>"
      case ALLOC_GPR: {
        if (Balloc->gpr != Aalloc->gpr) {
          string_append(buffer, "\tmov %");
          string_append_sv(buffer, gpr_to_sv(Balloc->gpr));
          string_append(buffer, ", %");
          string_append_sv(buffer, gpr_to_sv(Aalloc->gpr));
          string_append(buffer, "\n");
        }
        // else A already contains B
        // #NOTE I am asking myself, "when would we have
        // ever allocated A precisely where we had allocated B?"
        // I am not entirely sure what the source code would look
        // like to produce the situation where the allocator
        // allocated B, then B was released, then when we allocated
        // A above it was allocated precisely where B used to be.
        // however iff that happens it is a trivial optimization
        // to simply not emit a mov instruction.
        break;
      }

      // "mov -<B-offset>(%rbp), %<A-gpr>"
      case ALLOC_STACK: {
        string_append(buffer, "\tmov -");
        string_append_u64(buffer, Balloc->offset);
        string_append(buffer, "(%rbp), %");
        string_append_sv(buffer, gpr_to_sv(Aalloc->gpr));
        string_append(buffer, "\n");
        break;
      }

      default:
        unreachable();
      }
      break;
    }

    // "mov <B>, -<A-offset>(%rbp)"
    case ALLOC_STACK: {
      switch (Balloc->kind) {
      // "mov %<B-gpr>, -<A-offset>(%rbp)"
      case ALLOC_GPR: {
        string_append(buffer, "\tmov %");
        string_append_sv(buffer, gpr_to_sv(Balloc->gpr));
        string_append(buffer, ", -");
        string_append_u64(buffer, Aalloc->offset);
        string_append(buffer, "(%rbp)\n");
        break;
      }

      // "mov -<B-offset>(%rbp), %<tmp-gpr>"
      // "mov %<tmp-gpr>, -<A-offset>(%rbp)"
      case ALLOC_STACK: {
        // #NOTE: OOF. this instruction gets expanded to
        // two instructions, and has to make use of a temp
        // register. this is because both A and B are allocated
        // to the stack, and "mov" can only handle 1 memory operand.
        // This -technically speaking- adds an extra SSA local we
        // have to reason about, which breaks the assumptions of
        // our LocalAllocator.
        // So hopefully we can sidestep this somehow.
        PANIC("#TODO");
        break;
      }

      default:
        unreachable();
      }
      break;
    }

    default:
      unreachable();
    }
    break;
  }

  // "mov $<B-imm>, <A>"
  case OPRFMT_CONSTANT: {
    i64 imm         = 0;
    Value *constant = context_constants_at(c, B);
    switch (constant->kind) {
    case VALUEKIND_UNINITIALIZED:
      PANIC("uninitialized value");
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

    switch (Aalloc->kind) {
    case ALLOC_GPR: {
      string_append(buffer, "\tmov $");
      string_append_i64(buffer, imm);
      string_append(buffer, ", %");
      string_append_sv(buffer, gpr_to_sv(Aalloc->gpr));
      string_append(buffer, "\n");
      break;
    }

    case ALLOC_STACK: {
      string_append(buffer, "\tmov $");
      string_append_i64(buffer, imm);
      string_append(buffer, ", -");
      string_append_u64(buffer, Aalloc->offset);
      string_append(buffer, "(%rbp)\n");
      break;
    }

    default:
      unreachable();
    }
    break;
  }

  // "mov $<B-imm>, <A>"
  case OPRFMT_IMMEDIATE: {
    switch (Aalloc->kind) {
    case ALLOC_GPR: {
      string_append(buffer, "\tmov $");
      string_append_u64(buffer, B);
      string_append(buffer, ", %");
      string_append_sv(buffer, gpr_to_sv(Aalloc->gpr));
      string_append(buffer, "\n");
      break;
    }

    case ALLOC_STACK: {
      string_append(buffer, "\tmov $");
      string_append_u64(buffer, B);
      string_append(buffer, ", -");
      string_append_u64(buffer, Aalloc->offset);
      string_append(buffer, "\n");
      break;
    }

    default:
      unreachable();
    }
    break;
  }

  default:
    unreachable();
  }

  return EXIT_SUCCESS;
}

static i32 codegen_bytecode(Context *restrict c, LocalAllocations *restrict la,
                            Bytecode *restrict bc, String *restrict buffer) {
  Instruction *ip = bc->buffer;
  for (u16 idx = 0; idx < bc->length; ++idx) {
    Instruction I = ip[idx];
    switch (INST_OP(I)) {

    case OPC_RET: {
      if (codegen_ret(c, la, I, buffer) == EXIT_FAILURE) {
        return EXIT_FAILURE;
      }
      break;
    }

    case OPC_MOVE: {
      if (codegen_mov(c, la, I, idx, buffer) == EXIT_FAILURE) {
        return EXIT_FAILURE;
      }
      break;
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
    case OPC_ADD: {

      break;
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
    case OPC_SUB: {

      break;
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
    case OPC_MUL: {

      break;
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
      // that exactly the dividend, operand B, can be
      // allocated to RAX before the instruction.
      // we must also note that the result, A, is
      // allocated to RAX afterwords.
    case OPC_DIV: {

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
  return EXIT_SUCCESS;
}

static i32 codegen_function(Context *restrict c, FILE *restrict file,
                            StringView name, FunctionBody *restrict body) {
  // there is a catch here, that gets in the way of simply
  // emitting x64 assembly directly as we walk through
  // the bytecode making up the function body.
  // and that is stack allocation of variables.
  // we need to emit the function prolouge right up front.
  // part of the prolouge is decrementing the stack pointer
  // to allocate space on the stack for local variables.
  // but we cannot know how much to decrement the stack
  // pointer until we know how many local variables are
  // going to be spilled to the stack. This cannot happen
  // until we attempt to allocate the local variables
  // and end up having to spill some to the stack.
  // Which is going to happen while we are attempting to
  // select instructions to emit, because we are combining
  // instruction selection and register allocation into the
  // same algorithm.
  // (we are ignoring instruction scheduling for now.)
  // the current solution is just buffer the converted x64
  // assembly in a string, then once it's done converting
  // we can write out the whole string, after writing out
  // the correct prolouge, and follow everything up with
  // the correct epilouge too.

  Bytecode *bc        = &body->bc;
  LocalAllocations la = la_create(body);
  String buffer       = string_create();

  if (codegen_bytecode(c, &la, bc, &buffer) == EXIT_FAILURE) {
    return EXIT_FAILURE;
  }

  directive_globl(name, file);
  directive_type(name, STT_FUNC, file);
  directive_label(name, file);

  file_write("\tpush rbp\n", file);
  file_write("\tmov %rsp, %rbp\n", file);
  if (la.stack_size != 0) {
    file_write("\tsub $", file);
    print_u64(la.stack_size, RADIX_DECIMAL, file);
    file_write(", %rsp\n", file);
  }

  file_write(buffer.buffer, file);

  directive_size_label_relative(name, file);

  la_destroy(&la);
  string_destroy(&buffer);

  return EXIT_SUCCESS;
}

static i32 codegen_ste(Context *restrict c, FILE *restrict file,
                       SymbolTableElement *restrict ste) {
  StringView name = ste->name;
  switch (ste->kind) {
  case STE_UNDEFINED: {
    return EXIT_FAILURE;
  }

  case STE_FUNCTION: {
    FunctionBody *body = &ste->function_body;
    return codegen_function(c, file, name, body);
  }

  default:
    unreachable();
  }
}

i32 codegen(Context *restrict context) {
  FILE *file               = context_open_output(context);
  SymbolTableIterator iter = context_global_symbol_iterator(context);
  while (!symbol_table_iterator_done(&iter)) {

    if (codegen_ste(context, file, iter.element) == EXIT_FAILURE) {
      return EXIT_FAILURE;
    }

    symbol_table_iterator_next(&iter);
  }

  file_close(file);
  return EXIT_SUCCESS;
}