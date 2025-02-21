/**
 * Copyright (C) 2025 Cade Weinberg
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

/**
 * @file targets/x86_64/imr/registers.c
 */

#include "targets/x86_64/imr/registers.h"
#include "utility/assert.h"
#include "utility/unreachable.h"

void print_x86_64_gpr(String *buffer, x86_64_GPR gpr) {
    EXP_ASSERT(buffer != nullptr);
    string_append(buffer, SV("%"));
    string_append(buffer, x86_64_gpr_to_sv(gpr));
}

StringView x86_64_gpr_to_sv(x86_64_GPR gpr) {
    switch (gpr) {
    case X86_64_GPR_rAX: EXP_UNREACHABLE();
    case X86_64_GPR_AL:  return SV("al");
    case X86_64_GPR_AX:  return SV("ax");
    case X86_64_GPR_EAX: return SV("eax");
    case X86_64_GPR_RAX: return SV("rax");

    case X86_64_GPR_rBX: EXP_UNREACHABLE();
    case X86_64_GPR_BL:  return SV("bl");
    case X86_64_GPR_BX:  return SV("bx");
    case X86_64_GPR_EBX: return SV("ebx");
    case X86_64_GPR_RBX: return SV("rbx");

    case X86_64_GPR_rCX: EXP_UNREACHABLE();
    case X86_64_GPR_CL:  return SV("cl");
    case X86_64_GPR_CX:  return SV("cx");
    case X86_64_GPR_ECX: return SV("ecx");
    case X86_64_GPR_RCX: return SV("rcx");

    case X86_64_GPR_rDX: EXP_UNREACHABLE();
    case X86_64_GPR_DL:  return SV("dl");
    case X86_64_GPR_DX:  return SV("dx");
    case X86_64_GPR_EDX: return SV("edx");
    case X86_64_GPR_RDX: return SV("rdx");

    case X86_64_GPR_rSI: EXP_UNREACHABLE();
    case X86_64_GPR_SI:  return SV("si");
    case X86_64_GPR_SIL: return SV("sil");
    case X86_64_GPR_ESI: return SV("esi");
    case X86_64_GPR_RSI: return SV("rsi");

    case X86_64_GPR_rDI: EXP_UNREACHABLE();
    case X86_64_GPR_DI:  return SV("di");
    case X86_64_GPR_DIL: return SV("dil");
    case X86_64_GPR_EDI: return SV("edi");
    case X86_64_GPR_RDI: return SV("rdi");

    case X86_64_GPR_rBP: EXP_UNREACHABLE();
    case X86_64_GPR_BP:  return SV("bp");
    case X86_64_GPR_BPL: return SV("bpl");
    case X86_64_GPR_EBP: return SV("ebp");
    case X86_64_GPR_RBP: return SV("rbp");

    case X86_64_GPR_rSP: EXP_UNREACHABLE();
    case X86_64_GPR_SP:  return SV("sp");
    case X86_64_GPR_SPL: return SV("spl");
    case X86_64_GPR_ESP: return SV("esp");
    case X86_64_GPR_RSP: return SV("rsp");

    case X86_64_GPR_r8:  EXP_UNREACHABLE();
    case X86_64_GPR_R8B: return SV("r8b");
    case X86_64_GPR_R8W: return SV("r8w");
    case X86_64_GPR_R8D: return SV("r8d");
    case X86_64_GPR_R8:  return SV("r8");

    case X86_64_GPR_r9:  EXP_UNREACHABLE();
    case X86_64_GPR_R9B: return SV("r9b");
    case X86_64_GPR_R9W: return SV("r9w");
    case X86_64_GPR_R9D: return SV("r9d");
    case X86_64_GPR_R9:  return SV("r9");

    case X86_64_GPR_r10:  EXP_UNREACHABLE();
    case X86_64_GPR_R10B: return SV("r10b");
    case X86_64_GPR_R10W: return SV("r10w");
    case X86_64_GPR_R10D: return SV("r10d");
    case X86_64_GPR_R10:  return SV("r10");

    case X86_64_GPR_r11:  EXP_UNREACHABLE();
    case X86_64_GPR_R11B: return SV("r11b");
    case X86_64_GPR_R11W: return SV("r11w");
    case X86_64_GPR_R11D: return SV("r11d");
    case X86_64_GPR_R11:  return SV("r11");

    case X86_64_GPR_r12:  EXP_UNREACHABLE();
    case X86_64_GPR_R12B: return SV("r12b");
    case X86_64_GPR_R12W: return SV("r12w");
    case X86_64_GPR_R12D: return SV("r12d");
    case X86_64_GPR_R12:  return SV("r12");

    case X86_64_GPR_r13:  EXP_UNREACHABLE();
    case X86_64_GPR_R13B: return SV("r13b");
    case X86_64_GPR_R13W: return SV("r13w");
    case X86_64_GPR_R13D: return SV("r13d");
    case X86_64_GPR_R13:  return SV("r13");

    case X86_64_GPR_r14:  EXP_UNREACHABLE();
    case X86_64_GPR_R14B: return SV("r14b");
    case X86_64_GPR_R14W: return SV("r14w");
    case X86_64_GPR_R14D: return SV("r14d");
    case X86_64_GPR_R14:  return SV("r14");

    case X86_64_GPR_r15:  EXP_UNREACHABLE();
    case X86_64_GPR_R15B: return SV("r15b");
    case X86_64_GPR_R15W: return SV("r15w");
    case X86_64_GPR_R15D: return SV("r15d");
    case X86_64_GPR_R15:  return SV("r15");

    default: EXP_UNREACHABLE();
    }
}

