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

#include "utility/string_view.h"

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
typedef enum x64_GPR : u8 {
    X64_GPR_rAX,
    // X64_GPR_AH,
    X64_GPR_AL,
    X64_GPR_AX,
    X64_GPR_EAX,
    X64_GPR_RAX,

    X64_GPR_rBX,
    // X64_GPR_BH,
    X64_GPR_BL,
    X64_GPR_BX,
    X64_GPR_EBX,
    X64_GPR_RBX,

    X64_GPR_rCX,
    // X64_GPR_CH,
    X64_GPR_CL,
    X64_GPR_CX,
    X64_GPR_ECX,
    X64_GPR_RCX,

    X64_GPR_rDX,
    // X64_GPR_DH,
    X64_GPR_DL,
    X64_GPR_DX,
    X64_GPR_EDX,
    X64_GPR_RDX,

    X64_GPR_rSI,
    X64_GPR_SI,
    X64_GPR_SIL,
    X64_GPR_ESI,
    X64_GPR_RSI,

    X64_GPR_rDI,
    X64_GPR_DI,
    X64_GPR_DIL,
    X64_GPR_EDI,
    X64_GPR_RDI,

    X64_GPR_rBP,
    X64_GPR_BP,
    X64_GPR_BPL,
    X64_GPR_EBP,
    X64_GPR_RBP,

    X64_GPR_rSP,
    X64_GPR_SP,
    X64_GPR_SPL,
    X64_GPR_ESP,
    X64_GPR_RSP,

    X64_GPR_r8,
    X64_GPR_R8B,
    X64_GPR_R8W,
    X64_GPR_R8D,
    X64_GPR_R8,

    X64_GPR_r9,
    X64_GPR_R9B,
    X64_GPR_R9W,
    X64_GPR_R9D,
    X64_GPR_R9,

    X64_GPR_r10,
    X64_GPR_R10B,
    X64_GPR_R10W,
    X64_GPR_R10D,
    X64_GPR_R10,

    X64_GPR_r11,
    X64_GPR_R11B,
    X64_GPR_R11W,
    X64_GPR_R11D,
    X64_GPR_R11,

    X64_GPR_r12,
    X64_GPR_R12B,
    X64_GPR_R12W,
    X64_GPR_R12D,
    X64_GPR_R12,

    X64_GPR_r13,
    X64_GPR_R13B,
    X64_GPR_R13W,
    X64_GPR_R13D,
    X64_GPR_R13,

    X64_GPR_r14,
    X64_GPR_R14B,
    X64_GPR_R14W,
    X64_GPR_R14D,
    X64_GPR_R14,

    X64_GPR_r15,
    X64_GPR_R15B,
    X64_GPR_R15W,
    X64_GPR_R15D,
    X64_GPR_R15,
} x64_GPR;

StringView x64_gpr_to_sv(x64_GPR r);
x64_GPR x64_gpr_with_size(u8 gpr_index, u64 size);
x64_GPR x64_gpr_resize(x64_GPR gpr, u64 size);
x64_GPR x64_gpr_scalar_argument(u8 argument_index, u64 size);
bool x64_gpr_overlap(x64_GPR A, x64_GPR B);
u8 x64_gpr_size(x64_GPR gpr);
u8 x64_gpr_index(x64_GPR gpr);

#endif // !EXP_BACKEND_X64_GPR_H
