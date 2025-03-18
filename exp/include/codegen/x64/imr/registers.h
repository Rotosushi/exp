// Copyright (C) 2024 Cade Weinberg
//
// This file is part of exp.
//
// exp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// exp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with exp.  If not, see <https://www.gnu.org/licenses/>.
#ifndef EXP_BACKEND_X64_GPR_H
#define EXP_BACKEND_X64_GPR_H

#include "support/string_view.h"

/**
 * @brief General Purpose Register
 *
 * @note each GPR can be addressed in a few different ways,
 * in order to access them as different sizes. all GPRs can be
 * accessed in a 1 byte, 2 byte, 4 byte, or 8 byte size.
 * in order to alleviate programming with so many names, the
 * synonyms for the physical register are all layed out sequentially,
 * with each successive synonym being the next size up of that same register.
 * this allows code to address any sized register while only having to
 * specify the base register. which can be the generic register name
 * needless to say, only modify this file if you are prepared to follow that
 * rule, so existing code continues to work
 */
 typedef enum x86_64_GPR : u8 {
    X86_64_GPR_rAX,
    // X86_64_GPR_AH,
    X86_64_GPR_AL,
    X86_64_GPR_AX,
    X86_64_GPR_EAX,
    X86_64_GPR_RAX,

    X86_64_GPR_rBX,
    // X86_64_GPR_BH,
    X86_64_GPR_BL,
    X86_64_GPR_BX,
    X86_64_GPR_EBX,
    X86_64_GPR_RBX,

    X86_64_GPR_rCX,
    // X86_64_GPR_CH,
    X86_64_GPR_CL,
    X86_64_GPR_CX,
    X86_64_GPR_ECX,
    X86_64_GPR_RCX,

    X86_64_GPR_rDX,
    // X86_64_GPR_DH,
    X86_64_GPR_DL,
    X86_64_GPR_DX,
    X86_64_GPR_EDX,
    X86_64_GPR_RDX,

    X86_64_GPR_rSI,
    X86_64_GPR_SI,
    X86_64_GPR_SIL,
    X86_64_GPR_ESI,
    X86_64_GPR_RSI,

    X86_64_GPR_rDI,
    X86_64_GPR_DI,
    X86_64_GPR_DIL,
    X86_64_GPR_EDI,
    X86_64_GPR_RDI,

    X86_64_GPR_rBP,
    X86_64_GPR_BP,
    X86_64_GPR_BPL,
    X86_64_GPR_EBP,
    X86_64_GPR_RBP,

    X86_64_GPR_rSP,
    X86_64_GPR_SP,
    X86_64_GPR_SPL,
    X86_64_GPR_ESP,
    X86_64_GPR_RSP,

    X86_64_GPR_r8,
    X86_64_GPR_R8B,
    X86_64_GPR_R8W,
    X86_64_GPR_R8D,
    X86_64_GPR_R8,

    X86_64_GPR_r9,
    X86_64_GPR_R9B,
    X86_64_GPR_R9W,
    X86_64_GPR_R9D,
    X86_64_GPR_R9,

    X86_64_GPR_r10,
    X86_64_GPR_R10B,
    X86_64_GPR_R10W,
    X86_64_GPR_R10D,
    X86_64_GPR_R10,

    X86_64_GPR_r11,
    X86_64_GPR_R11B,
    X86_64_GPR_R11W,
    X86_64_GPR_R11D,
    X86_64_GPR_R11,

    X86_64_GPR_r12,
    X86_64_GPR_R12B,
    X86_64_GPR_R12W,
    X86_64_GPR_R12D,
    X86_64_GPR_R12,

    X86_64_GPR_r13,
    X86_64_GPR_R13B,
    X86_64_GPR_R13W,
    X86_64_GPR_R13D,
    X86_64_GPR_R13,

    X86_64_GPR_r14,
    X86_64_GPR_R14B,
    X86_64_GPR_R14W,
    X86_64_GPR_R14D,
    X86_64_GPR_R14,

    X86_64_GPR_r15,
    X86_64_GPR_R15B,
    X86_64_GPR_R15W,
    X86_64_GPR_R15D,
    X86_64_GPR_R15,
} x86_64_GPR;

StringView x86_64_gpr_to_sv(x86_64_GPR r);
x86_64_GPR x86_64_gpr_with_size(u8 gpr_index, u64 size);
x86_64_GPR x86_64_gpr_resize(x86_64_GPR gpr, u64 size);
x86_64_GPR x86_64_gpr_scalar_argument(u8 argument_index, u64 size);
bool x86_64_gpr_valid_size(u64 size);
bool x86_64_gpr_is_sized(x86_64_GPR gpr);
bool x86_64_gpr_overlap(x86_64_GPR A, x86_64_GPR B);
u8 x86_64_gpr_size(x86_64_GPR gpr);
u8 x86_64_gpr_index(x86_64_GPR gpr);


#endif // !EXP_BACKEND_X64_GPR_H
