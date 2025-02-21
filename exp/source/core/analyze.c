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
 * @file core/analyze.c
 */

#include "core/analyze.h"
#include "analysis/analyze.h"
#include "utility/assert.h"

ExpResult analyze_context(Context *context) {
    EXP_ASSERT(context != nullptr);

    SymbolTable *symbol_table = &context->symbol_table;
    for (u64 index = 0; index < symbol_table->capacity; ++index) {
        Symbol *symbol = symbol_table->elements[index];
        if (symbol == nullptr) { continue; }
        Function *function = &symbol->function_body;
        if (analyze_function(function, context) != EXP_SUCCESS) {
            return EXP_FAILURE;
        }
    }
    return EXP_SUCCESS;
}
