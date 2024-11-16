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

#include "env/context_options.h"
#include "env/global_labels.h"
#include "env/string_interner.h"
#include "env/symbol_table.h"
#include "env/type_interner.h"
#include "env/values.h"
#include "imr/function_body.h"

/**
 * @brief A context models a Translation Unit.
 *
 */
typedef struct Context {
  ContextOptions options;
  StringInterner string_interner;
  TypeInterner type_interner;
  SymbolTable global_symbol_table;
  GlobalLabels global_labels;
  FunctionBody *current_function;
  Values values;
} Context;

/**
 * @brief create a new options
 *
 * @note takes ownership of the options passed in.
 *
 * @param options
 * @return Context
 */
Context context_create(CLIOptions *restrict options);
void context_destroy(Context *restrict context);

// context options functions
bool context_do_assemble(Context *restrict context);
bool context_do_link(Context *restrict context);
bool context_do_cleanup(Context *restrict context);

StringView context_source_path(Context *restrict context);
StringView context_assembly_path(Context *restrict context);
StringView context_object_path(Context *restrict context);
StringView context_output_path(Context *restrict context);

// string interner functions
StringView context_intern(Context *restrict context, StringView sv);

// type interner functions
Type *context_nil_type(Context *restrict context);
Type *context_boolean_type(Context *restrict context);
Type *context_i64_type(Context *restrict context);
Type *context_tuple_type(Context *restrict context, TupleType tuple);
Type *context_function_type(Context *restrict context,
                            Type *return_type,
                            TupleType argument_types);

// global labels functions
u16 context_global_labels_insert(Context *restrict context, StringView symbol);
StringView context_global_labels_at(Context *restrict context, u16 index);

// symbol table functions
SymbolTableElement *context_global_symbol_table_at(Context *restrict context,
                                                   StringView name);

SymbolTableIterator
context_global_symbol_table_iterator(Context *restrict context);

// function functions
FunctionBody *context_enter_function(Context *restrict c, StringView name);
FunctionBody *context_current_function(Context *restrict c);
Bytecode *context_active_bytecode(Context *restrict c);

// CallPair context_new_call(Context *restrict c);
// ActualArgumentList *context_call_at(Context *restrict c, u64 idx);

void context_def_local_const(Context *restrict c,
                             StringView name,
                             Operand value);

LocalVariable *context_lookup_local(Context *restrict c, StringView name);
LocalVariable *context_lookup_ssa(Context *restrict c, u16 ssa);

FormalArgument *context_lookup_argument(Context *restrict c, StringView name);
FormalArgument *context_argument_at(Context *restrict c, u8 index);

void context_leave_function(Context *restrict c);

// Values functions
Operand context_values_append(Context *restrict context, Value value);
Value *context_values_at(Context *restrict context, u16 index);

// Bytecode functions
void context_emit_return(Context *restrict c, Operand B);
Operand context_emit_call(Context *restrict c, Operand B, Operand C);
Operand context_emit_dot(Context *restrict c, Operand B, Operand C);
Operand context_emit_load(Context *restrict c, Operand B);
Operand context_emit_neg(Context *restrict c, Operand B);
Operand context_emit_add(Context *restrict c, Operand B, Operand C);
Operand context_emit_sub(Context *restrict c, Operand B, Operand C);
Operand context_emit_mul(Context *restrict c, Operand B, Operand C);
Operand context_emit_div(Context *restrict c, Operand B, Operand C);
Operand context_emit_mod(Context *restrict c, Operand B, Operand C);

#endif // !EXP_ENV_CONTEXT_H
