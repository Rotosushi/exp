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
 * along with exp.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <assert.h>

#include "backend/emitter.h"
#include "filesystem/io.h"
#include "utility/numbers_to_string.h"
#include "utility/panic.h"

/**
 * @brief used as the argument for the ".arch" directive in GNU as
 *
 * @note as specified here:
 * https://sourceware.org/binutils/docs/as/i386_002dArch.html
 * this is specific to the cpu that I am currently
 * developing on. in a finished compiler the target cpu
 * would be figured out at runtime, either specified on
 * the command line or retrieved by parsing the output of
 * something like "/proc/cpuinfo". it is important to note
 * that knowing the cpu is step one, as the "cpu_type"
 * argument to the ".arch" directive takes in names which
 * specify the microarchitecture of the cpu. and not it's
 * product name. for instance, my development cpu is
 * an AMD Ryzen 7 5800. this cpu is based on the zen 3
 * microarchitecture, and that is why I chose "znver3"
 * (I assume it stands for zen version 3, the as documentation
 * just give the list of available arguments, they don't
 * list the meaning of each argument.) (on another note
 * specific sub-architectures can also be specified, and
 * these specifiers read more like the availability or
 * unavailability of specific features of a microarchitecture,
 * like ".sse", ".monitor", etc.
 * it is also important to note here that these subfeatures
 * are also listed within "/proc/cpuinfo")
 *
 * @todo #TODO storing this string as a static string literal
 * only works when it is acceptable to have the value hardcoded.
 * This data needs to be associated with some dynamic structure
 * which builds up it's content at runtime.
 */
static char const *cpu_type = "znver3";

/**
 * @brief emit the header of the assembly file representing the
 * given context.
 *
 * @note this information includes the directives:
 *  ".file", ".arch", anything else??
 *
 * @param file the FILE to write to.
 */
static void emit_x64_linux_header(Context *restrict context, FILE *file) {
  StringView path = context_source_path(context);
  file_write(".file ", sizeof(".file "), file);
  file_write(path.ptr, path.length, file);
  file_write("\n", sizeof("\n"), file);

  file_write(".arch ", sizeof(".arch "), file);
  file_write(cpu_type, sizeof(cpu_type), file);
  file_write("\n", sizeof("\n"), file);

  file_write("\n", sizeof("\n"), file);
}

/**
 * @brief emit the footer for the assembly file representing the
 * given context.
 *
 * @note this information includes what?? maybe debugging info??
 *
 * @param context
 * @param file
 */
static void emit_x64_linux_footer([[maybe_unused]] Context *restrict context,
                                  [[maybe_unused]] FILE *file) {
  return;
}

/**
 * @brief emit the assembly for the given global symbol into the assembly
 * file representing the given context.
 *
 * @param context
 * @param element
 * @param file
 */
static void
emit_x64_linux_global_symbol([[maybe_unused]] Context *restrict context,
                             SymbolTableElement *global, FILE *file) {
  /*
a global object declaration in assembly looks like:
  .globl <name>
  .bss | .data
  .align <alignment>
  .type <name>, @object
  .size <name>, <sizeof>
<name>:
  .byte <init> | .zero <sizeof> | .quad <init> | .int <init> | ...


-- all global constant symbols can go into the .data section, unless they
are uninitialized. then they default initialized to zero, and can go into
the .bss section.

--

*/
  StringView name = global->name;
  Type *type = global->type;
  Value *value = &(global->value);
  switch (type->kind) {
  // #TODO
  case TYPEKIND_NIL:
  case TYPEKIND_BOOLEAN:
    break;

  case TYPEKIND_INTEGER:
    file_write("  .globl ", sizeof("  .globl "), file);
    file_write(name.ptr, name.length, file);
    file_write("\n", sizeof("\n"), file);

    if (value->kind == VALUEKIND_INTEGER) {
      file_write("  .data\n", sizeof("  .data\n"), file);
    } else {
      file_write("  .bss\n", sizeof("  .bss\n"), file);
    }

    file_write("  .align ", sizeof("  .align "), file);
    file_write("8\n", sizeof("8\n"), file);

    file_write("  .type ", sizeof("  .type "), file);
    file_write(name.ptr, name.length, file);
    file_write(", @object\n", sizeof(", @object\n"), file);

    file_write("  .size ", sizeof("  .size "), file);
    file_write(name.ptr, name.length, file);
    file_write(", 8\n", sizeof(", 8\n"), file);

    file_write(name.ptr, name.length, file);
    file_write(":\n", sizeof(":\n"), file);

    if (value->kind == VALUEKIND_INTEGER) {
      file_write("  .quad ", sizeof("  .quad "), file);
      size_t length =
          intmax_safe_strlen((intmax_t)value->integer, RADIX_DECIMAL);
      char i[length + 1];
      if (intmax_to_str((intmax_t)value->integer, i, RADIX_DECIMAL) == NULL) {
        panic("int conversion failed", sizeof("int conversion failed"));
      }
      file_write(i, length, file);
      file_write("\n", sizeof("\n"), file);
    } else { // we assume the value is nil because there was no explicit
             // initializer
      file_write("  .zero 8\n", sizeof("  .zero 8\n"), file);
    }
    break;

  // #TODO
  case TYPEKIND_STRING_LITERAL:
    break;

  default:
    panic("bad VALUEKIND", sizeof("bad VALUEKIND"));
  }

  // give an extra line between globals in the assembly file
  file_write("\n", sizeof("\n"), file);
}

void emit(Context *restrict context) {
  StringView path = context_source_path(context);
  FILE *file = fopen(path.ptr, "w");
  if (file == NULL) {
    panic_errno("fopen failed", sizeof("fopen failed"));
  }

  SymbolTableIterator iter =
      symbol_table_iterator_create(&(context->global_symbols));

  emit_x64_linux_header(context, file);

  while (!symbol_table_iterator_done(&iter)) {
    emit_x64_linux_global_symbol(context, iter.element, file);

    symbol_table_iterator_next(&iter);
  }

  emit_x64_linux_footer(context, file);

  if (fclose(file) == EOF) {
    panic_errno("fclose failed", sizeof("fclose failed"));
  }
}