static bool valid_gpr_size(u64 size) {
    return (size == 1) || (size == 2) || (size == 4) || (size == 8);
}

static x86_64_GPR size_to_gpr(x86_64_GPR base, u64 size) {
    EXP_ASSERT(valid_gpr_size(size));
    if (size == 1) return base + 1;
    else if (size == 2) return base + 2;
    else if (size <= 4) return base + 3;
    else return base + 4;
}

x86_64_GPR x86_64_gpr_with_size(u8 gpr_index, u64 size) {
    EXP_ASSERT(valid_gpr_size(size));
    switch (gpr_index) {
    case 0:  return size_to_gpr(X86_64_GPR_rAX, size);
    case 1:  return size_to_gpr(X86_64_GPR_rBX, size);
    case 2:  return size_to_gpr(X86_64_GPR_rCX, size);
    case 3:  return size_to_gpr(X86_64_GPR_rDX, size);
    case 4:  return size_to_gpr(X86_64_GPR_rSI, size);
    case 5:  return size_to_gpr(X86_64_GPR_rDI, size);
    case 6:  return size_to_gpr(X86_64_GPR_rBP, size);
    case 7:  return size_to_gpr(X86_64_GPR_rSP, size);
    case 8:  return size_to_gpr(X86_64_GPR_r8, size);
    case 9:  return size_to_gpr(X86_64_GPR_r9, size);
    case 10: return size_to_gpr(X86_64_GPR_r10, size);
    case 11: return size_to_gpr(X86_64_GPR_r11, size);
    case 12: return size_to_gpr(X86_64_GPR_r12, size);
    case 13: return size_to_gpr(X86_64_GPR_r13, size);
    case 14: return size_to_gpr(X86_64_GPR_r14, size);
    case 15: return size_to_gpr(X86_64_GPR_r15, size);
    default: EXP_UNREACHABLE();
    }
}

