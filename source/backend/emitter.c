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

#include "backend/emitter.h"
#include "filesystem/io.h"
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
 * @brief emit the header for the output assembly file.
 *
 * @note this information includes the directives:
 *  ".file", ".arch", anything else??
 *
 * @param file the FILE to write to.
 */
static void emit_x64_linux_header(FILE *file) {}

static void emit_x64_linux_footer(FILE *file) {}

static void emit_x64_linux_element(SymbolTableElement *element, FILE *file) {}

void emit(Context *restrict context, StringView path) {
  FILE *file = fopen(path.ptr, "w");
  if (file == NULL) {
    panic_errno("fopen failed", sizeof("fopen failed"));
  }

  SymbolTableIterator iter =
      symbol_table_iterator_create(&(context->global_symbols));

  emit_x64_linux_header(file);

  while (!symbol_table_iterator_done(&iter)) {
    emit_x64_linux_element(iter.element, file);

    symbol_table_iterator_next(&iter);
  }

  emit_x64_linux_footer(file);

  if (fclose(file) == EOF) {
    panic_errno("fclose failed", sizeof("fclose failed"));
  }
}