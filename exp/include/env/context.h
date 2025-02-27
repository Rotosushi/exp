// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
#ifndef EXP_ENV_CONTEXT_H
#define EXP_ENV_CONTEXT_H

#include "env/constants.h"
#include "env/context_options.h"
#include "env/error.h"
#include "env/labels.h"
#include "env/registers.h"
#include "env/stack.h"
#include "env/string_interner.h"
#include "env/symbol_table.h"
#include "env/type_interner.h"

/**
 * @brief A context holds the common information needed
 * by the compiler for interpretation.
 *
 */
typedef struct Context {
    ContextOptions options;
    Error current_error;
    StringInterner string_interner;
    TypeInterner type_interner;
    SymbolTable symbol_table;
    Labels labels;
    Constants constants;
    Stack stack;
    Registers registers;
} Context;

/**
 * @brief create a new options
 *
 * @note takes ownership of the options passed in.
 *
 * @param options
 * @return Context
 */
void context_initialize(Context *context, Bitset flags, StringView source,
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
StringView context_intern(Context *context, StringView sv);

// type interner functions
Type const *context_nil_type(Context *context);
Type const *context_boolean_type(Context *context);
Type const *context_i8_type(Context *context);
Type const *context_i16_type(Context *context);
Type const *context_i32_type(Context *context);
Type const *context_i64_type(Context *context);
Type const *context_u8_type(Context *context);
Type const *context_u16_type(Context *context);
Type const *context_u32_type(Context *context);
Type const *context_u64_type(Context *context);
Type const *context_tuple_type(Context *context, TupleType tuple);
Type const *context_function_type(Context *context, Type const *return_type,
                                  TupleType argument_types);

// symbol table functions
Symbol *context_symbol_table_at(Context *context, StringView name);

// labels functions
u32 context_labels_append(Context *context, StringView label);
StringView context_labels_at(Context *context, u32 label);

// Values functions
Value *context_constants_at(Context *context, u32 constant);
u32 context_constants_append_tuple(Context *context, Tuple tuple);

// stack functions
u32 context_stack_length(Context *context);
Value *context_stack_top(Context *context);
Value *context_stack_peek(Context *context, u32 n);
void context_stack_push(Context *context, Value value);
Value context_stack_pop(Context *context);
void context_stack_pop_n(Context *context, u32 n);

// registers functions
bool context_registers_next_available(Context *context, u8 *register_);

void context_registers_set(Context *context, u8 register_, Scalar value);
Scalar context_registers_get(Context *context, u8 register_);
Scalar context_registers_unset(Context *context, u8 register_);

#endif // !EXP_ENV_CONTEXT_H
