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

#include "env/call_stack.h"
#include "env/constants.h"
#include "env/context_options.h"
#include "env/locals.h"
#include "env/string_interner.h"
#include "env/symbol_table.h"
#include "env/type_interner.h"
#include "imr/function_body.h"
#include "imr/operand.h"

/**
 * @brief A context models a Translation Unit.
 *
 */
typedef struct Context {
  ContextOptions options;
  StringInterner string_interner;
  TypeInterner type_interner;
  SymbolTable global_symbols;
  CallStack call_stack;
  Locals locals;
  Constants constants;
} Context;

/**
 * @brief create a new options
 *
 * @note takes ownership of the options passed in.
 *
 * @param options
 * @return Context
 */
Context context_create(ContextOptions *restrict options);
void context_destroy(Context *restrict context);

// context options functions
StringView context_source_path(Context *restrict context);

FILE *context_open_source(Context *restrict context);

String context_buffer_source(Context *restrict context);

StringView context_output_path(Context *restrict context);

FILE *context_open_output(Context *restrict context);

// string interner functions
StringView context_intern(Context *restrict context, StringView sv);

// type interner functions
Type *context_nil_type(Context *restrict context);

Type *context_boolean_type(Context *restrict context);

Type *context_i64_type(Context *restrict context);

Type *context_function_type(Context *restrict context, Type *return_type,
                            ArgumentTypes argument_types);

// symbol table functions
SymbolTableElement *context_global_symbols_at(Context *restrict context,
                                              StringView name);

// function functions
CallFrame context_push_function(Context *restrict c, StringView name);

void context_pop_function(Context *restrict c);

CallFrame context_active_frame(Context *restrict c);

// Locals functions
Operand context_new_local(Context *restrict c);

Value *context_local_at(Context *restrict c, Operand operand);

// Constants functions
Operand context_constants_add(Context *restrict context, Value value);

Value *context_constants_at(Context *restrict context, u16 index);

// emit instruction functions
Operand context_emit_move(Context *restrict c, Operand B);

Operand context_emit_neg(Context *restrict c, Operand B);

Operand context_emit_add(Context *restrict c, Operand B, Operand C);
Operand context_emit_sub(Context *restrict c, Operand B, Operand C);
Operand context_emit_mul(Context *restrict c, Operand B, Operand C);
Operand context_emit_div(Context *restrict c, Operand B, Operand C);
Operand context_emit_mod(Context *restrict c, Operand B, Operand C);

void context_emit_return(Context *restrict c, Operand B);
#endif // !EXP_ENV_CONTEXT_H