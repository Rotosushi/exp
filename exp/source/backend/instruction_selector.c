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

#include "backend/instruction_selector.h"
#include "utility/string_view.h"
/*
a global object declaration in assembly looks like:
.globl <name>
.bss | .data
.align <alignment>
.type <name>, @object
.size <name>, <sizeof>
<name>:
.byte <init> | .zero <sizeof> | .quad <init> | .i32 <init> | ...

-- all global symbols can go into the .data section. unless they
are uninitialized, then they are default initialized to zero, and can go into
the .bss section. This holds for constants and variables, it is up to the
compiler to prevent writes to constants.
*/

/**
 * @brief emit the header of the assembly file representing the
 * given context.
 *
 * @note this information includes the directives:
 *  ".file <filename>"
 *  ".arch <cpu-name>"
 *
 * @param file the FILE to write to.
 */
// static void emit_x64_linux_header(Context *restrict context, FILE *file) {
//   StringView path = context_source_path(context);
//   directive_file(path, file);
//   directive_arch(cpu_type, file);
//   file_write("\n", file);
// }

/**
 * @brief emit the footer for the assembly file representing the
 * given context.
 *
 * @note this information includes the directives:
 * ".ident <version-string>"
 * ".section .note.GNU-stack,"",@progbits"
 *
 * @param context
 * @param file
 */
// static void emit_x64_linux_footer([[maybe_unused]] Context *restrict context,
//                                   FILE *file) {
//   StringView exp_version = string_view_from_cstring(EXP_VERSION_STRING);
//   directive_ident(exp_version, file);
//   directive_noexecstack(file);
// }

/**
 * @brief used as the argument for the ".arch" directive in GNU as
 *
 * @note as specified here:
 * https://sourceware.org/binutils/docs/as/i386_002dArch.html
 * this is specific to the cpu that I am currently
 * developing on. in a finished compiler the target cpu
 * would be figured out at runtime, either specified on
 * the command line or retrieved by parsing the output of
 * something like "/proc/cpuinfo". it is important to note
 * that knowing the cpu is step one, as the "cpu_type"
 * argument to the ".arch" directive takes in names which
 * specify the microarchitecture of the cpu. and not it's
 * product name. for instance, my development cpu is
 * an AMD Ryzen 7 5800. this cpu is based on the zen 3
 * microarchitecture, and that is why I chose "znver3"
 * (I assume it stands for zen version 3, the as documentation
 * just give the list of available arguments, they don't
 * list the meaning of each argument.) (on another note
 * specific sub-architectures can also be specified, and
 * these specifiers read more like the availability or
 * unavailability of specific features of a microarchitecture,
 * like ".sse", ".monitor", etc.
 * it is also important to note here that these subfeatures
 * are also listed within "/proc/cpuinfo")
 *
 * addendum: some x64 processors have a specific "CPUID" instruction
 * which reports the capabilitys of the current processor itself.
 * see: AMD64 Architecture Programmer's Manual Volume 3 Appendix E
 * or equivalent section of the Intel x64 Programmers Manual.
 *
 * @todo #TODO storing this string as a static string literal
 * only works when it is acceptable to have the value hardcoded.
 * This data needs to be associated with some dynamic structure
 * which builds up it's content at runtime.
 */
[[maybe_unused]] static StringView cpu_type = {sizeof("znver3") - 1, "znver3"};
