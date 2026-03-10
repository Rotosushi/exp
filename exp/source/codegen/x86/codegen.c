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

#include "codegen/x86/codegen.h"
#include "codegen/GAS/directives.h"
#include "imr/type.h"
#include "imr/type/composite.h"
#include "imr/value.h"
#include "support/assert.h"
#include "support/config.h"
#include "support/unreachable.h"

/*
 * #TODO:
 *  a popular replacement for this handrolled backend is to generate
 *  assembly based on some form of x86-64 specification language.
 *  which if done well, can allow other backends to be written only
 *  by adding a specification of them. this works in LLVM via TableGen,
 *  Which to my understanding generates a generator. and the machanism
 *  for generation is some form of Graph Covering.
 */

void x86_compile_symbol(Symbol *restrict symbol, Context *restrict context) {
    EXP_ASSERT(symbol != NULL);
    EXP_ASSERT(context != NULL);
    x86_Context *x86_context = context_get_target_context(context);
    StringView   name        = symbol->name;
    Value const *value       = symbol->value;
    Type const  *type        = symbol->type;

    x86_Symbol *x86_symbol = x86_context_symbol_table_at(x86_context, name);
    x86_symbol->type       = type;

    switch (value->kind) {
    case VALUE_KIND_UNINITIALIZED: {
        x86_symbol->value =
            x86_context_constant_uninitialized(x86_context, type);
        break;
    }
    }
}

static void x86_print_assembly_header(String *restrict buffer,
                                      Context *restrict context);
static void x86_print_assembly_footer(String *restrict buffer,
                                      Context *restrict context);
static void x86_print_assembly_primary(String *restrict buffer,
                                       x86_Symbol const *restrict symbol);
static void x86_print_assembly_function(String *restrict buffer,
                                        x86_Symbol const *restrict symbol);
static void x86_print_assembly_composite(String *restrict buffer,
                                         x86_Symbol const *restrict symbol);
static void x86_print_assembly_symbol(String *restrict buffer,
                                      x86_Symbol const *restrict symbol);

void x86_print_assembly(String *restrict buffer, Context *restrict context) {
    EXP_ASSERT(buffer != NULL);
    EXP_ASSERT(context != NULL);

    x86_Context *x86_context =
        (x86_Context *)context_get_target_context(context);

    x86_print_assembly_header(buffer, context);

    x86_SymbolTable *symbols = &x86_context->symbols;
    for (u32 i = 0; i < symbols->capacity; ++i) {
        x86_Symbol *symbol = symbols->elements[i];
        if (symbol == NULL) { continue; }

        x86_print_assembly_symbol(buffer, symbol);
    }

    x86_print_assembly_footer(buffer, context);
}

static void x86_print_assembly_header(String *restrict buffer,
                                      Context *restrict context) {
    EXP_ASSERT(buffer != NULL);
    EXP_ASSERT(context != NULL);
    gas_directive_file(context_source_path(context), buffer);
    // #TODO: this needs to come from the arch specific section of the target.
    // else we leave it blank.
    gas_directive_arch(SV("znver3"), buffer);
    gas_directive_intel_syntax(buffer);
    string_append(buffer, SV("\n"));
}

static void
x86_print_assembly_footer(String *restrict buffer,
                          [[maybe_unused]] Context *restrict context) {
    EXP_ASSERT(buffer != NULL);
    EXP_ASSERT(context != NULL);
    gas_directive_noexecstack(buffer);
    gas_directive_ident(SV(EXP_VERSION_STRING), buffer);
}

static void x86_print_assembly_primary(String *restrict buffer,
                                       x86_Symbol const *restrict symbol) {
    EXP_ASSERT(buffer != NULL);
    EXP_ASSERT(symbol != NULL);
    // EXP_ASSERT(context != NULL);
    u64 size      = layout_size_of(symbol->type->layout);
    u64 alignment = layout_align_of(symbol->type->layout);
    gas_directive_data(buffer);
    gas_directive_globl(symbol->name, buffer);
    gas_directive_balign(alignment, buffer);
    gas_directive_type(symbol->name, STT_OBJECT, buffer);
    gas_directive_size(symbol->name, size, buffer);
    gas_directive_label(symbol->name, buffer);
    x86_value_print(buffer, symbol->value);
}

static void x86_print_assembly_function(String *restrict buffer,
                                        x86_Symbol const *restrict symbol) {
    EXP_ASSERT(buffer != NULL);
    EXP_ASSERT(symbol != NULL);
    // EXP_ASSERT(context != NULL);
    u64 alignment = layout_align_of(symbol->type->layout);
    gas_directive_text(buffer);
    gas_directive_globl(symbol->name, buffer);
    gas_directive_balign(alignment, buffer);
    gas_directive_type(symbol->name, STT_FUNC, buffer);
    gas_directive_label(symbol->name, buffer);
    x86_value_print(buffer, symbol->value);
    gas_directive_size_label_relative(symbol->name, buffer);
}

static void x86_print_assembly_composite(String *restrict buffer,
                                         x86_Symbol const *restrict symbol) {
    EXP_ASSERT(buffer != NULL);
    EXP_ASSERT(symbol != NULL);
    x86_Value const     *value     = symbol->value;
    Type const          *type      = value->type;
    TypeComposite const *composite = &type->composite;
    switch (composite->kind) {
    case TYPE_COMPOSITE_KIND_TUPLE:
        x86_print_assembly_primary(buffer, symbol);
        break;

    case TYPE_COMPOSITE_KIND_FUNCTION:
        x86_print_assembly_function(buffer, symbol);
        break;

    default: EXP_UNREACHABLE();
    }
}

static void x86_print_assembly_symbol(String *restrict buffer,
                                      x86_Symbol const *restrict symbol) {
    EXP_ASSERT(buffer != NULL);
    EXP_ASSERT(symbol != NULL);
    x86_Value const *value = symbol->value;
    Type const      *type  = value->type;
    switch (type->kind) {
    case TYPE_KIND_PRIMARY: x86_print_assembly_primary(buffer, symbol); break;

    case TYPE_KIND_COMPOSITE:
        x86_print_assembly_composite(buffer, symbol);
        break;

    default: EXP_UNREACHABLE();
    }
}
