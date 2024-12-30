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

#include "core/codegen.h"
#include "imr/type.h"
#include "utility/io.h"

static void emit_ir_assembly(Context *context) {
    assert(context != nullptr);

    String buffer;
    string_initialize(&buffer);
    SymbolTable *symbol_table = &context->symbol_table;

    for (u64 i = 0; i < symbol_table->capacity; ++i) {
        Symbol *symbol = symbol_table->elements[i];
        if (symbol == nullptr) { continue; }
        string_append(&buffer, symbol->name);
        string_append(&buffer, SV(" :"));
        print_type(&buffer, symbol->type);
        string_append(&buffer, SV("\n"));
        print_function_body(&buffer, &symbol->function_body, context);
        string_append(&buffer, SV("\n"));
    }

    StringView ir_path = context_ir_path(context);
    FILE *ir_file      = file_open(ir_path.ptr, "w");
    file_write(string_to_cstring(&buffer), ir_file);
    file_close(ir_file);
}

void codegen(Context *context) {
    assert(context != nullptr);

    if (context_emit_ir_assembly(context)) { emit_ir_assembly(context); }
}
