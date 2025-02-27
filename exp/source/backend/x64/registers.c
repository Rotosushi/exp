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
#include "utility/unreachable.h"

StringView x64_gpr_to_sv(x64_GPR r) {
    switch (r) {
    case X64_GPR_RAX: return SV("rax");
    case X64_GPR_RBX: return SV("rbx");
    case X64_GPR_RCX: return SV("rcx");
    case X64_GPR_RDX: return SV("rdx");
    case X64_GPR_RBP: return SV("rbp");
    case X64_GPR_RSI: return SV("rsi");
    case X64_GPR_RDI: return SV("rdi");
    case X64_GPR_RSP: return SV("rsp");
    case X64_GPR_R8:  return SV("r8");
    case X64_GPR_R9:  return SV("r9");
    case X64_GPR_R10: return SV("r10");
    case X64_GPR_R11: return SV("r11");
    case X64_GPR_R12: return SV("r12");
    case X64_GPR_R13: return SV("r13");
    case X64_GPR_R14: return SV("r14");
    case X64_GPR_R15: return SV("r15");
    default:          EXP_UNREACHABLE();
    }
}

x64_GPR x64_scalar_argument_gpr(u8 index) {
    switch (index) {
    case 0: return X64_GPR_RDI;
    case 1: return X64_GPR_RSI;
    case 2: return X64_GPR_RDX;
    case 3: return X64_GPR_RCX;
    case 4: return X64_GPR_R8;
    case 5: return X64_GPR_R9;
    // the rest of the arguments are passed on the stack.
    default: EXP_UNREACHABLE();
    }
}
