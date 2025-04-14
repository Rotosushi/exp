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

#include "codegen/x86/env/symbols.h"
#include "support/allocation.h"
#include "support/unreachable.h"

void x86_symbol_destroy(x86_Symbol *restrict symbol) {
    x86_function_destroy(&symbol->body);
}

x86_SymbolTable x86_symbol_table_create(u64 count) {
    x86_SymbolTable symbols = {.count  = count,
                               .buffer = callocate(count, sizeof(x86_Symbol))};
    return symbols;
}

void x86_symbol_table_destroy(x86_SymbolTable *restrict symbols) {
    assert(symbols != NULL);
    for (u64 i = 0; i < symbols->count; ++i) {
        x86_symbol_destroy(symbols->buffer + i);
    }
    symbols->count = 0;
    deallocate(symbols->buffer);
    symbols->buffer = NULL;
}

x86_Symbol *x86_symbol_table_at(x86_SymbolTable *restrict symbols,
                                StringView name) {
    assert(symbols != NULL);
    for (u64 i = 0; i < symbols->count; ++i) {
        x86_Symbol *sym = symbols->buffer + i;
        if (string_view_empty(sym->name)) {
            sym->name = name;
            return sym;
        }

        if (string_view_equal(sym->name, name)) { return sym; }
    }
    EXP_UNREACHABLE();
}
