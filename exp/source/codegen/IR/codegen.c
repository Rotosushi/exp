/**
 * Copyright (C) 2025 cade
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

#include "codegen/IR/codegen.h"
#include "codegen/IR/directives.h"
#include "support/assert.h"
#include "support/config.h"

i32 ir_header(String *restrict buffer, Context *restrict context) {
    exp_assert(buffer != NULL);
    exp_assert(context != NULL);
    ir_directive_version(SV("1.0"), buffer);
    ir_directive_file(context_source_path(context), buffer);
    return 0;
}

i32 ir_codegen(String *restrict buffer,
               Symbol const *restrict symbol,
               Context *restrict context) {
    exp_assert(buffer != NULL);
    exp_assert(symbol != NULL);
    exp_assert(context != NULL);
    ir_directive_let(symbol->name, buffer);
    string_append(buffer, SV(": "));
    print_type(buffer, symbol->type);
    string_append(buffer, SV(" = "));
    print_value(buffer, symbol->value, context);
    string_append(buffer, SV(";\n"));
    return 0;
}

i32 ir_footer(String *restrict buffer, Context *restrict context) {
    exp_assert(buffer != NULL);
    exp_assert(context != NULL);
    ir_directive_comment(SV(EXP_VERSION_STRING), buffer);
    return 0;
}
