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
#ifndef EXP_BACKEND_X86_GPR_H
#define EXP_BACKEND_X86_GPR_H

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
 * specify the base register. which can be the generic register name.
 * All that to say, only modify this file if you are prepared to follow that
 * rule, so existing code continues to work. or refactor the entirety of
 * registers.c to use a different system.
 * which may need to occur regardless, because AH, BH, CH, and DH are not
 * currently supported.
 */
typedef enum x86_64_GPR : u8 {
    X86_GPR_rAX,
    // X86_GPR_AH,
    X86_GPR_AL,
    X86_GPR_AX,
    X86_GPR_EAX,
    X86_GPR_RAX,

    X86_GPR_rBX,
    // X86_GPR_BH,
    X86_GPR_BL,
    X86_GPR_BX,
    X86_GPR_EBX,
    X86_GPR_RBX,

    X86_GPR_rCX,
    // X86_GPR_CH,
    X86_GPR_CL,
    X86_GPR_CX,
    X86_GPR_ECX,
    X86_GPR_RCX,

    X86_GPR_rDX,
    // X86_GPR_DH,
    X86_GPR_DL,
    X86_GPR_DX,
    X86_GPR_EDX,
    X86_GPR_RDX,

    X86_GPR_rSI,
    X86_GPR_SI,
    X86_GPR_SIL,
    X86_GPR_ESI,
    X86_GPR_RSI,

    X86_GPR_rDI,
    X86_GPR_DI,
    X86_GPR_DIL,
    X86_GPR_EDI,
    X86_GPR_RDI,

    X86_GPR_rBP,
    X86_GPR_BP,
    X86_GPR_BPL,
    X86_GPR_EBP,
    X86_GPR_RBP,

    X86_GPR_rSP,
    X86_GPR_SP,
    X86_GPR_SPL,
    X86_GPR_ESP,
    X86_GPR_RSP,

    X86_GPR_r8,
    X86_GPR_R8B,
    X86_GPR_R8W,
    X86_GPR_R8D,
    X86_GPR_R8,

    X86_GPR_r9,
    X86_GPR_R9B,
    X86_GPR_R9W,
    X86_GPR_R9D,
    X86_GPR_R9,

    X86_GPR_r10,
    X86_GPR_R10B,
    X86_GPR_R10W,
    X86_GPR_R10D,
    X86_GPR_R10,

    X86_GPR_r11,
    X86_GPR_R11B,
    X86_GPR_R11W,
    X86_GPR_R11D,
    X86_GPR_R11,

    X86_GPR_r12,
    X86_GPR_R12B,
    X86_GPR_R12W,
    X86_GPR_R12D,
    X86_GPR_R12,

    X86_GPR_r13,
    X86_GPR_R13B,
    X86_GPR_R13W,
    X86_GPR_R13D,
    X86_GPR_R13,

    X86_GPR_r14,
    X86_GPR_R14B,
    X86_GPR_R14W,
    X86_GPR_R14D,
    X86_GPR_R14,

    X86_GPR_r15,
    X86_GPR_R15B,
    X86_GPR_R15W,
    X86_GPR_R15D,
    X86_GPR_R15,
} x86_GPR;

/**
 * @brief convert a GPR to a string view of it's mnemonic
 *
 * @note this returns a static string view, so it should not be modified
 * and is safe to use as a static string.
 *
 * @param gpr the GPR to convert
 * @return a string view of the mnemonic
 */
StringView x86_gpr_mnemonic(x86_GPR gpr);

/**
 * @brief given an index to a GPR, and a size, return the GPR
 * that corresponds to that index and size.
 *
 * @note GPR's are indexed starting at 0, so index >= 0 and index <= 15.
 * 0 -> rAX, 1 -> rBX, 2 -> rCX, 3 -> rDX, 4 -> rSI, 5 -> rDI, 6 -> rBP,
 * 7 -> rSP, 8 -> r8, 9 -> r9, 10 -> r10, 11 -> r11, 12 -> r12, 13 -> r13,
 * 14 -> r14, 15 -> r15
 *
 * @note GPR's are accessable as 1 byte, 2 byte, 4 byte, and 8 byte,
 * so size >= 1 and size <= 8.
 *
 * @param gpr_index the index of the GPR
 * @param size the size of the GPR
 * @return the GPR that corresponds to the index and size
 */
x86_GPR x86_gpr_with_size(u8 gpr_index, u64 size);

/**
 * @brief resize a GPR to the given size
 *
 * @note this will return the same GPR if the GPR is already the correct size
 *
 * @param gpr the GPR to resize
 * @param size the size to resize the GPR to
 * @return the resized GPR
 */
x86_GPR x86_gpr_resize(x86_GPR gpr, u64 size);

/**
 * @brief Get the correct GPR for a scalar argument at position `argument_index`
 * with size `size` according to the System V AMD64 ABI.
 *
 * @note the first 6 arguments are passed in registers, the rest are passed on
 * the stack. So this function only handles argument_index < 6.
 *
 * @todo This function is better suited for a target specific file, built for
 * the System V AMD64 ABI. As it is not specific to x86_64.
 *
 * @param argument_index the index of the argument
 * @param size the size of the argument
 * @return the GPR that corresponds to the argument
 */
x86_GPR x86_gpr_scalar_argument(u8 argument_index, u64 size);

/**
 * @brief check if a size is a valid size for a GPR
 *
 * @note valid sizes are between 1 and 8 inclusive.
 *
 * @param size the size to check
 * @return true if the size is valid, false otherwise
 */
bool x86_gpr_valid_size(u64 size);

/**
 * @brief check if a GPR is a sized GPR
 *
 * @note the generic GPR enumerations are not considered sized,
 * they are used as a size agnostic way to refer to a GPR. for instance
 * X86_64_GPR_rAX is not sized, but X86_64_GPR_RAX is sized.
 * The rAX, rBX, etc. naming convention is used in the official
 * Intel and AMD documentation to refer to the 8-bit, 16-bit, 32-bit,
 * and 64-bit variants of a register as a whole.
 *
 * @param gpr the GPR to check
 * @return true if the GPR is sized, false otherwise
 */
bool x86_gpr_is_sized(x86_GPR gpr);

/**
 * @brief check if two GPRs refer to the same register.
 *
 * @note two registers A and B overlap iff they refer to the same register.
 * Not necessarily that they are the same enumeration.
 * GPRs are enumerated in such a way that this can be checked by simple
 * arithmetic. So it's not much worse than the integer equality.
 *
 * @param A the first GPR
 * @param B the second GPR
 * @return true if the GPRs overlap, false otherwise
 */
bool x86_gpr_overlap(x86_GPR A, x86_GPR B);

/**
 * @brief get the size of a GPR
 *
 * @note the size is only valid for GPRs that are sized.
 * and it is not valid to pass in the unsized enumerations.
 *
 * @param gpr the GPR to get the size of
 * @return the size of the GPR
 */
u8 x86_gpr_size(x86_GPR gpr);

/**
 * @brief get the index of a GPR
 *
 * @note the index is valid for all GPRs, sized and unsized.
 *
 * @param gpr the GPR to get the index of
 * @return the index of the GPR
 */
u8 x86_gpr_index(x86_GPR gpr);

#endif // !EXP_BACKEND_X86_GPR_H
