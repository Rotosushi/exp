/**
 * Copyright 2025 Cade Weinberg. All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#include "analysis/finalize.h"
#include "core/finalize.h"
#include "utility/assert.h"

ExpResult finalize_context(Context *context) {
    EXP_ASSERT(context != nullptr);

    SymbolTable *symbol_table = &context->symbol_table;
    for (u64 index = 0; index < symbol_table->capacity; ++index) {
        Symbol *symbol = symbol_table->elements[index];
        if (symbol == nullptr) { continue; }
        Function *function = &symbol->function_body;
        if (finalize_function(function, context) != EXP_SUCCESS) {
            return EXP_FAILURE;
        }
    }
    return EXP_SUCCESS;
}
