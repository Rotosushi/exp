
#include <assert.h>

#include "analysis/finalize.h"
#include "core/finalize.h"

ExpResult finalize_context(Context *context) {
    assert(context != nullptr);

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
