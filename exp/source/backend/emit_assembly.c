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

#include "backend/compute_allocations.h"
#include "backend/compute_lifetimes.h"
#include "backend/emit_assembly.h"

/*
  1 - compute lifetime intervals
  2 - allocate SSA locals to registers or the stack
  3 - select instructions
  4 - write file
*/
static i32 emit_global_symbol(SymbolTableElement *symbol) {
  FunctionBody *body = &symbol->function_body;

  Lifetimes lifetimes     = compute_lifetimes(body);
  Allocations allocations = compute_allocations(&lifetimes);

  lifetimes_destroy(&lifetimes);
  allocations_destroy(&allocations);
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