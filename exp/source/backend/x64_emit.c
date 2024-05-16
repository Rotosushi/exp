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

#include "backend/x64_emit.h"
#include "backend/directives.h"
#include "utility/config.h"

static void emit_x64symbol(X64Symbol *restrict sym,
                           String *restrict buffer,
                           Context *restrict context) {
  directive_text(buffer);
  directive_globl(sym->name, buffer);
  directive_type(sym->name, STT_FUNC, buffer);
  directive_label(sym->name, buffer);

  x64bytecode_emit(&sym->body.bc, buffer, context);

  directive_size_label_relative(sym->name, buffer);
}

static void emit_file_prolouge(Context *restrict context,
                               String *restrict buffer) {
  directive_file(context_source_path(context), buffer);
  string_append(buffer, SV("\n"));
}

static void emit_file_epilouge(String *restrict buffer) {
  StringView version = SV(EXP_VERSION_STRING);
  directive_ident(version, buffer);
  directive_noexecstack(buffer);
}

void x64emit(X64Context *restrict x64context) {
  String buffer = string_create();

  emit_file_prolouge(x64context->context, &buffer);

  X64Symbols *symbols = &x64context->symbols;
  for (u64 i = 0; i < symbols->count; ++i) {
    X64Symbol *sym = symbols->buffer + i;
    emit_x64symbol(sym, &buffer, x64context->context);
  }

  emit_file_epilouge(&buffer);

  StringView path = context_assembly_path(x64context->context);
  FILE *file      = file_open(path.ptr, "w");
  file_write(string_to_cstring(&buffer), file);
  file_close(file);

  string_destroy(&buffer);
}