x86_64_GPR x86_64_gpr_resize(x86_64_GPR gpr, u64 size) {
    EXP_ASSERT(valid_gpr_size(size));
    switch (gpr) {
    case X86_64_GPR_rAX:
    case X86_64_GPR_AL:
    case X86_64_GPR_AX:
    case X86_64_GPR_EAX:
    case X86_64_GPR_RAX: return size_to_gpr(X86_64_GPR_rAX, size);

    case X86_64_GPR_rBX:
    case X86_64_GPR_BL:
    case X86_64_GPR_BX:
    case X86_64_GPR_EBX:
    case X86_64_GPR_RBX: return size_to_gpr(X86_64_GPR_rBX, size);

    case X86_64_GPR_rCX:
    case X86_64_GPR_CL:
    case X86_64_GPR_CX:
    case X86_64_GPR_ECX:
    case X86_64_GPR_RCX: return size_to_gpr(X86_64_GPR_rCX, size);

    case X86_64_GPR_rDX:
    case X86_64_GPR_DL:
    case X86_64_GPR_DX:
    case X86_64_GPR_EDX:
    case X86_64_GPR_RDX: return size_to_gpr(X86_64_GPR_rDX, size);

    case X86_64_GPR_rSI:
    case X86_64_GPR_SI:
    case X86_64_GPR_SIL:
    case X86_64_GPR_ESI:
    case X86_64_GPR_RSI: return size_to_gpr(X86_64_GPR_rSI, size);

    case X86_64_GPR_rDI:
    case X86_64_GPR_DI:
    case X86_64_GPR_DIL:
    case X86_64_GPR_EDI:
    case X86_64_GPR_RDI: return size_to_gpr(X86_64_GPR_rDI, size);

    case X86_64_GPR_rBP:
    case X86_64_GPR_BP:
    case X86_64_GPR_BPL:
    case X86_64_GPR_EBP:
    case X86_64_GPR_RBP: return size_to_gpr(X86_64_GPR_rBP, size);

    case X86_64_GPR_rSP:
    case X86_64_GPR_SP:
    case X86_64_GPR_SPL:
    case X86_64_GPR_ESP:
    case X86_64_GPR_RSP: return size_to_gpr(X86_64_GPR_rSP, size);

    case X86_64_GPR_r8:
    case X86_64_GPR_R8B:
    case X86_64_GPR_R8W:
    case X86_64_GPR_R8D:
    case X86_64_GPR_R8:  return size_to_gpr(X86_64_GPR_r8, size);

    case X86_64_GPR_r9:
    case X86_64_GPR_R9B:
    case X86_64_GPR_R9W:
    case X86_64_GPR_R9D:
    case X86_64_GPR_R9:  return size_to_gpr(X86_64_GPR_r9, size);

    case X86_64_GPR_r10:
    case X86_64_GPR_R10B:
    case X86_64_GPR_R10W:
    case X86_64_GPR_R10D:
    case X86_64_GPR_R10:  return size_to_gpr(X86_64_GPR_r10, size);

    case X86_64_GPR_r11:
    case X86_64_GPR_R11B:
    case X86_64_GPR_R11W:
    case X86_64_GPR_R11D:
    case X86_64_GPR_R11:  return size_to_gpr(X86_64_GPR_r11, size);

    case X86_64_GPR_r12:
    case X86_64_GPR_R12B:
    case X86_64_GPR_R12W:
    case X86_64_GPR_R12D:
    case X86_64_GPR_R12:  return size_to_gpr(X86_64_GPR_r12, size);

    case X86_64_GPR_r13:
    case X86_64_GPR_R13B:
    case X86_64_GPR_R13W:
    case X86_64_GPR_R13D:
    case X86_64_GPR_R13:  return size_to_gpr(X86_64_GPR_r13, size);

    case X86_64_GPR_r14:
    case X86_64_GPR_R14B:
    case X86_64_GPR_R14W:
    case X86_64_GPR_R14D:
    case X86_64_GPR_R14:  return size_to_gpr(X86_64_GPR_r14, size);

    case X86_64_GPR_r15:
    case X86_64_GPR_R15B:
    case X86_64_GPR_R15W:
    case X86_64_GPR_R15D:
    case X86_64_GPR_R15:  return size_to_gpr(X86_64_GPR_r15, size);

    default: EXP_UNREACHABLE();
    }
}

x86_64_GPR x86_64_gpr_scalar_argument(u8 argument_index, u64 size) {
    EXP_ASSERT(valid_gpr_size(size));
    switch (argument_index) {
    case 0: return x86_64_gpr_with_size(X86_64_GPR_rDI, size);
    case 1: return x86_64_gpr_with_size(X86_64_GPR_rSI, size);
    case 2: return x86_64_gpr_with_size(X86_64_GPR_rDX, size);
    case 3: return x86_64_gpr_with_size(X86_64_GPR_rCX, size);
    case 4: return x86_64_gpr_with_size(X86_64_GPR_r8, size);
    case 5: return x86_64_gpr_with_size(X86_64_GPR_r9, size);
    // the rest of the arguments are passed on the stack.
    default: EXP_UNREACHABLE();
    }
}

