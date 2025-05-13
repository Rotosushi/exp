/**
 * Copyright (C) 2025 Cade Weinberg
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

#include "codegen/x86/value/scalar.h"
#include "codegen/GAS/directives.h"

void x86_codegen_uninitialized_symbol(String *restrict buffer,
                                      Symbol const *restrict symbol,
                                      Context *restrict context) {
    u64 size  = context_size_of(context, symbol->type);
    u64 align = context_align_of(context, symbol->type);
    gas_directive_bss(buffer);
    gas_directive_globl(symbol->name, buffer);
    gas_directive_balign(align, buffer);
    gas_directive_type(symbol->name, STT_OBJECT, buffer);
    gas_directive_size(symbol->name, size, buffer);
    gas_directive_label(symbol->name, buffer);
    gas_directive_zero(size, buffer);
}

void x86_codegen_nil_symbol(String *restrict buffer,
                            Symbol const *restrict symbol) {
    gas_directive_data(buffer);
    gas_directive_globl(symbol->name, buffer);
    gas_directive_type(symbol->name, STT_OBJECT, buffer);
    gas_directive_size(symbol->name, 1, buffer);
    gas_directive_label(symbol->name, buffer);
    gas_directive_u8(0, buffer);
}

void x86_codegen_bool_symbol(String *restrict buffer,
                             Symbol const *restrict symbol) {
    gas_directive_data(buffer);
    gas_directive_globl(symbol->name, buffer);
    gas_directive_type(symbol->name, STT_OBJECT, buffer);
    gas_directive_size(symbol->name, 1, buffer);
    gas_directive_label(symbol->name, buffer);
    gas_directive_u8(symbol->value->bool_, buffer);
}

void x86_codegen_u8_symbol(String *restrict buffer,
                           Symbol const *restrict symbol) {
    gas_directive_data(buffer);
    gas_directive_globl(symbol->name, buffer);
    gas_directive_type(symbol->name, STT_OBJECT, buffer);
    gas_directive_size(symbol->name, 1, buffer);
    gas_directive_label(symbol->name, buffer);
    gas_directive_u8(symbol->value->u8_, buffer);
}

void x86_codegen_u16_symbol(String *restrict buffer,
                            Symbol const *restrict symbol) {
    gas_directive_data(buffer);
    gas_directive_globl(symbol->name, buffer);
    gas_directive_balign(2, buffer);
    gas_directive_type(symbol->name, STT_OBJECT, buffer);
    gas_directive_size(symbol->name, 2, buffer);
    gas_directive_label(symbol->name, buffer);
    gas_directive_u16(symbol->value->u16_, buffer);
}

void x86_codegen_u32_symbol(String *restrict buffer,
                            Symbol const *restrict symbol) {
    gas_directive_data(buffer);
    gas_directive_globl(symbol->name, buffer);
    gas_directive_balign(4, buffer);
    gas_directive_type(symbol->name, STT_OBJECT, buffer);
    gas_directive_size(symbol->name, 4, buffer);
    gas_directive_label(symbol->name, buffer);
    gas_directive_u32(symbol->value->u32_, buffer);
}

void x86_codegen_u64_symbol(String *restrict buffer,
                            Symbol const *restrict symbol) {
    gas_directive_data(buffer);
    gas_directive_globl(symbol->name, buffer);
    gas_directive_balign(8, buffer);
    gas_directive_type(symbol->name, STT_OBJECT, buffer);
    gas_directive_size(symbol->name, 8, buffer);
    gas_directive_label(symbol->name, buffer);
    gas_directive_u64(symbol->value->u64_, buffer);
}

void x86_codegen_i8_symbol(String *restrict buffer,
                           Symbol const *restrict symbol) {
    gas_directive_data(buffer);
    gas_directive_globl(symbol->name, buffer);
    gas_directive_balign(1, buffer);
    gas_directive_type(symbol->name, STT_OBJECT, buffer);
    gas_directive_size(symbol->name, 1, buffer);
    gas_directive_label(symbol->name, buffer);
    gas_directive_i8(symbol->value->i8_, buffer);
}

void x86_codegen_i16_symbol(String *restrict buffer,
                            Symbol const *restrict symbol) {
    gas_directive_data(buffer);
    gas_directive_globl(symbol->name, buffer);
    gas_directive_balign(2, buffer);
    gas_directive_type(symbol->name, STT_OBJECT, buffer);
    gas_directive_size(symbol->name, 2, buffer);
    gas_directive_label(symbol->name, buffer);
    gas_directive_i16(symbol->value->i16_, buffer);
}

void x86_codegen_i32_symbol(String *restrict buffer,
                            Symbol const *restrict symbol) {
    gas_directive_data(buffer);
    gas_directive_globl(symbol->name, buffer);
    gas_directive_balign(4, buffer);
    gas_directive_type(symbol->name, STT_OBJECT, buffer);
    gas_directive_size(symbol->name, 4, buffer);
    gas_directive_label(symbol->name, buffer);
    gas_directive_i32(symbol->value->i32_, buffer);
}

void x86_codegen_i64_symbol(String *restrict buffer,
                            Symbol const *restrict symbol) {
    gas_directive_data(buffer);
    gas_directive_globl(symbol->name, buffer);
    gas_directive_balign(8, buffer);
    gas_directive_type(symbol->name, STT_OBJECT, buffer);
    gas_directive_size(symbol->name, 8, buffer);
    gas_directive_label(symbol->name, buffer);
    gas_directive_i64(symbol->value->i64_, buffer);
}
