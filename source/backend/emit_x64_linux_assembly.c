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

#include "backend/emit_x64_linux_assembly.h"
#include "intrinsics/alignment.h"
#include "intrinsics/size.h"
#include "utility/config.h"
#include "utility/io.h"
#include "utility/numbers_to_string.h"
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
 * @todo #TODO storing this string as a static string literal
 * only works when it is acceptable to have the value hardcoded.
 * This data needs to be associated with some dynamic structure
 * which builds up it's content at runtime.
 */
static StringView cpu_type = {"znver3", sizeof("znver3")};

static void directive_file(StringView path, FILE *file) {
  file_write("  .file \"", file);
  file_write(path.ptr, file);
  file_write("\"\n", file);
}

static void directive_arch(StringView cpu_type, FILE *file) {
  file_write("  .arch ", file);
  file_write(cpu_type.ptr, file);
  file_write("\n", file);
}

static void directive_ident(StringView comment, FILE *file) {
  file_write("  .ident \"", file);
  file_write(comment.ptr, file);
  file_write("\"\n", file);
}

static void directive_noexecstack(FILE *file) {
  /**
   * @brief the assembly directive which marks the stack as unexecutable.
   * (as far as I can tell)
   */
  static char const noexecstack[] =
      "  .section .note.GNU-stack,\"\",@progbits\n";
  file_write(noexecstack, file);
}

static void directive_globl(StringView name, FILE *file) {
  file_write("  .globl ", file);
  file_write(name.ptr, file);
  file_write("\n", file);
}

static void directive_data(FILE *file) { file_write("  .data\n", file); }

static void directive_bss(FILE *file) { file_write("  .bss\n", file); }

[[maybe_unused]] static void directive_text(FILE *file) {
  file_write("  .text\n", file);
}

static void directive_align(Type *type, FILE *file) {
  size_t align = align_of(type);
  size_t len = uintmax_safe_strlen(align, RADIX_DECIMAL);
  char str[len + 1];
  if (uintmax_to_str(align, str, RADIX_DECIMAL) == NULL) {
    panic("conversion failed");
  }
  str[len] = '\0';

  file_write("  .align ", file);
  file_write(str, file);
  file_write("\n", file);
}

/**
 * @brief emits the .size <name>, <expression> directive
 *
 * @param name the name of the symbol to associate with the size
 * @param size the size to place in <expression>
 * @param file
 */
static void directive_size(StringView name, size_t size, FILE *file) {
  size_t len = uintmax_safe_strlen(size, RADIX_DECIMAL);
  char str[len + 1];
  if (uintmax_to_str(size, str, RADIX_DECIMAL) == NULL) {
    panic("conversion failed");
  }
  str[len] = '\0';

  file_write("  .size ", file);
  file_write(name.ptr, file);
  file_write(", ", file);
  file_write(str, file);
  file_write("\n", file);
}

static void directive_type(StringView name, Type *type, FILE *file) {
  file_write("  .type ", file);
  file_write(name.ptr, file);
  file_write(", ", file);

  switch (type->kind) {
  // essentially everything is an @object unless it's an @function.
  // with the edgecases of thread-locals @tls_object,
  // common symbols @common (linker merges these symbols across translation
  // units), and indirect-functions @gnu_indirect_function.
  // (the actual function to be called can be resolved at runtime;
  // it's complex. https://maskray.me/blog/2021-01-18-gnu-indirect-function
  // and mainly used so programmers can override malloc/free in the
  // c stdlib. or so I've read.)
  case TYPEKIND_NIL:
  case TYPEKIND_BOOLEAN:
  case TYPEKIND_INTEGER:
  case TYPEKIND_STRING_LITERAL:
    file_write("@object", file);
    break;

  default:
    panic("bad TYPEKIND");
  }

  file_write("\n", file);
}

static void directive_quad(long value, FILE *file) {
  size_t len = intmax_safe_strlen(value, RADIX_DECIMAL);
  char str[len + 1];
  if (intmax_to_str(value, str, RADIX_DECIMAL) == NULL) {
    panic("conversion failed");
  }
  str[len] = '\0';

  file_write("  .quad ", file);
  file_write(str, file);
  file_write("\n", file);
}