static bool overlap_(x86_64_GPR gpr, x86_64_GPR B) {
    return (B >= gpr) && (B <= (gpr + 4));
}

bool x86_64_gpr_overlap(x86_64_GPR A, x86_64_GPR B) {
    switch (A) {
    case X86_64_GPR_rAX:
    case X86_64_GPR_AL:
    case X86_64_GPR_AX:
    case X86_64_GPR_EAX:
    case X86_64_GPR_RAX: return overlap_(X86_64_GPR_rAX, B);

    case X86_64_GPR_rBX:
    case X86_64_GPR_BL:
    case X86_64_GPR_BX:
    case X86_64_GPR_EBX:
    case X86_64_GPR_RBX: return overlap_(X86_64_GPR_rBX, B);

    case X86_64_GPR_rCX:
    case X86_64_GPR_CL:
    case X86_64_GPR_CX:
    case X86_64_GPR_ECX:
    case X86_64_GPR_RCX: return overlap_(X86_64_GPR_rCX, B);

    case X86_64_GPR_rDX:
    case X86_64_GPR_DL:
    case X86_64_GPR_DX:
    case X86_64_GPR_EDX:
    case X86_64_GPR_RDX: return overlap_(X86_64_GPR_rDX, B);

    case X86_64_GPR_rSI:
    case X86_64_GPR_SI:
    case X86_64_GPR_SIL:
    case X86_64_GPR_ESI:
    case X86_64_GPR_RSI: return overlap_(X86_64_GPR_rSI, B);

    case X86_64_GPR_rDI:
    case X86_64_GPR_DI:
    case X86_64_GPR_DIL:
    case X86_64_GPR_EDI:
    case X86_64_GPR_RDI: return overlap_(X86_64_GPR_rDI, B);

    case X86_64_GPR_rBP:
    case X86_64_GPR_BP:
    case X86_64_GPR_BPL:
    case X86_64_GPR_EBP:
    case X86_64_GPR_RBP: return overlap_(X86_64_GPR_rBP, B);

    case X86_64_GPR_rSP:
    case X86_64_GPR_SP:
    case X86_64_GPR_SPL:
    case X86_64_GPR_ESP:
    case X86_64_GPR_RSP: return overlap_(X86_64_GPR_rSP, B);

    case X86_64_GPR_r8:
    case X86_64_GPR_R8B:
    case X86_64_GPR_R8W:
    case X86_64_GPR_R8D:
    case X86_64_GPR_R8:  return overlap_(X86_64_GPR_r8, B);

    case X86_64_GPR_r9:
    case X86_64_GPR_R9B:
    case X86_64_GPR_R9W:
    case X86_64_GPR_R9D:
    case X86_64_GPR_R9:  return overlap_(X86_64_GPR_r9, B);

    case X86_64_GPR_r10:
    case X86_64_GPR_R10B:
    case X86_64_GPR_R10W:
    case X86_64_GPR_R10D:
    case X86_64_GPR_R10:  return overlap_(X86_64_GPR_r10, B);

    case X86_64_GPR_r11:
    case X86_64_GPR_R11B:
    case X86_64_GPR_R11W:
    case X86_64_GPR_R11D:
    case X86_64_GPR_R11:  return overlap_(X86_64_GPR_r11, B);

    case X86_64_GPR_r12:
    case X86_64_GPR_R12B:
    case X86_64_GPR_R12W:
    case X86_64_GPR_R12D:
    case X86_64_GPR_R12:  return overlap_(X86_64_GPR_r12, B);

    case X86_64_GPR_r13:
    case X86_64_GPR_R13B:
    case X86_64_GPR_R13W:
    case X86_64_GPR_R13D:
    case X86_64_GPR_R13:  return overlap_(X86_64_GPR_r13, B);

    case X86_64_GPR_r14:
    case X86_64_GPR_R14B:
    case X86_64_GPR_R14W:
    case X86_64_GPR_R14D:
    case X86_64_GPR_R14:  return overlap_(X86_64_GPR_r14, B);

    case X86_64_GPR_r15:
    case X86_64_GPR_R15B:
    case X86_64_GPR_R15W:
    case X86_64_GPR_R15D:
    case X86_64_GPR_R15:  return overlap_(X86_64_GPR_r15, B);

    default: EXP_UNREACHABLE();
    }
}

