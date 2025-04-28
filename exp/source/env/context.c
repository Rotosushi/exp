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

#include "codegen/IR/directives.h"
#include "codegen/x86/codegen.h"
#include "codegen/x86/emit.h"
#include "codegen/x86/env/context.h"
#include "env/context.h"
#include "env/context_options.h"
#include "support/config.h"
#include "support/io.h"
#include "support/string_view.h"

#define EXP_IR_EXTENSION  "eir"
#define EXP_ASM_EXTENSION "s"
#define EXP_OBJ_EXTENSION "o"
#define EXP_EXE_EXTENSION ""
#define EXP_LIB_EXTENSION "a"

static void generate_path_from_source(String *restrict target,
                                      StringView source_path,
                                      StringView extension) {
    string_assign(target, source_path);
    string_replace_extension(target, extension);
}

void context_create(Context *restrict context,
                    ContextOptions *restrict options,
                    StringView source_path) {
    assert(context != nullptr);
    assert(options != nullptr);
    assert(!string_view_empty(source_path));
    context->options = *options;
    string_initialize(&(context->source_path));
    string_initialize(&(context->ir_path));
    string_initialize(&(context->assembly_path));
    string_initialize(&(context->object_path));
    string_initialize(&(context->executable_path));
    string_initialize(&(context->library_path));
    string_assign(&(context->source_path), source_path);
    generate_path_from_source(
        &(context->ir_path), source_path, SV(EXP_IR_EXTENSION));
    generate_path_from_source(
        &(context->assembly_path), source_path, SV(EXP_ASM_EXTENSION));
    generate_path_from_source(
        &(context->object_path), source_path, SV(EXP_OBJ_EXTENSION));
    generate_path_from_source(
        &(context->executable_path), source_path, SV(EXP_EXE_EXTENSION));
    generate_path_from_source(
        &(context->library_path), source_path, SV(EXP_LIB_EXTENSION));
    context->current_function    = nullptr;
    context->current_error       = error_create();
    context->global_symbol_table = symbol_table_create();
    // context->global_labels       = labels_create();
    context->constants       = constants_create();
    context->string_interner = string_interner_create();
    context->type_interner   = type_interner_create();
}

void context_destroy(Context *context) {
    assert(context != nullptr);
    string_destroy(&(context->source_path));
    string_destroy(&(context->ir_path));
    string_destroy(&(context->assembly_path));
    string_destroy(&(context->object_path));
    string_destroy(&(context->executable_path));
    string_destroy(&(context->library_path));
    string_interner_destroy(&(context->string_interner));
    type_interner_destroy(&(context->type_interner));
    symbol_table_destroy(&(context->global_symbol_table));
    // labels_destroy(&(context->global_labels));
    constants_destroy(&(context->constants));
    error_destroy(&context->current_error);
    context->current_function = nullptr;
}

bool context_shall_prolix(Context const *context) {
    assert(context != nullptr);
    return context->options.prolix;
}

bool context_shall_create_ir_artifact(Context const *context) {
    assert(context != nullptr);
    return context->options.create_ir_artifact;
}
bool context_shall_create_assembly_artifact(Context const *context) {
    assert(context != nullptr);
    return context->options.create_assembly_artifact;
}
bool context_shall_create_object_artifact(Context const *context) {
    assert(context != nullptr);
    return context->options.create_object_artifact;
}
bool context_shall_create_executable_artifact(Context const *context) {
    assert(context != nullptr);
    return context->options.create_executable_artifact;
}
bool context_shall_cleanup_ir_artifact(Context const *context) {
    assert(context != nullptr);
    return context->options.cleanup_ir_artifact;
}
bool context_shall_cleanup_assembly_artifact(Context const *context) {
    assert(context != nullptr);
    return context->options.cleanup_assembly_artifact;
}
bool context_shall_cleanup_object_artifact(Context const *context) {
    assert(context != nullptr);
    return context->options.cleanup_object_artifact;
}

