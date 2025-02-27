/**
 * Copyright 2025 Cade Weinberg. All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#include "targets/x86_64/emit.h"
#include "targets/x86_64/codegen/instruction.h"
#include "targets/x86_64/print/gnu_as_directives.h"
#include "utility/assert.h"
#include "utility/config.h"
#include "utility/io.h"

static ExpResult emit_x86_64_file_prolouge(String *buffer, Context *context) {
    EXP_ASSERT(buffer != nullptr);
    EXP_ASSERT(context != nullptr);
    directive_file(buffer, context_assembly_path(context));
    string_append(buffer, SV("\n"));
    return EXP_SUCCESS;
}

static ExpResult emit_x86_64_file_epilouge(String *buffer, Context *context) {
    EXP_ASSERT(buffer != nullptr);
    EXP_ASSERT(context != nullptr);
    directive_ident(buffer, SV(EXP_VERSION_STRING));
    directive_noexecstack(buffer);
    return EXP_SUCCESS;
}

ExpResult emit_x86_64_symbol(String *buffer, Symbol *symbol, Context *context) {
    EXP_ASSERT(buffer != nullptr);
    EXP_ASSERT(symbol != nullptr);
    EXP_ASSERT(context != nullptr);

    directive_text(buffer);
    directive_globl(buffer, symbol->name);
    directive_type(buffer, symbol->name, STT_FUNC);
    directive_label(buffer, symbol->name);

    Function *function = &symbol->function_body;
    Block *block       = &function->block;
    for (u64 index = 0; index < block->length; ++index) {
        x86_64_codegen_instruction(buffer, block->buffer + index, function,
                                   context);
    }

    directive_size_label_relative(buffer, symbol->name);
    string_append(buffer, SV("\n"));
    return EXP_SUCCESS;
}

ExpResult emit_x86_64_assembly(Context *context) {
    EXP_ASSERT(context != nullptr);

    String buffer;
    string_initialize(&buffer);

    if (emit_x86_64_file_prolouge(&buffer, context) != EXP_SUCCESS) {
        return EXP_FAILURE;
    }

    SymbolTable *symbol_table = &context->symbol_table;
    for (u64 index = 0; index < symbol_table->capacity; ++index) {
        Symbol *symbol = symbol_table->elements[index];
        if (symbol == nullptr) { continue; }
        if (emit_x86_64_symbol(&buffer, symbol, context) != EXP_SUCCESS) {
            return EXP_FAILURE;
        }
        string_append(&buffer, SV("\n"));
    }

    if (emit_x86_64_file_epilouge(&buffer, context) != EXP_SUCCESS) {
        return EXP_FAILURE;
    }

    StringView path = context_assembly_path(context);
    File file       = file_open(path, FILEMODE_WRITE);
    file_write(string_to_view(&buffer), &file);
    file_close(&file);
    string_terminate(&buffer);
    return EXP_SUCCESS;
}
