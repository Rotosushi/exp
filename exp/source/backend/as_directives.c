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

#include "backend/as_directives.h"
#include "utility/numeric_conversions.h"
#include "utility/panic.h"

void directive_file(StringView path, FILE *file) {
  file_write("  .file \"", file);
  file_write(path.ptr, file);
  file_write("\"\n", file);
}

void directive_arch(StringView cpu_type, FILE *file) {
  file_write("  .arch ", file);
  file_write(cpu_type.ptr, file);
  file_write("\n", file);
}

void directive_ident(StringView comment, FILE *file) {
  file_write("  .ident \"", file);
  file_write(comment.ptr, file);
  file_write("\"\n", file);
}

void directive_noexecstack(FILE *file) {
  /**
   * this is the assembly directive which marks the stack as unexecutable.
   * (as far as I can tell, I cannot find documentation which explicitly
   * states that ".note.GNU-stack" marks the stack as noexec, only that
   * "... and the .note.GNU-stack section may have the executable (x)
   *  flag added". which implies to me that the .note... has something
   * to do with marking the stack as exec or noexec.)
   */
  static char const noexecstack[] =
      "  .section .note.GNU-stack,\"\",@progbits\n";
  file_write(noexecstack, file);
}

void directive_globl(StringView name, FILE *file) {
  file_write("  .globl ", file);
  file_write(name.ptr, file);
  file_write("\n", file);
}

void directive_data(FILE *file) { file_write("  .data\n", file); }

void directive_bss(FILE *file) { file_write("  .bss\n", file); }

void directive_text(FILE *file) { file_write("  .text\n", file); }

void directive_balign(u64 align, FILE *file) {
  file_write("  .balign ", file);
  print_uintmax(align, RADIX_DECIMAL, file);
  file_write("\n", file);
}

void directive_size(StringView name, u64 size, FILE *file) {
  file_write("  .size ", file);
  file_write(name.ptr, file);
  file_write(", ", file);
  print_uintmax(size, RADIX_DECIMAL, file);
  file_write("\n", file);
}

void directive_size_label_relative(StringView name, FILE *file) {
  file_write("  .size ", file);
  file_write(name.ptr, file);
  // the '.' symbol refers to the current address, the '-' is
  // arithmetic subtraction, and the label refers to the address
  // of the label. thus, label relative size computes to the
  // numeric difference between the current address and the address
  // of the label directive
  file_write(", .-", file);
  file_write(name.ptr, file);
  file_write("\n", file);
}

void directive_type(StringView name, STT_Type kind, FILE *file) {
  file_write("  .type ", file);
  file_write(name.ptr, file);
  file_write(", ", file);

  switch (kind) {
  case STT_OBJECT:
    file_write("@object\n", file);
    break;

  case STT_FUNC:
    file_write("@function\n", file);
    break;

  case STT_TLS:
    file_write("@tls_object\n", file);
    break;

  case STT_COMMON:
    file_write("@common\n", file);
    break;

  default:
    PANIC("bad STT_Type");
    break;
  }
}

void directive_quad(i64 value, FILE *file) {
  file_write("  .quad ", file);
  print_intmax(value, RADIX_DECIMAL, file);
  file_write("\n", file);
}

void directive_byte(unsigned char value, FILE *file) {
  file_write("  .byte ", file);
  print_uintmax(value, RADIX_DECIMAL, file);
  file_write("\n", file);
}

void directive_zero(u64 bytes, FILE *file) {
  file_write("  .zero ", file);
  print_uintmax(bytes, RADIX_DECIMAL, file);
  file_write("\n", file);
}

void directive_string(StringView sv, FILE *file) {
  file_write("  .string \"", file);
  file_write(sv.ptr, file);
  file_write("\"\n", file);
}

void directive_label(StringView name, FILE *file) {
  file_write(name.ptr, file);
  file_write(":\n", file);
}
