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
#include "utility/unreachable.h"

Lifetime lifetime_immortal() {
  Lifetime lifetime = {.first_use = 0, .last_use = u64_MAX};
  return lifetime;
}

Lifetimes lifetimes_create(u64 count) {
  Lifetimes lifetiems = {.count  = count,
                         .buffer = callocate(count, sizeof(Lifetime))};
  return lifetiems;
}

void lifetimes_destroy(Lifetimes *restrict lifetiems) {
  lifetiems->count = 0;
  deallocate(lifetiems->buffer);
  lifetiems->buffer = NULL;
}

Lifetime *lifetimes_at(Lifetimes *restrict lifetiems, u64 ssa) {
  assert(ssa < lifetiems->count);
  return lifetiems->buffer + ssa;
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
      if (I.B.format == OPERAND_KIND_SSA) {
        Lifetime *Bl = lifetimes_at(&lifetimes, I.B.ssa);
        if (inst > Bl->last_use) { Bl->last_use = inst; }
      } else if (I.B.format == OPERAND_KIND_VALUE) {
        Value *value = context_values_at(context, I.B.index);
        assert(value->kind == VALUEKIND_TUPLE);
        Tuple *tuple = &value->tuple;

        for (u64 i = 0; i < tuple->size; ++i) {
          Operand element = tuple->elements[i];
          if (element.format == OPERAND_KIND_SSA) {
            Lifetime *el = lifetimes_at(&lifetimes, element.ssa);
            if (inst > el->last_use) { el->last_use = inst; }
          }
        }
      }
      break;
    }

    case IFMT_AB: {
      Lifetime *Al  = lifetimes_at(&lifetimes, I.A);
      Al->first_use = inst;

      if (I.B.format == OPERAND_KIND_SSA) {
        Lifetime *Bl = lifetimes_at(&lifetimes, I.B.ssa);
        if (inst > Bl->last_use) { Bl->last_use = inst; }
      } else if (I.B.format == OPERAND_KIND_VALUE) {
        Value *value = context_values_at(context, I.B.index);
        assert(value->kind == VALUEKIND_TUPLE);
        Tuple *tuple = &value->tuple;

        for (u64 i = 0; i < tuple->size; ++i) {
          Operand element = tuple->elements[i];
          if (element.format == OPERAND_KIND_SSA) {
            Lifetime *el = lifetimes_at(&lifetimes, element.ssa);
            if (inst > el->last_use) { el->last_use = inst; }
          }
        }
      }
      break;
    }

    case IFMT_ABC: {
      Lifetime *Al  = lifetimes_at(&lifetimes, I.A);
      Al->first_use = inst;

      if (I.B.format == OPERAND_KIND_SSA) {
        Lifetime *Bl = lifetimes_at(&lifetimes, I.B.ssa);
        if (inst > Bl->last_use) { Bl->last_use = inst; }
      } else if (I.B.format == OPERAND_KIND_VALUE) {
        Value *value = context_values_at(context, I.B.index);
        assert(value->kind == VALUEKIND_TUPLE);
        Tuple *tuple = &value->tuple;

        for (u64 i = 0; i < tuple->size; ++i) {
          Operand element = tuple->elements[i];
          if (element.format == OPERAND_KIND_SSA) {
            Lifetime *el = lifetimes_at(&lifetimes, element.ssa);
            if (inst > el->last_use) { el->last_use = inst; }
          }
        }
      }

      if (I.C.format == OPERAND_KIND_SSA) {
        Lifetime *Cl = lifetimes_at(&lifetimes, I.C.ssa);
        if (inst > Cl->last_use) { Cl->last_use = inst; }
      } else if (I.C.format == OPERAND_KIND_VALUE) {
        Value *value = context_values_at(context, I.C.index);
        assert(value->kind == VALUEKIND_TUPLE);
        Tuple *tuple = &value->tuple;

        for (u64 i = 0; i < tuple->size; ++i) {
          Operand element = tuple->elements[i];
          if (element.format == OPERAND_KIND_SSA) {
            Lifetime *el = lifetimes_at(&lifetimes, element.ssa);
            if (inst > el->last_use) { el->last_use = inst; }
          }
        }
      }
      break;
    }

    default: EXP_UNREACHABLE();
    }
  }

  return lifetimes;
}
