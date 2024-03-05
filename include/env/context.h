// Copyright (C) 2024 Cade Weinberg
//
// This file is part of exp.
//
// exp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// exp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with exp.  If not, see <http://www.gnu.org/licenses/>.
#ifndef EXP_ENV_CONTEXT_H
#define EXP_ENV_CONTEXT_H

#include "env/options.h"
#include "env/string_interner.h"
#include "env/symbol_table.h"
#include "env/type_interner.h"
// #include "imr/bytecode.h"
#include "imr/constants.h"
#include "imr/stack.h"
// #include "imr/registers.h"

typedef struct Context {
  Options options;
  StringInterner string_interner;
  TypeInterner type_interner;
  SymbolTable global_symbols;
  Constants constants;
  Stack stack;
  //  Bytecode global_bytecode;
  //  Registers registers;
} Context;

Context context_create(CLIOptions *restrict options);
void context_destroy(Context *restrict context);

StringView context_source_path(Context *restrict context);

StringView context_intern(Context *restrict context, char const *data,
                          size_t length);

Type *context_integer_type(Context *restrict context);

bool context_insert_global(Context *restrict context, StringView name,
                           Type *type, Value value);

SymbolTableElement *context_lookup_global(Context *restrict context,
                                          StringView name);

#endif // !EXP_ENV_CONTEXT_H