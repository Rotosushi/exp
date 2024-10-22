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

#include "backend/lifetimes.h"
#include "utility/alloc.h"
#include "utility/array_growth.h"
#include "utility/unreachable.h"

static Lifetime lifetime_create(u64 first_use, u64 last_use) {
  Lifetime lifetime = {.first_use = first_use, .last_use = last_use};
  return lifetime;
}

Lifetime lifetime_immortal(u64 Idx) { return lifetime_create(Idx, u64_MAX); }

Lifetime lifetime_one_shot(u64 Idx) { return lifetime_create(Idx, Idx); }

Lifetimes lifetimes_create(u64 count) {
  Lifetimes lifetimes = {.count    = count,
                         .capacity = count,
                         .buffer   = callocate(count, sizeof(Lifetime))};
  return lifetimes;
}

void lifetimes_destroy(Lifetimes *restrict lifetimes) {
  lifetimes->count    = 0;
  lifetimes->capacity = 0;
  deallocate(lifetimes->buffer);
  lifetimes->buffer = NULL;
}

Lifetime lifetimes_at(Lifetimes *restrict lifetimes, u64 ssa) {
  assert(ssa < lifetimes->count);
  return lifetimes->buffer[ssa];
}

static void compute_operand(OperandFormat format,
                            OperandValue operand,
                            u64 inst,
                            Lifetimes *restrict lifetimes,
                            Context *restrict context);

static void compute_value(Value *restrict value,
                          u64 inst,
                          Lifetimes *restrict lifetimes,
                          Context *restrict context) {
  switch (value->kind) {
  case VALUEKIND_TUPLE: {
    Tuple *tuple = &value->tuple;
    for (u64 i = 0; i < tuple->size; ++i) {
      Operand element = tuple->elements[i];
      compute_operand(element.format, element.value, inst, lifetimes, context);
    }
    break;
  }

  default: return;
  }
}

static void compute_operand(OperandFormat format,
                            OperandValue operand,
                            u64 inst,
                            Lifetimes *restrict lifetimes,
                            Context *restrict context) {
  switch (format) {
  case OPRFMT_SSA: {
    Lifetime *lifetime = lifetimes_at(lifetimes, operand.ssa);
    if (inst > lifetime->last_use) { lifetime->last_use = inst; }
    break;
  }

  case OPRFMT_VALUE: {
    Value *value = context_values_at(context, operand.index);
    compute_value(value, inst, lifetimes, context);
    break;
  }

  case OPRFMT_CALL: {
    ActualArgumentList *list = context_call_at(context, operand.index);
    for (u64 i = 0; i < list->size; ++i) {
      Operand argument = list->list[i];
      compute_operand(
          argument.format, argument.value, inst, lifetimes, context);
    }
    break;
  }

  default: return;
  }
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
Lifetimes lifetimes_compute(FunctionBody *restrict body,
                            Context *restrict context) {
  Bytecode *bc        = &body->bc;
  Lifetimes lifetimes = lifetimes_create(body->ssa_count);

  for (u64 i = bc->length; i > 0; --i) {
    u64 inst      = i - 1;
    Instruction I = bc->buffer[inst];
    switch (I.format) {
    case IFMT_B: {
      compute_operand(I.B_format, I.B, inst, &lifetimes, context);
      break;
    }

    case IFMT_AB: {
      compute_operand(I.A_format, I.A, inst, &lifetimes, context);
      compute_operand(I.B_format, I.B, inst, &lifetimes, context);
      break;
    }

    case IFMT_ABC: {
      compute_operand(I.A_format, I.A, inst, &lifetimes, context);
      compute_operand(I.B_format, I.B, inst, &lifetimes, context);
      compute_operand(I.C_format, I.C, inst, &lifetimes, context);
      break;
    }

    default: EXP_UNREACHABLE;
    }
  }

  return lifetimes;
}

static bool lifetimes_full(Lifetimes *restrict lifetimes) {
  return (lifetimes->count + 1) >= lifetimes->capacity;
}

static void lifetimes_grow(Lifetimes *restrict lifetimes) {
  Growth g            = array_growth_u64(lifetimes->capacity, sizeof(Lifetime));
  lifetimes->buffer   = reallocate(lifetimes->buffer, g.alloc_size);
  lifetimes->capacity = g.new_capacity;
}

void lifetimes_add(Lifetimes *restrict lifetimes, Lifetime lifetime) {
  if (lifetimes_full(lifetimes)) { lifetimes_grow(lifetimes); }

  lifetimes->buffer[lifetimes->count++] = lifetime;
}
