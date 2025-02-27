
#include <assert.h>

#include "targets/ir/emit.h"
#include "utility/io.h"

ExpResult emit_ir_assembly(Context *context) {
    assert(context != nullptr);

    String buffer;
    string_initialize(&buffer);
    SymbolTable *symbol_table = &context->symbol_table;

    for (u64 i = 0; i < symbol_table->capacity; ++i) {
        Symbol *symbol = symbol_table->elements[i];
        if (symbol == nullptr) { continue; }
        string_append(&buffer, symbol->name);
        string_append(&buffer, SV(" "));
        print_function(&buffer, &symbol->function_body, context);
        string_append(&buffer, SV("\n"));
    }

    StringView ir_path = context_ir_path(context);
    FILE *ir_file      = file_open(ir_path.ptr, "w");
    file_write(string_to_cstring(&buffer), ir_file);
    file_close(ir_file);
    return EXP_SUCCESS;
}
