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

#include "env/context.h"
#include "imr/bytecode.h"
#include "utility/alloc.h"
#include "utility/array_growth.h"

Bytecode bytecode_create() {
  Bytecode bc;
  bc.length   = 0;
  bc.capacity = 0;
  bc.buffer   = NULL;
  return bc;
}

void bytecode_destroy(Bytecode *restrict bytecode) {
  assert(bytecode != NULL);
  bytecode->length   = 0;
  bytecode->capacity = 0;
  deallocate(bytecode->buffer);
  bytecode->buffer = NULL;
}

u64 bytecode_current_index(Bytecode *restrict bytecode) {
  return bytecode->length;
}

static bool bytecode_full(Bytecode *restrict bytecode) {
  return bytecode->capacity <= (bytecode->length + 1);
}

static void bytecode_grow(Bytecode *restrict bytecode) {
  Growth g         = array_growth_u64(bytecode->capacity, sizeof(Instruction));
  bytecode->buffer = reallocate(bytecode->buffer, g.alloc_size);
  bytecode->capacity = g.new_capacity;
}

void bytecode_append(Bytecode *restrict bytecode, Instruction I) {
  if (bytecode_full(bytecode)) { bytecode_grow(bytecode); }

  bytecode->buffer[bytecode->length] = I;
  bytecode->length += 1;
}

void print_bytecode(Bytecode const *restrict bc,
                    String *restrict out,
                    Context *restrict context) {
  // walk the entire buffer and print each instruction
  for (u64 i = 0; i < bc->length; ++i) {
    string_append(out, SV("  "));
    string_append_u64(out, i);
    string_append(out, SV(": "));
    print_instruction(bc->buffer[i], out, context);
    string_append(out, SV("\n"));
  }
}
