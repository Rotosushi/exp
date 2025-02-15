/**
 * Copyright 2025 Cade Weinberg. All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#include "targets/x86_64/print/gnu_as_directives.h"
#include "utility/assert.h"
#include "utility/unreachable.h"

void directive_file(String *buffer, StringView path) {
    EXP_ASSERT(buffer != nullptr);
    string_append(buffer, SV("\t.file \""));
    string_append(buffer, path);
    string_append(buffer, SV("\"\n"));
}

void directive_arch(String *buffer, StringView cpu_type) {
    EXP_ASSERT(buffer != nullptr);
    string_append(buffer, SV("\t.arch "));
    string_append(buffer, cpu_type);
    string_append(buffer, SV("\n"));
}

void directive_ident(String *buffer, StringView comment) {
    EXP_ASSERT(buffer != nullptr);
    string_append(buffer, SV("\t.ident \""));
    string_append(buffer, comment);
    string_append(buffer, SV("\"\n"));
}

void directive_noexecstack(String *buffer) {
    EXP_ASSERT(buffer != nullptr);
    /**
     * this is the assembly directive which marks the stack as unexecutable.
     * (as far as I can tell, I cannot find documentation which explicitly
     * states that ".note.GNU-stack" marks the stack as noexec, only that
     * "... and the .note.GNU-stack section may have the executable (x)
     *  flag added". which implies to me that the .note... has something
     * to do with marking the stack as exec or noexec.)
     */
    StringView noexecstack = SV("\t.section .note.GNU-stack,\"\",@progbits\n");
    string_append(buffer, noexecstack);
}

void directive_globl(String *buffer, StringView name) {
    EXP_ASSERT(buffer != nullptr);
    string_append(buffer, SV("\t.globl "));
    string_append(buffer, name);
    string_append(buffer, SV("\n"));
}

void directive_data(String *buffer) {
    EXP_ASSERT(buffer != nullptr);
    string_append(buffer, SV("\t.data\n"));
}

void directive_bss(String *buffer) {
    EXP_ASSERT(buffer != nullptr);
    string_append(buffer, SV("\t.bss\n"));
}

void directive_text(String *buffer) {
    EXP_ASSERT(buffer != nullptr);
    string_append(buffer, SV("\t.text\n"));
}

void directive_balign(String *buffer, u64 align) {
    EXP_ASSERT(buffer != nullptr);
    string_append(buffer, SV("\t.balign "));
    string_append_u64(buffer, align);
    string_append(buffer, SV("\n"));
}

void directive_size(String *buffer, StringView name, u64 size) {
    EXP_ASSERT(buffer != nullptr);
    string_append(buffer, SV("\t.size "));
    string_append(buffer, name);
    string_append(buffer, SV(", "));
    string_append_u64(buffer, size);
    string_append(buffer, SV("\n"));
}

void directive_size_label_relative(String *buffer, StringView name) {
    EXP_ASSERT(buffer != nullptr);
    string_append(buffer, SV("\t.size "));
    string_append(buffer, name);
    // the '.' symbol refers to the current address, the '-' is
    // arithmetic subtraction, and the label refers to the address
    // of the label. thus, label relative size computes to the
    // numeric difference between the current address and the address
    // of the label directive
    string_append(buffer, SV(", .-"));
    string_append(buffer, name);
    string_append(buffer, SV("\n"));
}

void directive_type(String *buffer, StringView name, STT_Type kind) {
    EXP_ASSERT(buffer != nullptr);
    string_append(buffer, SV("\t.type "));
    string_append(buffer, name);
    string_append(buffer, SV(", "));

    switch (kind) {
    case STT_OBJECT: string_append(buffer, SV("@object\n")); break;
    case STT_FUNC:   string_append(buffer, SV("@function\n")); break;
    case STT_TLS:    string_append(buffer, SV("@tls_object\n")); break;
    case STT_COMMON: string_append(buffer, SV("@common\n")); break;
    default:         EXP_UNREACHABLE();
    }
}

void directive_quad(String *buffer, i64 value) {
    EXP_ASSERT(buffer != nullptr);
    string_append(buffer, SV("\t.quad "));
    string_append_i64(buffer, value);
    string_append(buffer, SV("\n"));
}

void directive_byte(String *buffer, unsigned char value) {
    EXP_ASSERT(buffer != nullptr);
    string_append(buffer, SV("\t.byte "));
    string_append_u64(buffer, value);
    string_append(buffer, SV("\n"));
}

void directive_zero(String *buffer, u64 bytes) {
    EXP_ASSERT(buffer != nullptr);
    string_append(buffer, SV("\t.zero "));
    string_append_u64(buffer, bytes);
    string_append(buffer, SV("\n"));
}

void directive_string(String *buffer, StringView sv) {
    EXP_ASSERT(buffer != nullptr);
    string_append(buffer, SV("\t.string \""));
    string_append(buffer, sv);
    string_append(buffer, SV("\"\n"));
}

void directive_label(String *buffer, StringView name) {
    EXP_ASSERT(buffer != nullptr);
    string_append(buffer, name);
    string_append(buffer, SV(":\n"));
}
