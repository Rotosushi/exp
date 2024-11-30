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

#include "backend/x64/emit.h"
#include "backend/directives.h"
#include "utility/config.h"

static void x64_emit_symbol(x64_Symbol *restrict sym,
                            String *restrict buffer,
                            x64_Context *restrict x64_context) {
    directive_text(buffer);
    directive_globl(sym->name, buffer);
    directive_type(sym->name, STT_FUNC, buffer);
    directive_label(sym->name, buffer);

    x64_bytecode_emit(&sym->body.bc, buffer, x64_context);

    directive_size_label_relative(sym->name, buffer);
    string_append(buffer, SV("\n"));
}

static void x64_emit_file_prolouge(x64_Context *restrict x64_context,
                                   String *restrict buffer) {
    directive_file(context_source_path(x64_context->context), buffer);
    string_append(buffer, SV("\n"));
}

static void x64_emit_file_epilouge(String *restrict buffer) {
    StringView version = SV(EXP_VERSION_STRING);
    directive_ident(version, buffer);
    directive_noexecstack(buffer);
}

void x64_emit(x64_Context *restrict x64_context) {
    String buffer;
    string_initialize(&buffer);

    x64_emit_file_prolouge(x64_context, &buffer);

    x64_SymbolTable *symbols = &x64_context->symbols;
    for (u64 i = 0; i < symbols->count; ++i) {
        x64_Symbol *sym = symbols->buffer + i;
        x64_emit_symbol(sym, &buffer, x64_context);
    }

    x64_emit_file_epilouge(&buffer);

    StringView path = context_assembly_path(x64_context->context);
    FILE *file      = file_open(path.ptr, "w");
    file_write(string_to_cstring(&buffer), file);
    file_close(file);

    string_destroy(&buffer);
}
