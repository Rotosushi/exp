// Copyright (C) 2025 Cade Weinberg
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

/**
 * @file targets/x86_64/print/gnu_as_directives.h
 */

#ifndef EXP_BACKEND_AS_DIRECTIVES_H
#define EXP_BACKEND_AS_DIRECTIVES_H

#include "utility/string.h"

/**
 * @brief this directive is used to tell as about the
 * start of a new logical file.
 *
 * @param path
 * @param file
 */
void directive_file(String *buffer, StringView path);

/**
 * @brief This directive specifies the specific
 * architecture of the x86 chip to assemble for.
 * letting as produce more efficient bytecode for the given
 * assembly and causes as produce diagnostics about the usage of
 * features which are not available on the chip.
 *
 * @param cpu_type
 * @param file
 */
void directive_arch(String *buffer, StringView cpu_type);

/**
 * @brief tells as to place comments/tags into the
 * produced object files.
 *
 * @note only works on ELF targets
 *
 * @param comment
 * @param file
 */
void directive_ident(String *buffer, StringView comment);

/**
 * @brief tells as to mark the stack as noexec,
 *
 * @note only works on GNU systems.
 *
 * @param file
 */
void directive_noexecstack(String *buffer);

/**
 * @brief defines a new symbol visible to ld for linking,
 * where the definition comes from a label (as far as I can tell.)
 * this means that .global is used for both forward declarations
 * and definitions.
 *
 * @param name
 * @param file
 */
void directive_globl(String *buffer, StringView name);

/**
 * @brief tells as to assemble the following statements into
 * the data section.
 *
 * @param file
 */
void directive_data(String *buffer);

/**
 * @brief tells as to assemble the following statements into
 * the bss section.
 *
 * @param file
 */
void directive_bss(String *buffer);

/**
 * @brief tells as to assemble the following statements into
 * the text section.
 *
 * @param file
 */
void directive_text(String *buffer);

/**
 * @brief pads the location counter to a particular storage boundary.
 * this causes an allocation which follows the align directive to be
 * emitted at that particular storage boundary.
 *
 * @note balign is specific to GNU as
 *
 * @param type
 * @param file
 */
void directive_balign(String *buffer, u64 bytes);

/**
 * @brief emits the .size <name>, <expression> directive
 *
 * @param name the name of the symbol to associate with the size
 * @param size the size to place in <expression>
 * @param file
 */
void directive_size(String *buffer, StringView name, u64 size);

/**
 * @brief emits a .size directive with a value equal to the
 * difference between the address of th directive and the
 * address of the given label.
 *
 * @warning assumes the label is emitted before the .size directive,
 * and that the label appears immediately before the addresses allocated
 * for the data the label refers to.
 *
 * @param name
 * @param file
 */
void directive_size_label_relative(String *buffer, StringView name);

typedef enum STT_Type {
    STT_FUNC,
    STT_OBJECT,
    STT_TLS,
    STT_COMMON,
} STT_Type;

void directive_type(String *buffer, StringView name, STT_Type kind);

void directive_quad(String *buffer, i64 value);

void directive_byte(String *buffer, unsigned char value);

void directive_zero(String *buffer, u64 bytes);

void directive_string(String *buffer, StringView sv);

void directive_label(String *buffer, StringView name);

#endif // !EXP_BACKEND_AS_DIRECTIVES_H
