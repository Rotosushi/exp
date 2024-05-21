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
#include <stddef.h>

#include "backend/x64/registers.h"

StringView x64_gpr_to_sv(x64_GPR r) {
  switch (r) {
  case X64GPR_RAX: return SV("rax");
  case X64GPR_RBX: return SV("rbx");
  case X64GPR_RCX: return SV("rcx");
  case X64GPR_RDX: return SV("rdx");
  case X64GPR_RBP: return SV("rbp");
  case X64GPR_RSI: return SV("rsi");
  case X64GPR_RDI: return SV("rdi");
  case X64GPR_RSP: return SV("rsp");
  case X64GPR_R8:  return SV("r8");
  case X64GPR_R9:  return SV("r9");
  case X64GPR_R10: return SV("r10");
  case X64GPR_R11: return SV("r11");
  case X64GPR_R12: return SV("r12");
  case X64GPR_R13: return SV("r13");
  case X64GPR_R14: return SV("r14");
  case X64GPR_R15: return SV("r15");
  default:         unreachable();
  }
}