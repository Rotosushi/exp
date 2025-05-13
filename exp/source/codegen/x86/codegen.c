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
#include "codegen/x86/imr/function.h"
#include "codegen/x86/value/scalar.h"
#include "codegen/x86/value/tuple.h"
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

i32 x86_header(String *restrict buffer, Context *restrict context) {
    exp_assert(buffer != NULL);
    exp_assert(context != NULL);
    gas_directive_file(context_source_path(context), buffer);
    // #TODO: this needs to come from the arch specific section of the target.
    // else we leave it blank.
    gas_directive_arch(SV("znver3"), buffer);
    gas_directive_intel_syntax(buffer);
    return 0;
}

i32 x86_codegen(String *restrict buffer,
                Symbol const *restrict symbol,
                Context *restrict context) {
    exp_assert(buffer != NULL);
    exp_assert(context != NULL);

    Value const *value = symbol->value;
    switch (value->kind) {
    // NOTE: any uninitialized value must have a statically known type
    // Thus we can allocate the correct amount of space for
    // an uninitialized value by relying on the type of the value.
    case VALUE_KIND_UNINITIALIZED: {
        x86_codegen_uninitialized_symbol(buffer, symbol, context);
        break;
    }

    case VALUE_KIND_NIL: {
        x86_codegen_nil_symbol(buffer, symbol);
        break;
    }

    case VALUE_KIND_BOOL: {
        x86_codegen_bool_symbol(buffer, symbol);
        break;
    }

    case VALUE_KIND_U8: {
        x86_codegen_u8_symbol(buffer, symbol);
        break;
    }

    case VALUE_KIND_U16: {
        x86_codegen_u16_symbol(buffer, symbol);
        break;
    }

    case VALUE_KIND_U32: {
        x86_codegen_u32_symbol(buffer, symbol);
        break;
    }

    case VALUE_KIND_U64: {
        x86_codegen_u64_symbol(buffer, symbol);
        break;
    }

    case VALUE_KIND_I8: {
        x86_codegen_i8_symbol(buffer, symbol);
        break;
    }

    case VALUE_KIND_I16: {
        x86_codegen_i16_symbol(buffer, symbol);
        break;
    }

    case VALUE_KIND_I32: {
        x86_codegen_i32_symbol(buffer, symbol);
        break;
    }

    case VALUE_KIND_I64: {
        x86_codegen_i64_symbol(buffer, symbol);
        break;
    }

    case VALUE_KIND_TUPLE: {
        x86_codegen_tuple_symbol(buffer, symbol, context);
        break;
    }

    case VALUE_KIND_FUNCTION: {
        x86_Function x86_function;
        x86_function_create(&x86_function);

        if (x86_function_codegen(&x86_function, &value->function, context)) {
            x86_function_destroy(&x86_function);
            return 1;
        }

        gas_directive_text(buffer);
        gas_directive_globl(symbol->name, buffer);
        gas_directive_balign(8, buffer);
        gas_directive_type(symbol->name, STT_FUNC, buffer);
        gas_directive_label(symbol->name, buffer);

        print_x86_function(buffer, &x86_function);

        gas_directive_size_label_relative(symbol->name, buffer);

        x86_function_destroy(&x86_function);
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