u8 x86_64_gpr_size(x86_64_GPR gpr) {
    switch (gpr) {
    case X86_64_GPR_rAX: EXP_UNREACHABLE();
    case X86_64_GPR_AL:  return 1;
    case X86_64_GPR_AX:  return 2;
    case X86_64_GPR_EAX: return 4;
    case X86_64_GPR_RAX: return 8;

    case X86_64_GPR_rBX: EXP_UNREACHABLE();
    case X86_64_GPR_BL:  return 1;
    case X86_64_GPR_BX:  return 2;
    case X86_64_GPR_EBX: return 4;
    case X86_64_GPR_RBX: return 8;

    case X86_64_GPR_rCX: EXP_UNREACHABLE();
    case X86_64_GPR_CL:  return 1;
    case X86_64_GPR_CX:  return 2;
    case X86_64_GPR_ECX: return 4;
    case X86_64_GPR_RCX: return 8;

    case X86_64_GPR_rDX: EXP_UNREACHABLE();
    case X86_64_GPR_DL:  return 1;
    case X86_64_GPR_DX:  return 2;
    case X86_64_GPR_EDX: return 4;
    case X86_64_GPR_RDX: return 8;

    case X86_64_GPR_rSI: EXP_UNREACHABLE();
    case X86_64_GPR_SI:  return 1;
    case X86_64_GPR_SIL: return 2;
    case X86_64_GPR_ESI: return 4;
    case X86_64_GPR_RSI: return 8;

    case X86_64_GPR_rDI: EXP_UNREACHABLE();
    case X86_64_GPR_DI:  return 1;
    case X86_64_GPR_DIL: return 2;
    case X86_64_GPR_EDI: return 4;
    case X86_64_GPR_RDI: return 8;

    case X86_64_GPR_rBP: EXP_UNREACHABLE();
    case X86_64_GPR_BP:  return 1;
    case X86_64_GPR_BPL: return 2;
    case X86_64_GPR_EBP: return 4;
    case X86_64_GPR_RBP: return 8;

    case X86_64_GPR_rSP: EXP_UNREACHABLE();
    case X86_64_GPR_SP:  return 1;
    case X86_64_GPR_SPL: return 2;
    case X86_64_GPR_ESP: return 4;
    case X86_64_GPR_RSP: return 8;

    case X86_64_GPR_r8:  EXP_UNREACHABLE();
    case X86_64_GPR_R8B: return 1;
    case X86_64_GPR_R8W: return 2;
    case X86_64_GPR_R8D: return 4;
    case X86_64_GPR_R8:  return 8;

    case X86_64_GPR_r9:  EXP_UNREACHABLE();
    case X86_64_GPR_R9B: return 1;
    case X86_64_GPR_R9W: return 2;
    case X86_64_GPR_R9D: return 4;
    case X86_64_GPR_R9:  return 8;

    case X86_64_GPR_r10:  EXP_UNREACHABLE();
    case X86_64_GPR_R10B: return 1;
    case X86_64_GPR_R10W: return 2;
    case X86_64_GPR_R10D: return 4;
    case X86_64_GPR_R10:  return 8;

    case X86_64_GPR_r11:  EXP_UNREACHABLE();
    case X86_64_GPR_R11B: return 1;
    case X86_64_GPR_R11W: return 2;
    case X86_64_GPR_R11D: return 4;
    case X86_64_GPR_R11:  return 8;

    case X86_64_GPR_r12:  EXP_UNREACHABLE();
    case X86_64_GPR_R12B: return 1;
    case X86_64_GPR_R12W: return 2;
    case X86_64_GPR_R12D: return 4;
    case X86_64_GPR_R12:  return 8;

    case X86_64_GPR_r13:  EXP_UNREACHABLE();
    case X86_64_GPR_R13B: return 1;
    case X86_64_GPR_R13W: return 2;
    case X86_64_GPR_R13D: return 4;
    case X86_64_GPR_R13:  return 8;

    case X86_64_GPR_r14:  EXP_UNREACHABLE();
    case X86_64_GPR_R14B: return 1;
    case X86_64_GPR_R14W: return 2;
    case X86_64_GPR_R14D: return 4;
    case X86_64_GPR_R14:  return 8;

    case X86_64_GPR_r15:  EXP_UNREACHABLE();
    case X86_64_GPR_R15B: return 1;
    case X86_64_GPR_R15W: return 2;
    case X86_64_GPR_R15D: return 4;
    case X86_64_GPR_R15:  return 8;

    default: EXP_UNREACHABLE();
    }
}

