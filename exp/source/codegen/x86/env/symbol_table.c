// Copyright (C) 2026 Cade Weinberg
//
// This file is part of exp.
//
// exp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// exp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with exp.  If not, see <http://www.gnu.org/licenses/>.

#include <math.h>

#include "codegen/x86/env/symbol_table.h"
#include "support/allocation.h"
#include "support/array_growth.h"
#include "support/assert.h"
#include "support/constant_string.h"
#include "support/hash.h"

void x86_symbol_table_create(x86_SymbolTable *restrict table) {
    EXP_ASSERT(table != NULL);
    table->capacity = table->count = 0;
    table->elements                = NULL;
}

void x86_symbol_table_destroy(x86_SymbolTable *restrict table) {
    EXP_ASSERT(table != NULL);

    for (u32 i = 0; i < table->capacity; ++i) {
        x86_Symbol *element = table->elements[i];
        if (element == NULL) { continue; }
        deallocate(element);
    }

    table->count    = 0;
    table->capacity = 0;
    deallocate(table->elements);
    table->elements = NULL;
}

static x86_Symbol **x86_symbol_table_find(x86_Symbol **restrict elements,
                                          u32        capacity,
                                          StringView name) {
    u32 index = hash_cstring(name.ptr, name.length) % capacity;

    while (true) {
        x86_Symbol **element = elements + index;
        if (((*element) == NULL) || string_view_equal((*element)->name, name)) {
            return element;
        }

        index = (index + 1) % capacity;
    }
}

static void x86_symbol_table_grow(x86_SymbolTable *restrict table) {
    Growth_u32   g = array_growth_u32(table->capacity, sizeof(x86_Symbol *));
    x86_Symbol **elements = callocate(g.new_capacity, sizeof(x86_Symbol *));

    if (table->elements != NULL) {
        for (u32 i = 0; i < table->capacity; ++i) {
            x86_Symbol *element = table->elements[i];
            if (element == NULL) { continue; }

            x86_Symbol **dst =
                x86_symbol_table_find(elements, g.new_capacity, element->name);
            *dst = element;
        }

        deallocate(table->elements);
    }

    table->elements = elements;
}

static bool x86_symbol_table_full(x86_SymbolTable const *restrict table) {
    // table is greater than 75% full
    return ((table->count * 4) / 3) >= table->capacity;
}

x86_Symbol *x86_symbol_table_at(x86_SymbolTable *restrict table,
                                StringView name) {
    EXP_ASSERT(table != NULL);
    EXP_ASSERT(!string_view_empty(name));

    if (x86_symbol_table_full(table)) { x86_symbol_table_grow(table); }

    x86_Symbol **element =
        x86_symbol_table_find(table->elements, table->capacity, name);
    return *element;
}
