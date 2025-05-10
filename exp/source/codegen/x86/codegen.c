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
#include "codegen/x86/intrinsics/align_of.h"
#include "codegen/x86/intrinsics/size_of.h"
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
        u64 size  = x86_size_of(symbol->type);
        u64 align = x86_align_of(symbol->type);
        gas_directive_bss(buffer);
        gas_directive_globl(symbol->name, buffer);
        gas_directive_balign(align, buffer);
        gas_directive_type(symbol->name, STT_OBJECT, buffer);
        gas_directive_size(symbol->name, size, buffer);
        gas_directive_label(symbol->name, buffer);
        gas_directive_zero(size, buffer);
        break;
    }

    case VALUE_KIND_NIL: {
        gas_directive_data(buffer);
        gas_directive_globl(symbol->name, buffer);
        gas_directive_type(symbol->name, STT_OBJECT, buffer);
        gas_directive_size(symbol->name, 1, buffer);
        gas_directive_label(symbol->name, buffer);
        gas_directive_u8(0, buffer);
        break;
    }

    case VALUE_KIND_BOOL: {
        gas_directive_data(buffer);
        gas_directive_globl(symbol->name, buffer);
        gas_directive_type(symbol->name, STT_OBJECT, buffer);
        gas_directive_size(symbol->name, 1, buffer);
        gas_directive_label(symbol->name, buffer);
        gas_directive_u8(value->bool_, buffer);
        break;
    }

    case VALUE_KIND_U8: {
        gas_directive_data(buffer);
        gas_directive_globl(symbol->name, buffer);
        gas_directive_type(symbol->name, STT_OBJECT, buffer);
        gas_directive_size(symbol->name, 1, buffer);
        gas_directive_label(symbol->name, buffer);
        gas_directive_u8(value->u8_, buffer);
        break;
    }

    case VALUE_KIND_U16: {
        gas_directive_data(buffer);
        gas_directive_globl(symbol->name, buffer);
        gas_directive_balign(2, buffer);
        gas_directive_type(symbol->name, STT_OBJECT, buffer);
        gas_directive_size(symbol->name, 2, buffer);
        gas_directive_label(symbol->name, buffer);
        gas_directive_u16(value->u16_, buffer);
        break;
    }

    case VALUE_KIND_U32: {
        gas_directive_data(buffer);
        gas_directive_globl(symbol->name, buffer);
        gas_directive_balign(4, buffer);
        gas_directive_type(symbol->name, STT_OBJECT, buffer);
        gas_directive_size(symbol->name, 4, buffer);
        gas_directive_label(symbol->name, buffer);
        gas_directive_u32(value->u32_, buffer);
        break;
    }

    case VALUE_KIND_U64: {
        gas_directive_data(buffer);
        gas_directive_globl(symbol->name, buffer);
        gas_directive_balign(8, buffer);
        gas_directive_type(symbol->name, STT_OBJECT, buffer);
        gas_directive_size(symbol->name, 8, buffer);
        gas_directive_label(symbol->name, buffer);
        gas_directive_u64(value->u64_, buffer);
        break;
    }

    case VALUE_KIND_I8: {
        gas_directive_data(buffer);
        gas_directive_globl(symbol->name, buffer);
        gas_directive_balign(1, buffer);
        gas_directive_type(symbol->name, STT_OBJECT, buffer);
        gas_directive_size(symbol->name, 1, buffer);
        gas_directive_label(symbol->name, buffer);
        gas_directive_i8(value->i8_, buffer);
        break;
    }

    case VALUE_KIND_I16: {
        gas_directive_data(buffer);
        gas_directive_globl(symbol->name, buffer);
        gas_directive_balign(2, buffer);
        gas_directive_type(symbol->name, STT_OBJECT, buffer);
        gas_directive_size(symbol->name, 2, buffer);
        gas_directive_label(symbol->name, buffer);
        gas_directive_i16(value->i16_, buffer);
        break;
    }

    case VALUE_KIND_I32: {
        gas_directive_data(buffer);
        gas_directive_globl(symbol->name, buffer);
        gas_directive_balign(4, buffer);
        gas_directive_type(symbol->name, STT_OBJECT, buffer);
        gas_directive_size(symbol->name, 4, buffer);
        gas_directive_label(symbol->name, buffer);
        gas_directive_i32(value->i32_, buffer);
        break;
    }

    case VALUE_KIND_I64: {
        gas_directive_data(buffer);
        gas_directive_globl(symbol->name, buffer);
        gas_directive_balign(8, buffer);
        gas_directive_type(symbol->name, STT_OBJECT, buffer);
        gas_directive_size(symbol->name, 8, buffer);
        gas_directive_label(symbol->name, buffer);
        gas_directive_i64(value->i64_, buffer);
        break;
    }

    case VALUE_KIND_TUPLE: {
        PANIC("#TODO: Support Global Tuple definitions");
        // x86_Tuple x86_tuple;
        // x86_tuple_create(&x86_tuple, &value->tuple, context);
        // u64 size  = x86_size_of(symbol->type);
        // u64 align = x86_align_of(symbol->type);
        // gas_directive_data(buffer);
        // gas_directive_globl(symbol->name, buffer);
        // gas_directive_balign(align, buffer);
        // gas_directive_type(symbol->name, STT_OBJECT, buffer);
        // gas_directive_size(symbol->name, size, buffer);
        // gas_directive_label(symbol->name, buffer);
        // print_x86_tuple(buffer, &x86_tuple);
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