void context_create_ir_artifact(Context *restrict context) {
    assert(context != NULL);

    String ir_path;
    string_initialize(&ir_path);
    generate_path_from_source(
        &ir_path, string_to_view(&context->source_path), SV(EXP_IR_EXTENSION));

    String contents;
    string_initialize(&contents);
    ir_directive_version(SV(EXP_VERSION_STRING), &contents);
    ir_directive_file(string_to_view(&context->source_path), &contents);

    SymbolTable *symbol_table = &context->global_symbol_table;
    for (u64 index = 0; index < symbol_table->capacity; ++index) {
        Symbol *symbol = symbol_table->elements[index];
        if (symbol == NULL) { continue; }

        if (symbol->kind == SYMBOL_KIND_FUNCTION) {
            ir_directive_function(symbol->name, &contents);
            print_function(&contents, &symbol->function_body, context);
            string_append(&contents, SV("\n"));
        }
    }

    FILE *ir_file = file_open(string_to_cstring(&ir_path), "w");
    file_write(string_to_view(&contents), ir_file);
    file_close(ir_file);
    string_destroy(&contents);
    string_destroy(&ir_path);
}

void context_create_assembly_artifact(Context *restrict context) {
    x86_Context  x86_context = x86_context_create(context);
    SymbolTable *table       = &context->global_symbol_table;

    for (u64 index = 0; index < table->capacity; ++index) {
        Symbol *element = table->elements[index];
        if (element == NULL) { continue; }
        x86_codegen_symbol(element, &x86_context);
    }

    x86_emit(&x86_context);
    x86_context_destroy(&x86_context);
}

void context_create_object_artifact(Context *restrict context);
void context_create_executable_artifact(Context *restrict context);
void context_cleanup_ir_artifact(Context *restrict context);
void context_cleanup_assembly_artifact(Context *restrict context);
void context_cleanup_object_artifact(Context *restrict context);

StringView context_source_path(Context const *restrict context) {
    assert(context != nullptr);
    return string_to_view(&(context->source_path));
}

StringView context_ir_path(Context const *context) {
    assert(context != nullptr);
    return string_to_view(&(context->ir_path));
}

StringView context_assembly_path(Context const *context) {
    assert(context != nullptr);
    return string_to_view(&context->assembly_path);
}

StringView context_object_path(Context const *context) {
    assert(context != nullptr);
    return string_to_view(&context->object_path);
}

StringView context_executable_path(Context const *context) {
    assert(context != nullptr);
    return string_to_view(&(context->executable_path));
}

Error *context_current_error(Context *context) {
    assert(context != nullptr);
    return &context->current_error;
}

bool context_has_error(Context const *context) {
    assert(context != nullptr);
    return context->current_error.code != ERROR_NONE;
}

ConstantString *context_intern(Context *context, StringView sv) {
    assert(context != nullptr);
    return string_interner_insert(&(context->string_interner), sv);
}

Type const *context_nil_type(Context *context) {
    assert(context != nullptr);
    return type_interner_nil_type(&(context->type_interner));
}

Type const *context_boolean_type(Context *context) {
    assert(context != nullptr);
    return type_interner_boolean_type(&(context->type_interner));
}

Type const *context_u8_type(Context *context) {
    assert(context != nullptr);
    return type_interner_u8_type(&(context->type_interner));
}

Type const *context_u16_type(Context *context) {
    assert(context != nullptr);
    return type_interner_u16_type(&(context->type_interner));
}

Type const *context_u32_type(Context *context) {
    assert(context != nullptr);
    return type_interner_u32_type(&(context->type_interner));
}

Type const *context_u64_type(Context *context) {
    assert(context != nullptr);
    return type_interner_u64_type(&(context->type_interner));
}

Type const *context_i8_type(Context *context) {
    assert(context != nullptr);
    return type_interner_i8_type(&(context->type_interner));
}

Type const *context_i16_type(Context *context) {
    assert(context != nullptr);
    return type_interner_i16_type(&(context->type_interner));
}

Type const *context_i32_type(Context *context) {
    assert(context != nullptr);
    return type_interner_i32_type(&(context->type_interner));
}

Type const *context_i64_type(Context *context) {
    assert(context != nullptr);
    return type_interner_i64_type(&(context->type_interner));
}

Type const *context_tuple_type(Context *context, TupleType tuple) {
    assert(context != nullptr);
    return type_interner_tuple_type(&context->type_interner, tuple);
}

Type const *context_function_type(Context    *context,
                                  Type const *return_type,
                                  TupleType   argument_types) {
    assert(context != nullptr);
    return type_interner_function_type(
        &context->type_interner, return_type, argument_types);
}

// u32 context_labels_insert(Context *context, StringView symbol) {
//     assert(context != nullptr);
//     return labels_insert(&context->global_labels, symbol);
// }

// StringView context_labels_at(Context *context, u32 index) {
//     assert(context != nullptr);
//     return labels_at(&context->global_labels, index);
// }

Symbol *context_global_symbol_table_at(Context *context, StringView name) {
    assert(context != nullptr);
    return symbol_table_at(&context->global_symbol_table, name);
}

