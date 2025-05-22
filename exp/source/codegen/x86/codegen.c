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
#include "codegen/x86/env/context.h"
#include "codegen/x86/function.h"
#include "codegen/x86/value.h"
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

i32 x86_header(String *restrict buffer, Context *restrict context) {
    exp_assert(buffer != NULL);
    exp_assert(context != NULL);
    gas_directive_file(context_source_path(context), buffer);
    // #TODO: this needs to come from the arch specific section of the target.
    // else we leave it blank.
    gas_directive_arch(SV("znver3"), buffer);
    gas_directive_intel_syntax(buffer);
    string_append(buffer, SV("\n"));
    return 0;
}

i32 x86_codegen(String *restrict buffer,
                Symbol const *restrict symbol,
                Context *restrict context) {
    exp_assert(buffer != NULL);
    exp_assert(context != NULL);

    switch (symbol->type->kind) {
    // text section
    case TYPE_KIND_FUNCTION:
        gas_directive_text(buffer);
        gas_directive_globl(symbol->name, buffer);
        gas_directive_balign(8, buffer);
        gas_directive_type(symbol->name, STT_FUNC, buffer);
        gas_directive_label(symbol->name, buffer);
        print_x86_function(buffer, symbol->value, context);
        gas_directive_size_label_relative(symbol->name, buffer);
        break;

    // data section
    case TYPE_KIND_NIL:
    case TYPE_KIND_BOOL:
    case TYPE_KIND_U8:
    case TYPE_KIND_U16:
    case TYPE_KIND_U32:
    case TYPE_KIND_U64:
    case TYPE_KIND_I8:
    case TYPE_KIND_I16:
    case TYPE_KIND_I32:
    case TYPE_KIND_I64:
    case TYPE_KIND_TUPLE: {
        x86_Layout const *layout =
            x86_context_layout_of_type(context, symbol->type);
        u64 size      = x86_layout_size_of(layout);
        u64 alignment = x86_layout_align_of(layout);
        gas_directive_data(buffer);
        gas_directive_globl(symbol->name, buffer);
        gas_directive_balign(alignment, buffer);
        gas_directive_type(symbol->name, STT_OBJECT, buffer);
        gas_directive_size(symbol->name, size, buffer);
        gas_directive_label(symbol->name, buffer);
        print_x86_value(buffer, symbol->value, layout);
        break;
    }

    default: EXP_UNREACHABLE();
    }

    string_append(buffer, SV("\n"));
    return 0;
}

i32 x86_footer(String *restrict buffer, Context *restrict context) {
    exp_assert(buffer != NULL);
    exp_assert(context != NULL);
    gas_directive_noexecstack(buffer);
    gas_directive_ident(SV(EXP_VERSION_STRING), buffer);
    return 0;
}
