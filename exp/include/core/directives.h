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
#ifndef EXP_BACKEND_AS_DIRECTIVES_H
#define EXP_BACKEND_AS_DIRECTIVES_H

#include "adt/string.h"

/**
 * @brief this directive is used to tell as about the
 * start of a new logical file.
 *
 * @param path
 * @param file
 */
void directive_file(StringView path, String *restrict str);

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
void directive_arch(StringView cpu_type, String *restrict file);

/**
 * @brief tells as to place comments/tags into the
 * produced object files.
 *
 * @note only works on ELF
 *
 * @param comment
 * @param file
 */
void directive_ident(StringView comment, String *restrict file);

/**
 * @brief tells as to mark the stack as noexec,
 *
 * @note only works on GNU systems.
 *
 * @param file
 */
void directive_noexecstack(String *restrict file);

/**
 * @brief defines a new symbol visible to ld for linking,
 * where the definition comes from a label (as far as I can tell.)
 * this means that .global is used for both forward declarations
 * and definitions.
 *
 * @param name
 * @param file
 */
void directive_globl(StringView name, String *restrict file);

/**
 * @brief tells as to assemble the following statements into
 * the data section.
 *
 * @param file
 */
void directive_data(String *restrict file);

/**
 * @brief tells as to assemble the following statements into
 * the bss section.
 *
 * @param file
 */
void directive_bss(String *restrict file);

/**
 * @brief tells as to assemble the following statements into
 * the text section.
 *
 * @param file
 */
void directive_text(String *restrict file);

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
void directive_balign(u64 bytes, String *restrict file);

/**
 * @brief emits the .size <name>, <expression> directive
 *
 * @param name the name of the symbol to associate with the size
 * @param size the size to place in <expression>
 * @param file
 */
void directive_size(StringView name, u64 size, String *restrict file);

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
void directive_size_label_relative(StringView name, String *restrict file);

typedef enum STT_Type {
  STT_FUNC,
  STT_OBJECT,
  STT_TLS,
  STT_COMMON,
} STT_Type;

void directive_type(StringView name, STT_Type kind, String *restrict file);

void directive_quad(i64 value, String *restrict file);

void directive_byte(unsigned char value, String *restrict file);

void directive_zero(u64 bytes, String *restrict file);

void directive_string(StringView sv, String *restrict file);

void directive_label(StringView name, String *restrict file);

#endif // !EXP_BACKEND_AS_DIRECTIVES_H