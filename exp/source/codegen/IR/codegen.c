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
#include "support/io.h"

i32 ir_codegen(Context *restrict context) {
    exp_assert(context != NULL);

    String buffer = string_create();
    ir_directive_version(SV("1.0"), &buffer);
    ir_directive_file(context_source_path(context), &buffer);

    SymbolTable *symbol_table = &context->global_symbol_table;
    for (u64 index = 0; index < symbol_table->capacity; ++index) {
        Symbol *symbol = symbol_table->elements[index];
        if (symbol == NULL) { continue; }

        if (symbol->kind == SYMBOL_KIND_FUNCTION) {
            ir_directive_function(symbol->name, &buffer);
            print_function(&buffer, &symbol->function_body, context);
            string_append(&buffer, SV("\n"));
        }
    }

    StringView ir_path = context_ir_path(context);
    FILE      *file    = file_open(ir_path.ptr, "w");
    file_write(string_to_view(&buffer), file);
    file_close(file);
    string_destroy(&buffer);
    return 0;
}
