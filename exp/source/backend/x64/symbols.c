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

#include "backend/x64/symbols.h"
#include "utility/alloc.h"
#include "utility/unreachable.h"

void x64_symbol_destroy(x64_Symbol *restrict symbol) {
    x64_function_body_terminate(&symbol->body);
}

x64_SymbolTable x64_symbol_table_create(u64 count) {
    x64_SymbolTable symbols = {.count  = count,
                               .buffer = callocate(count, sizeof(x64_Symbol))};
    return symbols;
}

void x64_symbol_table_destroy(x64_SymbolTable *restrict symbols) {
    assert(symbols != nullptr);
    for (u64 i = 0; i < symbols->count; ++i) {
        x64_symbol_destroy(symbols->buffer + i);
    }
    symbols->count = 0;
    deallocate(symbols->buffer);
    symbols->buffer = nullptr;
}

x64_Symbol *x64_symbol_table_at(x64_SymbolTable *restrict symbols,
                                StringView name) {
    assert(symbols != nullptr);
    for (u64 i = 0; i < symbols->count; ++i) {
        x64_Symbol *sym = symbols->buffer + i;
        if (string_view_empty(sym->name)) {
            sym->name = name;
            return sym;
        }

        if (string_view_equality(sym->name, name)) { return sym; }
    }
    EXP_UNREACHABLE();
}
