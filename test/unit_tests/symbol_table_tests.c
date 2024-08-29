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
#include <stdlib.h>
#include <time.h>

#include "env/symbol_table.h"

bool test_symbol_table(SymbolTable *restrict symbol_table, char const *name) {
  bool failure = 0;

  StringView sv               = string_view_from_cstring(name);
  SymbolTableElement *element = symbol_table_at(symbol_table, sv);

  if (element == NULL) return 1;

  failure |= !string_view_eq(element->name, sv);

  return failure;
}

i32 symbol_table_tests([[maybe_unused]] i32 argc,
                       [[maybe_unused]] char *argv[]) {
  srand((unsigned)time(NULL));
  bool failure             = 0;
  SymbolTable symbol_table = symbol_table_create();

  failure |= test_symbol_table(&symbol_table, "foo");
  failure |= test_symbol_table(&symbol_table, "bar");
  failure |= test_symbol_table(&symbol_table, "foobar");
  failure |= test_symbol_table(&symbol_table, "abc");
  failure |= test_symbol_table(&symbol_table, "bca");
  failure |= test_symbol_table(&symbol_table, "cab");
  failure |= test_symbol_table(&symbol_table, "acb");
  failure |= test_symbol_table(&symbol_table, "cba");
  failure |= test_symbol_table(&symbol_table, "bac");

  symbol_table_destroy(&symbol_table);
  if (failure) {
    return EXIT_FAILURE;
  } else {
    return EXIT_SUCCESS;
  }
}

