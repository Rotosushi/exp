/**
 * Copyright (C) 2025 Cade Weinberg
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

/**
 * @file env/symbol_table.c
 */

#include <math.h>

#include "env/symbol_table.h"
#include "utility/alloc.h"
#include "utility/array_growth.h"
#include "utility/assert.h"
#include "utility/hash.h"

#define SYMBOL_TABLE_MAX_LOAD 0.75

void symbol_table_create(SymbolTable *symbol_table) {
    EXP_ASSERT(symbol_table != nullptr);
    symbol_table->capacity = 0;
    symbol_table->count    = 0;
    symbol_table->elements = nullptr;
}

void symbol_table_destroy(SymbolTable *symbol_table) {
    EXP_ASSERT(symbol_table != nullptr);

    for (u64 i = 0; i < symbol_table->capacity; ++i) {
        Symbol *element = symbol_table->elements[i];
        if (element == nullptr) { continue; }
        function_terminate(&element->function_body);
        deallocate(element);
    }

    symbol_table->count    = 0;
    symbol_table->capacity = 0;
    deallocate(symbol_table->elements);
    symbol_table->elements = nullptr;
}

static Symbol **symbol_table_find(Symbol **elements, u64 capacity,
                                  StringView name) {
    u64 index = hash_cstring(name.ptr, name.length) % capacity;
    while (1) {
        Symbol **element = elements + index;
        if (((*element) == nullptr) ||
            string_view_equality(name, (*element)->name)) {
            return element;
        }

        index = (index + 1) % capacity;
    }
}

static void symbol_table_grow(SymbolTable *symbol_table) {
    Growth64 g = array_growth_u64(symbol_table->capacity, sizeof(Symbol *));
    Symbol **elements = callocate(g.new_capacity, sizeof(Symbol *));

    if (symbol_table->elements != nullptr) {
        for (u64 i = 0; i < symbol_table->capacity; ++i) {
            Symbol *element = symbol_table->elements[i];
            if (element == nullptr) { continue; }

            Symbol **dest =
                symbol_table_find(elements, g.new_capacity, element->name);
            *dest = element;
        }

        // we can avoid freeing each element because we
        // move the data to the new allocation.
        deallocate(symbol_table->elements);
    }

    symbol_table->capacity = g.new_capacity;
    symbol_table->elements = elements;
}

static bool symbol_table_full(SymbolTable *symbol_table) {
    u64 load_limit =
        (u64)floor((double)symbol_table->capacity * SYMBOL_TABLE_MAX_LOAD);
    return (symbol_table->count + 1) >= load_limit;
}

Symbol *symbol_table_at(SymbolTable *symbol_table, StringView name) {
    EXP_ASSERT(symbol_table != nullptr);

    if (symbol_table_full(symbol_table)) { symbol_table_grow(symbol_table); }

    Symbol **element =
        symbol_table_find(symbol_table->elements, symbol_table->capacity, name);
    // #TODO: We allocate a new element when we find any new name. if we lookup
    //  names that never get defined they will be considered "user defined
    //  symbols" by the for-each implementation of typecheck and codegen.
    //  This will be considered an error by both, though I think any symbols
    //  that get looked up before they are defined is something we want to
    //  handle.

    if ((*element) == nullptr) {
        (*element)       = callocate(1, sizeof(Symbol));
        (*element)->name = name;
        symbol_table->count += 1;
    }

    return *element;
}
