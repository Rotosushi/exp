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
 * along with exp.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <assert.h>

#include "backend/as_directives.h"
#include "backend/emit_x64_linux_assembly.h"
#include "imr/opcode.h"
#include "intrinsics/align_of.h"
#include "intrinsics/size_of.h"
#include "intrinsics/type_of.h"
#include "utility/config.h"
#include "utility/io.h"
#include "utility/numeric_conversions.h"
#include "utility/panic.h"

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
static StringView cpu_type = {sizeof("znver3") - 1, "znver3"};

// typedef enum RegisterName {
//   REG_RAX,
//   REG_RDI,
// } RegisterName;

// static void register_name(RegisterName name, FILE *file) {
//   switch (name) {
//   case REG_RAX:
//     file_write("%rax", file);
//     break;

//   case REG_RDI:
//     file_write("%rdi", file);
//     break;

//   default:
//     PANIC("unknown RegisterName");
//   }
// }

// static void immediate(i64 value, FILE *file) {
//   file_write("$", file);
//   print_intmax(value, RADIX_DECIMAL, file);
// }

/**
 * @brief emit a return (near) instruction.
 *
 * @note Returns from a procedure previously entered by a CALL near
instruction. This form of the RET instruction returns to a calling procedure
within the current code segment. This instruction pops the rIP from the stack,
with the size of the pop determined by the operand size. The new rIP is then
zero-extended to 64 bits. The RET instruction can accept an immediate value
operand that it adds to the rSP after it pops the target rIP. This action skips
over any parameters previously passed back to the subroutine that are no longer
needed. In 64-bit mode, the operand size defaults to 64 bits (eight bytes)
without the need for a REX prefix. No prefix is available to encode a 32-bit
operand size in 64-bit mode
 *
 * @param file
 */
// static void instruction_ret(FILE *file) { file_write("  ret\n", file); }

/**
 * @brief emit a call (near) instruction
 *
 * @note Pushes the offset of the next instruction onto the stack and branches
to the target address, which contains the first instruction of the called
procedure. The target operand can specify a register, a memory location, or a
label. A procedure accessed by a near CALL is located in the same code segment
as the CALL instruction.
 *
 * @param name
 * @param file
 */
// static void instruction_call(StringView name, FILE *file) {
//   file_write("  call ", file);
//   file_write(name.ptr, file);
//   file_write("\n", file);
// }

// static void instruction_mov_immediate(i64 value, RegisterName reg, FILE
// *file) {
//   file_write("  mov ", file);
//   immediate(value, file);
//   file_write(" ", file);
//   register_name(reg, file);
//   file_write("\n", file);
// }

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
static void emit_x64_linux_header(Context *restrict context, FILE *file) {
  StringView path = context_source_path(context);
  directive_file(path, file);
  directive_arch(cpu_type, file);
  file_write("\n", file);
}

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
static void emit_x64_linux_footer([[maybe_unused]] Context *restrict context,
                                  FILE *file) {
  StringView exp_version = string_view_from_cstring(EXP_VERSION_STRING);
  directive_ident(exp_version, file);
  directive_noexecstack(file);
}

/**
 * @brief emit the assembly for the given global constant symbol
 * into the assembly file representing the given context.
 *
 * @param context
 * @param element
 * @param file
 */
static void emit_x64_linux_global_const(Context *restrict context,
                                        SymbolTableElement *global,
                                        FILE *file) {
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
  StringView name = global->name;
  Value *value    = global->value;
  Type *type      = type_of(value, context);
  switch (type->kind) {
  case TYPEKIND_NIL:
    directive_globl(name, file);
    directive_bss(file);
    directive_type(name, STT_OBJECT, file);
    directive_size(name, size_of(type), file);

    directive_label(name, file);
    directive_zero(1UL, file);
    break;

  case TYPEKIND_BOOLEAN:
    directive_globl(name, file);
    if (value->kind == VALUEKIND_BOOLEAN) {
      directive_data(file);
    } else {
      directive_bss(file);
    }
    directive_type(name, STT_OBJECT, file);
    directive_size(name, size_of(type), file);

    directive_label(name, file);
    if (value->kind == VALUEKIND_BOOLEAN) {
      directive_byte((unsigned char)value->boolean, file);
    } else {
      directive_zero(1UL, file);
    }
    break;

  case TYPEKIND_I64:
    directive_globl(name, file);
    if (value->kind == VALUEKIND_I64) {
      directive_data(file);
    } else {
      directive_bss(file);
    }
    directive_balign(align_of(type), file);
    directive_type(name, STT_OBJECT, file);
    directive_size(name, size_of(type), file);

    directive_label(name, file);
    if (value->kind == VALUEKIND_I64) {
      directive_quad(value->integer, file);
    } else {
      directive_zero(size_of(type), file);
    }
    break;

  default:
    PANIC("bad VALUEKIND");
  }

  // give an extra line between globals in the assembly file
  file_write("\n", file);
}

void emit_x64_linux_assembly(Context *restrict context) {
  StringView path = context_output_path(context);
  FILE *file      = file_open(path.ptr, "w");

  SymbolTableIterator iter =
      symbol_table_iterator_create(&(context->global_symbols));

  emit_x64_linux_header(context, file);

  while (!symbol_table_iterator_done(&iter)) {
    emit_x64_linux_global_const(context, iter.element, file);

    symbol_table_iterator_next(&iter);
  }

  emit_x64_linux_footer(context, file);

  if (fclose(file) == EOF) {
    PANIC_ERRNO("fclose failed");
  }
}