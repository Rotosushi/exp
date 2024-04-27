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

#include "backend/register.h"
#include "utility/panic.h"

StringView register_to_sv(Register r) {
  switch (r) {
  case REG_RAX:
    return string_view_from_cstring("rax");
  case REG_RBX:
    return string_view_from_cstring("rbx");
  case REG_RCX:
    return string_view_from_cstring("rcx");
  case REG_RDX:
    return string_view_from_cstring("rdx");
  case REG_RBP:
    return string_view_from_cstring("rbp");
  case REG_RSI:
    return string_view_from_cstring("rsi");
  case REG_RDI:
    return string_view_from_cstring("rdi");
  case REG_RSP:
    return string_view_from_cstring("rsp");
  case REG_R8:
    return string_view_from_cstring("r8");
  case REG_R9:
    return string_view_from_cstring("r9");
  case REG_R10:
    return string_view_from_cstring("r10");
  case REG_R11:
    return string_view_from_cstring("r11");
  case REG_R12:
    return string_view_from_cstring("r12");
  case REG_R13:
    return string_view_from_cstring("r13");
  case REG_R14:
    return string_view_from_cstring("r14");
  case REG_R15:
    return string_view_from_cstring("r15");
  default:
    PANIC("bad register");
  }
}