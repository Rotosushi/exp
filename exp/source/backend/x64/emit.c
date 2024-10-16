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
#include <assert.h>

#include "backend/directives.h"
#include "backend/x64/emit.h"
#include "intrinsics/size_of.h"
#include "utility/config.h"
#include "utility/io.h"
#include "utility/panic.h"

static void x64_emit_function(x64_Symbol *restrict sym,
                              String *restrict buffer,
                              Context *restrict context) {
  directive_text(buffer);
  directive_balign(8, buffer);
  directive_globl(sym->name, buffer);
  directive_type(sym->name, STT_FUNC, buffer);
  directive_label(sym->name, buffer);

  x64_bytecode_emit(&sym->body.bc, buffer, context);

  directive_size_label_relative(sym->name, buffer);
  string_append(buffer, SV("\n"));
}

static void x64_emit_functions(x64_Symbol **restrict symbols,
                               u64 size,
                               String *restrict buffer,
                               Context *restrict context) {
  for (u64 i = 0; i < size; ++i) {
    x64_emit_function(symbols[i], buffer, context);
  }
}

static void x64_emit_constant(x64_Symbol *restrict sym,
                              String *restrict buffer,
                              [[maybe_unused]] Context *restrict context) {
  /* #TODO since these are global constants, they have to be known
   * at compile time. Thus it is a natural optimization to place these
   * comptime constants in the (usually) unalterable 'text' section of
   * an executable. Note however that since we are initializing these
   * constants at runtime (due to ease of implementation) we have to place
   * them in the 'data' section.
   * We can further note that this is where any global variables would be
   * placed, if we ever implement global variables
   */
  directive_data(buffer);
  directive_balign(8, buffer);
  directive_globl(sym->name, buffer);
  directive_type(sym->name, STT_OBJECT, buffer);
  u64 size = size_of(sym->type);
  directive_size(sym->name, size, buffer);
  directive_label(sym->name, buffer);

  directive_zero(size, buffer);

  string_append(buffer, SV("\n"));
}

static void x64_emit_constants(x64_Symbol **restrict symbols,
                               u64 size,
                               Context *restrict context,
                               String *restrict buffer) {
  for (u64 i = 0; i < size; ++i) {
    x64_emit_constant(symbols[i], buffer, context);
  }
}

static void x64_emit_init(Context *restrict context,
                          String *restrict buffer,
                          x64_Symbol **symbols,
                          u64 length) {
  StringView name = SV("_init");
  directive_text(buffer);
  directive_balign(8, buffer);
  directive_globl(name, buffer);
  directive_type(name, STT_FUNC, buffer);
  directive_label(name, buffer);

  for (u64 i = 0; i < length; ++i) {
    x64_Symbol *symbol = symbols[i];
    assert(symbol->kind == X64SYM_CONSTANT);
    x64_FunctionBody *body = &symbol->body;
    x64_Bytecode *bc       = &body->bc;
    x64_bytecode_emit(bc, buffer, context);
  }

  string_append(buffer, SV("ret\n"));

  directive_size_label_relative(name, buffer);
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

void x64_emit(x64_Context *restrict x64_context) {
  u64 size  = x64_context->symbols.size;
  u64 c_len = 0, f_len = 0;
  // #TODO: this is obviously wasteful...
  x64_Symbol *constants[size];
  x64_Symbol *functions[size];

  x64_SymbolIterator iter = x64_context_symbol_iterator(x64_context);
  for (u64 i = 0; !x64_symbol_iterator_done(&iter) && (i < size);
       x64_symbol_iterator_next(&iter), ++i) {
    switch (iter.symbol->kind) {
    case X64SYM_UNDEFINED: PANIC("bug in x64_symbol_iterator"); break;

    case X64SYM_FUNCTION: functions[f_len++] = iter.symbol; break;
    case X64SYM_CONSTANT: constants[c_len++] = iter.symbol; break;
    default:              PANIC("unknown x64 symbol kind"); break;
    }
  }

  String buffer = string_create();
  x64_emit_file_prolouge(x64_context->context, &buffer);

  x64_emit_constants(constants, c_len, x64_context->context, &buffer);

  x64_emit_functions(functions, f_len, &buffer, x64_context->context);

  x64_emit_init(x64_context->context, &buffer, constants, c_len);

  x64_emit_file_epilouge(&buffer);

  StringView path = context_assembly_path(x64_context->context);
  FILE *file      = file_open(path, SV("w"));
  file_write(file, string_to_view(&buffer));
  file_close(file);

  string_destroy(&buffer);
}