u8 x86_64_gpr_index(x86_64_GPR gpr) {
    switch (gpr) {
    case X86_64_GPR_rAX:
    case X86_64_GPR_AL:
    case X86_64_GPR_AX:
    case X86_64_GPR_EAX:
    case X86_64_GPR_RAX: return 0;

    case X86_64_GPR_rBX:
    case X86_64_GPR_BL:
    case X86_64_GPR_BX:
    case X86_64_GPR_EBX:
    case X86_64_GPR_RBX: return 1;

    case X86_64_GPR_rCX:
    case X86_64_GPR_CL:
    case X86_64_GPR_CX:
    case X86_64_GPR_ECX:
    case X86_64_GPR_RCX: return 2;

    case X86_64_GPR_rDX:
    case X86_64_GPR_DL:
    case X86_64_GPR_DX:
    case X86_64_GPR_EDX:
    case X86_64_GPR_RDX: return 3;

    case X86_64_GPR_rSI:
    case X86_64_GPR_SI:
    case X86_64_GPR_SIL:
    case X86_64_GPR_ESI:
    case X86_64_GPR_RSI: return 4;

    case X86_64_GPR_rDI:
    case X86_64_GPR_DI:
    case X86_64_GPR_DIL:
    case X86_64_GPR_EDI:
    case X86_64_GPR_RDI: return 5;

    case X86_64_GPR_rBP:
    case X86_64_GPR_BP:
    case X86_64_GPR_BPL:
    case X86_64_GPR_EBP:
    case X86_64_GPR_RBP: return 6;

    case X86_64_GPR_rSP:
    case X86_64_GPR_SP:
    case X86_64_GPR_SPL:
    case X86_64_GPR_ESP:
    case X86_64_GPR_RSP: return 7;

    case X86_64_GPR_r8:
    case X86_64_GPR_R8B:
    case X86_64_GPR_R8W:
    case X86_64_GPR_R8D:
    case X86_64_GPR_R8:  return 8;

    case X86_64_GPR_r9:
    case X86_64_GPR_R9B:
    case X86_64_GPR_R9W:
    case X86_64_GPR_R9D:
    case X86_64_GPR_R9:  return 9;

    case X86_64_GPR_r10:
    case X86_64_GPR_R10B:
    case X86_64_GPR_R10W:
    case X86_64_GPR_R10D:
    case X86_64_GPR_R10:  return 10;

    case X86_64_GPR_r11:
    case X86_64_GPR_R11B:
    case X86_64_GPR_R11W:
    case X86_64_GPR_R11D:
    case X86_64_GPR_R11:  return 11;

    case X86_64_GPR_r12:
    case X86_64_GPR_R12B:
    case X86_64_GPR_R12W:
    case X86_64_GPR_R12D:
    case X86_64_GPR_R12:  return 12;

    case X86_64_GPR_r13:
    case X86_64_GPR_R13B:
    case X86_64_GPR_R13W:
    case X86_64_GPR_R13D:
    case X86_64_GPR_R13:  return 13;

    case X86_64_GPR_r14:
    case X86_64_GPR_R14B:
    case X86_64_GPR_R14W:
    case X86_64_GPR_R14D:
    case X86_64_GPR_R14:  return 14;

    case X86_64_GPR_r15:
    case X86_64_GPR_R15B:
    case X86_64_GPR_R15W:
    case X86_64_GPR_R15D:
    case X86_64_GPR_R15:  return 15;

    default: EXP_UNREACHABLE();
    }
}
