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
    TypeInterner type_interner;
    SymbolTable symbol_table;
    Labels labels;
    Constants constants;
    Error current_error;
} Context;

/**
 * @brief create a new options
 *
 * @note takes ownership of the options passed in.
 *
 * @param options
 * @return Context
 */
void context_initialize(Context *context,
                        Bitset flags,
                        StringView source,
                        StringView output);
void context_terminate(Context *context);

// context options functions
bool context_emit_ir_assembly(Context *context);
bool context_emit_x86_64_assembly(Context *context);

bool context_create_elf_object(Context *context);
bool context_create_elf_executable(Context *context);

bool context_cleanup_x86_64_assembly(Context *context);
bool context_cleanup_elf_object(Context *context);

StringView context_source_path(Context *context);
StringView context_ir_path(Context *context);
StringView context_assembly_path(Context *context);
StringView context_object_path(Context *context);
StringView context_output_path(Context *context);

// current error functions
Error *context_current_error(Context *context);
bool context_has_error(Context *context);

// string interner functions
ConstantString *context_intern(Context *context, StringView sv);

// type interner functions
// Type const *context_nil_type(Context *context);
// Type const *context_boolean_type(Context *context);
Type const *context_i32_type(Context *context);
Type const *context_tuple_type(Context *context, TupleType tuple);
Type const *context_function_type(Context *context,
                                  Type const *return_type,
                                  TupleType argument_types);

// symbol table functions
Symbol *context_symbol_table_at(Context *context, StringView name);

u32 context_labels_append(Context *context, StringView label);
StringView context_labels_at(Context *context, u32 label);

// Values functions
Value *context_constants_at(Context *context, u32 constant);
u32 context_constants_append_tuple(Context *context, Tuple tuple);

#endif // !EXP_ENV_CONTEXT_H