Function *context_enter_function(Context *c, StringView name) {
    assert(c != nullptr);
    Symbol *element = symbol_table_at(&c->global_symbol_table, name);
    if (element->kind == SYMBOL_KIND_UNDEFINED) {
        element->kind = SYMBOL_KIND_FUNCTION;
    }

    c->current_function = &element->function_body;
    return c->current_function;
}

Function *context_current_function(Context *c) {
    assert(c != nullptr);
    assert(c->current_function != nullptr);
    return c->current_function;
}

Bytecode *context_active_bytecode(Context *c) {
    return &(context_current_function(c)->bc);
}

Local *context_declare_argument(Context *c) {
    assert(c != nullptr);
    assert(c->current_function != nullptr);
    return function_declare_argument(c->current_function);
}

Local *context_declare_local(Context *c) {
    assert(c != nullptr);
    assert(c->current_function != nullptr);
    return function_declare_local(c->current_function);
}

Local *context_lookup_argument(Context *c, u8 index) {
    assert(c != nullptr);
    assert(c->current_function != nullptr);
    return function_lookup_argument(c->current_function, index);
}

Local *context_lookup_local(Context *c, u32 ssa) {
    assert(c != nullptr);
    assert(c->current_function != nullptr);
    return function_lookup_local(c->current_function, ssa);
}

Local *context_lookup_local_name(Context *c, StringView name) {
    assert(c != nullptr);
    assert(c->current_function != nullptr);
    return function_lookup_local_name(c->current_function, name);
}

void context_leave_function(Context *c) {
    assert(c != nullptr);
    c->current_function = nullptr;
}

Operand context_constants_append(Context *context, Value value) {
    assert(context != nullptr);
    return constants_append(&(context->constants), value);
}

Value *context_constants_at(Context *context, u32 index) {
    assert(context != nullptr);
    return constants_at(&(context->constants), index);
}

void context_emit_return(Context *c, Operand B) {
    assert(c != nullptr);
    bytecode_append(context_active_bytecode(c), instruction_return(B));
}

Operand context_emit_call(Context *c, Operand B, Operand C) {
    assert(c != nullptr);
    Operand A = operand_ssa(context_declare_local(c)->ssa);
    bytecode_append(context_active_bytecode(c), instruction_call(A, B, C));
    return A;
}

Operand context_emit_dot(Context *c, Operand B, Operand C) {
    assert(c != nullptr);
    Operand A = operand_ssa(context_declare_local(c)->ssa);
    bytecode_append(context_active_bytecode(c), instruction_dot(A, B, C));
    return A;
}

Operand context_emit_load(Context *c, Operand B) {
    assert(c != nullptr);
    Operand A = operand_ssa(context_declare_local(c)->ssa);
    bytecode_append(context_active_bytecode(c), instruction_let(A, B));
    return A;
}

Operand context_emit_negate(Context *c, Operand B) {
    assert(c != nullptr);
    Operand A = operand_ssa(context_declare_local(c)->ssa);
    bytecode_append(context_active_bytecode(c), instruction_neg(A, B));
    return A;
}

Operand context_emit_add(Context *c, Operand B, Operand C) {
    assert(c != nullptr);
    Operand A = operand_ssa(context_declare_local(c)->ssa);
    bytecode_append(context_active_bytecode(c), instruction_add(A, B, C));
    return A;
}

Operand context_emit_subtract(Context *c, Operand B, Operand C) {
    assert(c != nullptr);
    Operand A = operand_ssa(context_declare_local(c)->ssa);
    bytecode_append(context_active_bytecode(c), instruction_sub(A, B, C));
    return A;
}

Operand context_emit_multiply(Context *c, Operand B, Operand C) {
    assert(c != nullptr);
    Operand A = operand_ssa(context_declare_local(c)->ssa);
    bytecode_append(context_active_bytecode(c), instruction_mul(A, B, C));
    return A;
}

Operand context_emit_divide(Context *c, Operand B, Operand C) {
    assert(c != nullptr);
    Operand A = operand_ssa(context_declare_local(c)->ssa);
    bytecode_append(context_active_bytecode(c), instruction_div(A, B, C));
    return A;
}

Operand context_emit_modulus(Context *c, Operand B, Operand C) {
    assert(c != nullptr);
    Operand A = operand_ssa(context_declare_local(c)->ssa);
    bytecode_append(context_active_bytecode(c), instruction_mod(A, B, C));
    return A;
}
