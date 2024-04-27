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
#include <stdlib.h>

#include "backend/as_directives.h"
#include "backend/compute_allocations.h"
#include "backend/compute_lifetimes.h"
#include "backend/emit_assembly.h"
#include "utility/config.h"
#include "utility/numeric_conversions.h"

/*
  #NOTE: we are using the defaults for the system we are on,
  thus:
  x64 Linux
    - the major thing to note is the syscall differences.
  x64 assembly
  GNU as
  AT&T syntax
      - the major differences are
         register names are prefixed with %
         immediate values are prefixed with $
         the destination is on the right,
         the source is on the left.

*/

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
static void emit_header(Context *restrict context, FILE *file) {
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
static void emit_footer([[maybe_unused]] Context *restrict context,
                        FILE *file) {
  StringView exp_version = string_view_from_cstring(EXP_VERSION_STRING);
  directive_ident(exp_version, file);
  directive_noexecstack(file);
}

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

static void emit_function_header(StringView name, FILE *file) {
  directive_text(file);
  directive_globl(name, file);
  directive_type(name, STT_FUNC, file);
  directive_label(name, file);
}

static void emit_function_footer(StringView name, FILE *file) {
  directive_size_label_relative(name, file);
}

static void print_register(Register r, FILE *file) {
  file_write("%", file);
  print_string_view(register_to_sv(r), file);
}

static void print_immediate(i64 i, FILE *file) {
  file_write("$", file);
  print_i64(i, RADIX_DECIMAL, file);
}

/*
  add r0, r1
works as-if
  r0 += r1
whereas the bytecode
  add A, B, C
works as-if
  A = B + C

 sub r0, r1
works as-if
 r0 -= r1
whereas the bytecode
 sub A, B, C
works as if
 A = B - C

 mul r0
works as-if
 RAX = RAX * r0
whereas the bytecode
 mul A, B, C
works as-if
 A = B * C

 div r0
works as-if
 RAX = RAX / r0
 RDX = RAX % r0
whereas the bytecode
 div A, B, C
works as-if
 A = B / C
and
 mod A, B, C
works as-if
 A = B % C

okay, so all instructions have preconditions which
affect what registers the locals can even be allocated into.

the destination register and one of the source registers
are the same register for all arithmetic binops. on top of that
multiplication, division, and modulus require one of the
operands to be in %rax.

the negation instruction has the same source and destination
register.

the return instruction implicitly expects %rax to hold the result.
(by convention to be fair)

the call instruction implicitly expects arguments to pe placed into
a specific sequence of regsters.

So, how do we handle that?

I think we need to choose register allocations based on more
constraints. Like, it isn't enough that the simply is any
free register, that register needs to be free and specific to
how it is going to be used later than it's declaration.
*/

static Allocation find_local_allocation(Allocations *restrict al, u16 local) {
  for (u16 i = 0; i < al->size; ++i) {
    Allocation *a = al->buffer + i;
    if (a->local == local) {
      return *a;
    }
  }

  unreachable();
}

static i64 get_constant(Constants *restrict constants, u16 index) {
  Value *value = constants_at(constants, index);
  assert(value != NULL);

  switch (value->kind) {
  case VALUEKIND_UNINITIALIZED:
  case VALUEKIND_NIL:
    return (i64)value->nil;

  case VALUEKIND_BOOLEAN:
    return (i64)value->boolean;

  case VALUEKIND_I64:
    return value->integer;

  default:
    unreachable();
  }
}

// OPC_MOVE
//  AB  -- L[A] = B
//  AB  -- L[A] = C[B]
//  AB  -- L[A] = L[B]
// x64 mov
// mov <reg>/<mem>/<imm>, <reg>/<mem>
static void emit_move(Instruction I, Allocations *restrict al,
                      Constants *restrict constants, FILE *restrict file) {
  file_write("\tmov ", file);

  switch (INST_B_FORMAT(I)) {
  case FORMAT_LOCAL: {
    u16 B        = INST_B(I);
    Allocation b = find_local_allocation(al, B);
    print_register(b.reg, file);
    break;
  }

  case FORMAT_IMMEDIATE: {
    print_immediate(INST_B(I), file);
    break;
  }

  case FORMAT_CONSTANT: {
    i64 imm = get_constant(constants, INST_B(I));
    print_immediate(imm, file);
    break;
  }

  default:
    unreachable();
  }

  file_write(", ", file);

  u16 A           = INST_A(I);
  Allocation a    = find_local_allocation(al, A);
  StringView regA = register_to_sv(a.reg);

  print_string_view(regA, file);

  file_write("\n", file);
}

// OPC_NEG
// AB  -- L[A] = -(B)
// AB  -- L[A] = -(C[B])
// AB  -- L[A] = -(L[B])
// x64 neg
// neg <reg>/<mem>
static void emit_neg(Instruction I, Allocations *restrict al,
                     Constants *restrict c, FILE *restrict file) {
  file_write("\tneg ", file);
  // if the argument is an immediate, then we have to load it into
  // the target register before emitting the neg instruction
  //
  //

  file_write("\n", file);
}

static void emit_instruction(Instruction I, Allocations *restrict al,
                             Constants *restrict c, FILE *restrict f) {
  switch (INST_OP(I)) {

  case OPC_MOVE: {
    emit_move(I, al, c, f);
    break;
  }

  case OPC_NEG: {
    emit_neg(I, al, c, f);
    break;
  }

  default:
    unreachable();
  }
}

static void emit_function(StringView name, FunctionBody *restrict body,
                          Allocations *restrict al,
                          Constants *restrict constants, FILE *restrict file) {
  Bytecode *bc = &body->bc;

  emit_function_header(name, file);

  for (u16 i = 0; i < bc->length; ++i) {
    emit_instruction(bc->buffer[i], al, constants, file);
  }

  emit_function_footer(name, file);
}

/*
  1 - compute lifetime intervals
  2 - allocate SSA locals to registers or the stack
  3 - select instructions
  4 - write file
*/
static i32 emit_global_symbol(SymbolTableElement *restrict symbol,
                              Constants *restrict constants,
                              FILE *restrict file) {
  FunctionBody *body = &symbol->function_body;

  Lifetimes lifetimes     = compute_lifetimes(body);
  Allocations allocations = compute_allocations(&lifetimes);

  emit_function(symbol->name, body, &allocations, constants, file);

  lifetimes_destroy(&lifetimes);
  allocations_destroy(&allocations);
  return EXIT_SUCCESS;
}

i32 emit_assembly(Context *restrict context) {
  assert(context != NULL);
  i32 result = EXIT_SUCCESS;

  Constants *constants = &context->constants;
  FILE *file           = context_open_output(context);

  emit_header(context, file);

  SymbolTableIterator iter = context_global_symbol_iterator(context);
  while (!symbol_table_iterator_done(&iter)) {
    SymbolTableElement *symbol = iter.element;

    result |= emit_global_symbol(symbol, constants, file);

    symbol_table_iterator_next(&iter);
  }

  emit_footer(context, file);

  file_close(file);

  return result;
}