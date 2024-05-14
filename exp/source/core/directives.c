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

#include "core/directives.h"
#include "utility/numeric_conversions.h"
#include "utility/panic.h"

void directive_file(StringView path, String *restrict str) {
  string_append(str, SV("\t.file \""));
  string_append(str, path);
  string_append(str, SV("\"\n"));
}

void directive_arch(StringView cpu_type, String *restrict str) {
  string_append(str, SV("\t.arch "));
  string_append(str, cpu_type);
  string_append(str, SV("\n"));
}

void directive_ident(StringView comment, String *restrict str) {
  string_append(str, SV("\t.ident \""));
  string_append(str, comment);
  string_append(str, SV("\"\n"));
}

void directive_noexecstack(String *restrict str) {
  /**
   * this is the assembly directive which marks the stack as unexecutable.
   * (as far as I can tell, I cannot find documentation which explicitly
   * states that ".note.GNU-stack" marks the stack as noexec, only that
   * "... and the .note.GNU-stack section may have the executable (x)
   *  flag added". which implies to me that the .note... has something
   * to do with marking the stack as exec or noexec.)
   */
  StringView noexecstack = SV("\t.section .note.GNU-stack,\"\",@progbits\n");
  string_append(str, noexecstack);
}

void directive_globl(StringView name, String *restrict str) {
  string_append(str, SV("\t.globl "));
  string_append(str, name);
  string_append(str, SV("\n"));
}

void directive_data(String *restrict str) {
  string_append(str, SV("\t.data\n"));
}

void directive_bss(String *restrict str) { string_append(str, SV("\t.bss\n")); }

void directive_text(String *restrict str) {
  string_append(str, SV("\t.text\n"));
}

void directive_balign(u64 align, String *restrict str) {
  string_append(str, SV("\t.balign "));
  string_append_u64(str, align);
  string_append(str, SV("\n"));
}

void directive_size(StringView name, u64 size, String *restrict str) {
  string_append(str, SV("\t.size "));
  string_append(str, name);
  string_append(str, SV(", "));
  string_append_u64(str, size);
  string_append(str, SV("\n"));
}

void directive_size_label_relative(StringView name, String *restrict str) {
  string_append(str, SV("\t.size "));
  string_append(str, name);
  // the '.' symbol refers to the current address, the '-' is
  // arithmetic subtraction, and the label refers to the address
  // of the label. thus, label relative size computes to the
  // numeric difference between the current address and the address
  // of the label directive
  string_append(str, SV(", .-"));
  string_append(str, name);
  string_append(str, SV("\n"));
}

void directive_type(StringView name, STT_Type kind, String *restrict str) {
  string_append(str, SV("\t.type "));
  string_append(str, name);
  string_append(str, SV(", "));

  switch (kind) {
  case STT_OBJECT:
    string_append(str, SV("@object\n"));
    break;

  case STT_FUNC:
    string_append(str, SV("@function\n"));
    break;

  case STT_TLS:
    string_append(str, SV("@tls_object\n"));
    break;

  case STT_COMMON:
    string_append(str, SV("@common\n"));
    break;

  default:
    PANIC("bad STT_Type");
    break;
  }
}

void directive_quad(i64 value, String *restrict str) {
  string_append(str, SV("\t.quad "));
  string_append_i64(str, value);
  string_append(str, SV("\n"));
}

void directive_byte(unsigned char value, String *restrict str) {
  string_append(str, SV("\t.byte "));
  string_append_u64(str, value);
  string_append(str, SV("\n"));
}

void directive_zero(u64 bytes, String *restrict str) {
  string_append(str, SV("\t.zero "));
  string_append_u64(str, bytes);
  string_append(str, SV("\n"));
}

void directive_string(StringView sv, String *restrict str) {
  string_append(str, SV("\t.string \""));
  string_append(str, sv);
  string_append(str, SV("\"\n"));
}

void directive_label(StringView name, String *restrict str) {
  string_append(str, name);
  string_append(str, SV(":\n"));
}
