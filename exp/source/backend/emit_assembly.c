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
 * along with exp.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <assert.h>
#include <stdlib.h>

#include "backend/emit_assembly.h"
#include "backend/lifetime_intervals.h"
#include "utility/alloc.h"
#include "utility/panic.h"

// walk the bytecode representing the function body.
// if an instruction assigns a value to a SSA local
// that is the first use of the SSA local.
// The last use is the last instruction which uses
// a SSA local as an operand.
// we want to compute last use in a single pass of the bytecode
// and ideally in the same pass as first use
// We can simply update the last use each time we
// see the local used.
// This solution, while correct, does a lot of overwriting.
// and each overwrite must first search the array to find
// the interval.
// so there is lots of room for improvement.
// we could refactor to keep a buffer sorted by local number.
// then updating last use doesn't need to search. but still
// overwrites. And this introduces a 'sort' step at the end.

// oh dang, if we walk backwards we get last use for free,
// because the first time we see a local used is by definition
// the latest time in the bytecode it is used.
// and the first time we see a local in position A is by definition
// the first use.
// so how do we add this information to an array efficiently?
// because a local can be used multiple times, so we only want
// to add a new lifetime if a lifetime doesn't already exist.
// which means when we add a lifetime we need to search for
// an existing one first.
// another useful fact is that by walking backwards we come
// accross the last local introduced first, which is by definition
// also the total number of locals used by the function body.
// once we know the total number of locals used, we know exactly
// how many lifetimes to create. this means we can use some scratch
// space to build up the information, then create the final
// sorted lifetime intervals afterwards.

// only set the last use of the given local if the given last use
// is greater than the existing last use.
static void set_last_use(Interval *restrict i, u16 last_use) {
  if (last_use > i->last_use) {
    i->last_use = last_use;
  }
}

// since we can only encounter the first use of any given local once
// we can always set the first use in the interval.
static void set_first_use(Interval *restrict i, u16 local, u16 first_use) {
  i->local     = local;
  i->first_use = first_use;
}

static LifetimeIntervals
compute_lifetime_intervals_impl(Bytecode *restrict bc,
                                Interval *restrict scratch, u16 num_locals) {
  // walk backwards through the bytecode building up the
  // lifetime intervals as we go.
  // since we are walking backwards we know two things
  //   1 - the first time we see a local used must be the last use
  //   2 - the first time we see a local assigned must be the first use
  // the complication is that since a local can be used multiple times
  // we have to be sure that we are not overwriting the last use
  // when we encounter a use of that same local after writing last use
  for (u16 idx = bc->length; idx > 0; --idx) {
    u16 i         = idx - 1;
    Instruction I = bc->buffer[i];
    switch (INST_FORMAT(I)) {
    case FORMAT_B: {
      OperandFormat Bfmt = INST_B_FORMAT(I);
      u16 B              = INST_B(I);
      if (Bfmt == FORMAT_LOCAL) {
        set_last_use(scratch + B, i);
      }
      break;
    }

    case FORMAT_AB: {
      u16 A = INST_A(I);
      set_first_use(scratch + A, A, i);

      OperandFormat Bfmt = INST_B_FORMAT(I);
      u16 B              = INST_B(I);
      if (Bfmt == FORMAT_LOCAL) {
        set_last_use(scratch + B, i);
      }
      break;
    }

    case FORMAT_ABC: {
      u16 A = INST_A(I);
      set_first_use(scratch + A, A, i);

      OperandFormat Bfmt = INST_B_FORMAT(I);
      u16 B              = INST_B(I);
      if (Bfmt == FORMAT_LOCAL) {
        set_last_use(scratch + B, i);
      }

      OperandFormat Cfmt = INST_C_FORMAT(I);
      u16 C              = INST_C(I);
      if (Cfmt == FORMAT_LOCAL) {
        set_last_use(scratch + C, i);
      }
      break;
    }

    default:
      unreachable();
    }
  }

  LifetimeIntervals li = lifetime_intervals_create();
  for (u16 i = 0; i < num_locals; ++i) {
    lifetime_intervals_insert_sorted(&li, scratch[i]);
  }
  return li;
}

static LifetimeIntervals compute_lifetime_intervals_stack(Bytecode *restrict bc,
                                                          u16 num_locals) {
  Interval scratch[num_locals] = {};
  return compute_lifetime_intervals_impl(bc, scratch, num_locals);
}

static LifetimeIntervals compute_lifetime_intervals_heap(Bytecode *restrict bc,
                                                         u16 num_locals) {
  Interval *scratch = allocate(num_locals * sizeof(Interval));
  LifetimeIntervals lifetimes =
      compute_lifetime_intervals_impl(bc, scratch, num_locals);
  free(scratch);
  return lifetimes;
}

static LifetimeIntervals
compute_lifetime_intervals(FunctionBody *restrict body) {
  // find the total number of locals used by this function
  Bytecode *bc = &body->bc;
  // walk backwards until we see the first A type instruction
  for (u16 idx = bc->length; idx > 0; --idx) {
    // bc->length == 1, I = bc->buffer[0],
    // bc->length == 2, I = bc->buffer[1],
    // etc.
    Instruction I = bc->buffer[idx - 1];
    switch (INST_FORMAT(I)) {
    case FORMAT_AB:
    case FORMAT_ABC: {
      // A type instructions always introduce a new SSA local
      // so the first A type instruction we encounter while walking
      // backwards must be the last SSA local introduced.
      u16 A = INST_A(I);
      // thus the number of locals within a function is equal to
      // the "name" of the last SSA local introduced plus one.
      // (as SSA locals are zero indexed.)
      u16 num_locals;
      if (__builtin_add_overflow(A, 1, &num_locals)) {
        PANIC("num_locals > u16_MAX");
      }
      // we need to allocate scratch space to speed up the
      // algorithm. so we need to choose where to allocate
      // that scratch space, on the stack or the heap.
      // stack space is less available than heap space,
      // so I don't think its a good idea to allocate
      // u16_MAX Intervals on the stack.
      // we use a simple heuristic here to choose.
      // #TODO what is the best number to use here?
      if (num_locals < 11) {
        return compute_lifetime_intervals_stack(bc, num_locals);
      } else {
        return compute_lifetime_intervals_heap(bc, num_locals);
      }
    }

    case FORMAT_B:
    default:
      break;
    }
  }

  return lifetime_intervals_create();
}

/*
  1 - compute lifetime intervals
  2 - allocate registers
  3 - select instructions
  4 - write file
*/
static i32 emit_global_symbol(SymbolTableElement *symbol) {
  LifetimeIntervals lifetimes =
      compute_lifetime_intervals(&symbol->function_body);

  lifetime_intervals_destroy(&lifetimes);
  return EXIT_SUCCESS;
}

i32 emit_assembly(Context *restrict context) {
  assert(context != NULL);
  i32 result = EXIT_SUCCESS;

  SymbolTableIterator iter = context_global_symbol_iterator(context);
  while (!symbol_table_iterator_done(&iter)) {
    SymbolTableElement *symbol = iter.element;

    result |= emit_global_symbol(symbol);

    symbol_table_iterator_next(&iter);
  }

  return EXIT_SUCCESS;
}