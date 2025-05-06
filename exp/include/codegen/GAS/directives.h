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
#ifndef EXP_BACKEND_GAS_DIRECTIVES_H
#define EXP_BACKEND_GAS_DIRECTIVES_H

#include "support/string.h"

/**
 * @brief this directive is used to tell as about the
 * start of a new logical file.
 */
void gas_directive_file(StringView path, String *restrict buffer);

/**
 * @brief this directive informs the assembler that the following
 * assembly instructions use the Intel x86 syntax.
 */
void gas_directive_intel_syntax(String *restrict buffer);

/**
 * @brief This directive specifies the specific
 * architecture of the x86 chip to assemble for.
 * letting as produce more efficient bytecode for the given
 * assembly and causes as produce diagnostics about the usage of
 * features which are not available on the chip.
 */
void gas_directive_arch(StringView cpu_type, String *restrict buffer);

/**
 * @brief tells as to place comments/tags into the
 * produced object files.
 *
 * @note only works on ELF
 */
void gas_directive_ident(StringView comment, String *restrict buffer);

/**
 * @brief tells as to mark the stack as noexec,
 *
 * @note only works on GNU systems.
 */
void gas_directive_noexecstack(String *restrict buffer);

/**
 * @brief defines a new symbol visible to ld for linking,
 * where the definition comes from a label (as far as I can tell.)
 * this means that .global is used for both forward declarations
 * and definitions.
 */
void gas_directive_globl(StringView name, String *restrict buffer);

/**
 * @brief tells as to assemble the following statements into
 * the data section.
 */
void gas_directive_data(String *restrict buffer);

/**
 * @brief tells as to assemble the following statements into
 * the bss section.
 */
void gas_directive_bss(String *restrict buffer);

/**
 * @brief tells as to assemble the following statements into
 * the text section.
 */
void gas_directive_text(String *restrict buffer);

/**
 * @brief pads the location counter to a particular storage boundary.
 * this causes an allocation which follows the align directive to be
 * emitted at that particular storage boundary.
 *
 * @note balign is specific to GNU as
 */
void gas_directive_balign(u64 bytes, String *restrict buffer);

/**
 * @brief emits the .size <name>, <expression> directive
 */
void gas_directive_size(StringView name, u64 size, String *restrict buffer);

/**
 * @brief emits the .align <name>, <expression> directive
 */
void gas_directive_align(StringView name, u64 align, String *restrict buffer);

/**
 * @brief emits a .size directive with a value equal to the
 * difference between the address of th directive and the
 * address of the given label.
 *
 * @warning assumes the label is emitted before the .size directive,
 * and that the label appears immediately before the addresses allocated
 * for the data the label refers to.
 */
void gas_directive_size_label_relative(StringView name,
                                       String *restrict buffer);

typedef enum STT_Type {
    STT_FUNC,
    STT_OBJECT,
    STT_TLS,
    STT_COMMON,
} STT_Type;

void gas_directive_type(StringView name,
                        STT_Type   kind,
                        String *restrict buffer);

void gas_directive_u8(u8 value, String *restrict buffer);
void gas_directive_i8(i8 value, String *restrict buffer);
void gas_directive_u16(u16 value, String *restrict buffer);
void gas_directive_i16(i16 value, String *restrict buffer);
void gas_directive_u32(u32 value, String *restrict buffer);
void gas_directive_i32(i32 value, String *restrict buffer);
void gas_directive_u64(u64 value, String *restrict buffer);
void gas_directive_i64(i64 value, String *restrict buffer);

void gas_directive_zero(u64 bytes, String *restrict buffer);

void gas_directive_string(StringView sv, String *restrict buffer);

void gas_directive_label(StringView name, String *restrict buffer);

#endif // !EXP_BACKEND_GAS_DIRECTIVES_H
