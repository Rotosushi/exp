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

Lifetimes li_create(u16 count) {
  Lifetimes li = {.count = count, .buffer = callocate(count, sizeof(Lifetime))};
  return li;
}

void li_destroy(Lifetimes *restrict li) {
  li->count = 0;
  deallocate(li->buffer);
  li->buffer = NULL;
}

Lifetime *li_at(Lifetimes *restrict li, u16 ssa) {
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
Lifetimes li_compute(FunctionBody *restrict body) {
  Bytecode *bc = &body->bc;
  Lifetimes li = li_create(body->ssa_count);

  for (u16 i = bc->length; i > 0; --i) {
    u16 inst      = i - 1;
    Instruction I = bc->buffer[inst];
    switch (I.I_format) {
    case IFMT_B: {
      if (I.Bfmt == OPRFMT_SSA) {
        Lifetime *Bl = li_at(&li, I.B);
        if (inst > Bl->last_use) { Bl->last_use = inst; }
      }
      break;
    }

    case IFMT_AB: {
      u16 A         = I.A;
      Lifetime *Al  = li_at(&li, A);
      Al->first_use = inst;

      if (I.Bfmt == OPRFMT_SSA) {
        Lifetime *Bl = li_at(&li, I.B);
        if (inst > Bl->last_use) { Bl->last_use = inst; }
      }
      break;
    }

    case IFMT_ABC: {
      u16 A         = I.A;
      Lifetime *Al  = li_at(&li, A);
      Al->first_use = inst;

      if (I.Bfmt == OPRFMT_SSA) {
        Lifetime *Bl = li_at(&li, I.B);
        if (inst > Bl->last_use) { Bl->last_use = inst; }
      }

      if (I.Cfmt == OPRFMT_SSA) {
        Lifetime *Cl = li_at(&li, I.C);
        if (inst > Cl->last_use) { Cl->last_use = inst; }
      }
      break;
    }

    default: unreachable();
    }
  }

  return li;
}