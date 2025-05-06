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
#include <assert.h>

#include "codegen/x86/imr/function.h"
#include "support/allocation.h"

static x86_FormalArgumentList x86_formal_argument_list_create(u8 size) {
    x86_FormalArgumentList args = {
        .size = size, .buffer = allocate(size * sizeof(x86_FormalArgument))};
    return args;
}

static void
x86_formal_arguments_destroy(x86_FormalArgumentList *restrict args) {
    args->size = 0;
    deallocate(args->buffer);
    args->buffer = NULL;
}

void x86_function_create(x86_Function *restrict x86_function,
                         Function const *restrict function,
                         Context *restrict context) {
    assert(x86_function != NULL);
    assert(function != NULL);
    x86_function->arguments =
        x86_formal_argument_list_create(function->arguments.size);
    x86_bytecode_create(&x86_function->body);
    x86_function->gprp = x86_gprp_construct();
    x86_locations_create(&x86_function->locations,
                         function_locals_count(function));

    Bytecode const *body = &function->body;
    for (u32 index = 0; index < body->length; ++index) {}
}

void x86_function_destroy(x86_Function *restrict function) {
    assert(function != NULL);
    x86_formal_arguments_destroy(&function->arguments);
    x86_bytecode_destroy(&function->body);
    function->gprp = x86_gprp_construct();
    x86_locations_destroy(&function->locations);
}