static void directive_byte(unsigned char value, FILE *file) {
  size_t len = uintmax_safe_strlen(value, RADIX_DECIMAL);
  char str[len + 1];
  if (uintmax_to_str(value, str, RADIX_DECIMAL) == NULL) {
    panic("conversion failed");
  }
  str[len] = '\0';

  file_write("  .byte ", file);
  file_write(str, file);
  file_write("\n", file);
}

static void directive_zero(size_t bytes, FILE *file) {
  size_t len = uintmax_safe_strlen(bytes, RADIX_DECIMAL);
  char str[len + 1];
  if (uintmax_to_str(bytes, str, RADIX_DECIMAL) == NULL) {
    panic("conversion failed");
  }
  str[len] = '\0';

  file_write("  .zero ", file);
  file_write(str, file);
  file_write("\n", file);
}

static void directive_string(StringView sv, FILE *file) {
  file_write("  .string \"", file);
  file_write(sv.ptr, file);
  file_write("\"\n", file);
}

static void directive_label(StringView name, FILE *file) {
  file_write(name.ptr, file);
  file_write(":\n", file);
}

/**
 * @brief emit the header of the assembly file representing the
 * given context.
 *
 * @note this information includes the directives:
 *  ".file", ".arch", anything else??
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
  StringView exp_version = {EXP_VERSION_STRING, sizeof(EXP_VERSION_STRING)};
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
static void
emit_x64_linux_global_const([[maybe_unused]] Context *restrict context,
                            SymbolTableElement *global, FILE *file) {
  /*
a global object declaration in assembly looks like:
  .globl <name>
  .bss | .data
  .align <alignment>
  .type <name>, @object
  .size <name>, <sizeof>
<name>:
  .byte <init> | .zero <sizeof> | .quad <init> | .int <init> | ...

-- all global symbols can go into the .data section. unless they
are uninitialized, then they are default initialized to zero, and can go into
the .bss section. This holds for constants and variables, it is up to the
compiler to prevent writes to constants.

*/
  StringView name = global->name;
  Type *type = global->type;
  Value *value = &(global->value);
  switch (type->kind) {
  case TYPEKIND_NIL:
    directive_globl(name, file);
    directive_bss(file);
    directive_type(name, type, file);
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
    directive_type(name, type, file);
    directive_size(name, size_of(type), file);

    directive_label(name, file);
    if (value->kind == VALUEKIND_BOOLEAN) {
      directive_byte((unsigned char)value->boolean, file);
    } else {
      directive_zero(1UL, file);
    }
    break;

  case TYPEKIND_INTEGER:
    directive_globl(name, file);
    if (value->kind == VALUEKIND_INTEGER) {
      directive_data(file);
    } else {
      directive_bss(file);
    }
    directive_align(type, file);
    directive_type(name, type, file);
    directive_size(name, size_of(type), file);

    directive_label(name, file);
    if (value->kind == VALUEKIND_INTEGER) {
      directive_quad(value->integer, file);
    } else {
      directive_zero(size_of(type), file);
    }
    break;

  case TYPEKIND_STRING_LITERAL:
    directive_globl(name, file);
    if (value->kind == VALUEKIND_STRING_LITERAL) {
      directive_data(file);
    } else {
      directive_bss(file);
    }
    directive_align(type, file);
    directive_type(name, type, file);
    if (value->kind == VALUEKIND_STRING_LITERAL) {
      directive_size(name, value->string_literal.length, file);
    } else {
      directive_size(name, 0UL, file);
    }

    directive_label(name, file);
    if (value->kind == VALUEKIND_STRING_LITERAL) {
      directive_string(value->string_literal, file);
    } else {
      directive_string(string_view_from_cstring(""), file);
    }
    break;

  default:
    panic("bad VALUEKIND");
  }

  // give an extra line between globals in the assembly file
  file_write("\n", file);
}

void emit_x64_linux_assembly(Context *restrict context) {
  StringView path = context_output_path(context);
  FILE *file = fopen(path.ptr, "w");
  if (file == NULL) {
    panic_errno("fopen failed");
  }

  SymbolTableIterator iter =
      symbol_table_iterator_create(&(context->global_symbols));

  emit_x64_linux_header(context, file);

  while (!symbol_table_iterator_done(&iter)) {
    emit_x64_linux_global_const(context, iter.element, file);

    symbol_table_iterator_next(&iter);
  }

  emit_x64_linux_footer(context, file);

  if (fclose(file) == EOF) {
    panic_errno("fclose failed");
  }
}