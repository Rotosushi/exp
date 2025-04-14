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

#include "codegen/x86/emit.h"
#include "codegen/directives.h"
#include "support/config.h"
#include "support/io.h"

static void x64_emit_symbol(x86_Symbol *restrict sym,
                            String *restrict buffer,
                            Context *restrict context) {
    directive_text(buffer);
    directive_globl(sym->name, buffer);
    directive_type(sym->name, STT_FUNC, buffer);
    directive_label(sym->name, buffer);

    x64_bytecode_emit(&sym->body.bc, buffer, context);

    directive_size_label_relative(sym->name, buffer);
    string_append(buffer, SV("\n"));
}

static void x64_emit_file_prolouge(Context *restrict context,
                                   String *restrict buffer) {
    directive_file(context_source_path(context), buffer);
    string_append(buffer, SV("\n"));
}

static void x64_emit_file_epilouge(String *restrict buffer) {
    StringView version = SV(EXP_VERSION_STRING);
    directive_ident(version, buffer);
    directive_noexecstack(buffer);
}

void x64_emit(x86_Context *restrict x64context) {
    String buffer = string_create();

    x64_emit_file_prolouge(x64context->context, &buffer);

    x86_SymbolTable *symbols = &x64context->symbols;
    for (u64 i = 0; i < symbols->count; ++i) {
        x86_Symbol *sym = symbols->buffer + i;
        x64_emit_symbol(sym, &buffer, x64context->context);
    }

    x64_emit_file_epilouge(&buffer);

    StringView path = context_assembly_path(x64context->context);

    FILE *file = file_open(path.ptr, "w");
    file_write(string_to_view(&buffer), file);
    file_close(file);

    string_destroy(&buffer);
}
