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

#include "env/constants.h"
#include "env/context_options.h"
#include "env/error.h"
#include "env/labels.h"
#include "env/string_interner.h"
#include "env/symbol_table.h"
#include "env/type_interner.h"

/**
 * @brief A context models a Translation Unit.
 *
 */
typedef struct Context {
    ContextOptions options;
    StringInterner string_interner;
    TypeInterner   type_interner;
    SymbolTable    global_symbol_table;
    Labels         global_labels;
    Constants      constants;
    Error          current_error;
    Function      *current_function;
} Context;

/**
 * @brief create a new options
 *
 * @note takes ownership of the options passed in.
 *
 * @param options
 * @return Context
 */
Context context_create(CLIOptions *options);
void    context_destroy(Context *context);

// context options functions
bool context_do_assemble(Context *context);
bool context_do_link(Context *context);
bool context_do_cleanup(Context *context);

StringView context_source_path(Context *context);
StringView context_assembly_path(Context *context);
StringView context_object_path(Context *context);
StringView context_output_path(Context *context);

// current error functions
Error *context_current_error(Context *context);
bool   context_has_error(Context *context);

// string interner functions
StringView context_intern(Context *context, StringView sv);

// type interner functions
Type *context_nil_type(Context *context);
Type *context_boolean_type(Context *context);
Type *context_i64_type(Context *context);
Type *context_tuple_type(Context *context, TupleType tuple);
Type *context_function_type(Context  *context,
                            Type     *return_type,
                            TupleType argument_types);

// labels functions
u32        context_labels_insert(Context *context, StringView symbol);
StringView context_labels_at(Context *context, u32 index);

// symbol table functions
Symbol *context_global_symbol_table_at(Context *context, StringView name);

SymbolTableIterator context_global_symbol_table_iterator(Context *context);

// function functions
Function *context_enter_function(Context *c, StringView name);
Function *context_current_function(Context *c);
Bytecode *context_active_bytecode(Context *c);

// CallPair context_new_call(Context * c);
// ActualArgumentList *context_call_at(Context * c, u64 idx);

void context_def_local_const(Context *c, StringView name, Operand value);

LocalVariable *context_lookup_local(Context *c, StringView name);
LocalVariable *context_lookup_ssa(Context *c, u32 ssa);

FormalArgument *context_lookup_argument(Context *c, StringView name);
FormalArgument *context_argument_at(Context *c, u8 index);

void context_leave_function(Context *c);

// Values functions
Operand context_constants_append(Context *context, Value value);
Value  *context_constants_at(Context *context, u32 index);

// Bytecode functions
void    context_emit_return(Context *c, Operand B);
Operand context_emit_call(Context *c, Operand B, Operand C);
Operand context_emit_dot(Context *c, Operand B, Operand C);
Operand context_emit_load(Context *c, Operand B);
Operand context_emit_negate(Context *c, Operand B);
Operand context_emit_add(Context *c, Operand B, Operand C);
Operand context_emit_subtract(Context *c, Operand B, Operand C);
Operand context_emit_multiply(Context *c, Operand B, Operand C);
Operand context_emit_divide(Context *c, Operand B, Operand C);
Operand context_emit_modulus(Context *c, Operand B, Operand C);

#endif // !EXP_ENV_CONTEXT_H
