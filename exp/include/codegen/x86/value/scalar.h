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

#ifndef EXP_CODEGEN_X86_VALUE_SCALAR_H
#define EXP_CODEGEN_X86_VALUE_SCALAR_H

#include "env/context.h"

void x86_codegen_uninitialized_symbol(String *restrict buffer,
                                      Symbol const *restrict symbol,
                                      Context *restrict context);

void x86_codegen_nil_symbol(String *restrict buffer,
                            Symbol const *restrict symbol);

void x86_codegen_bool_symbol(String *restrict buffer,
                             Symbol const *restrict symbol);

void x86_codegen_u8_symbol(String *restrict buffer,
                           Symbol const *restrict symbol);

void x86_codegen_u16_symbol(String *restrict buffer,
                            Symbol const *restrict symbol);

void x86_codegen_u32_symbol(String *restrict buffer,
                            Symbol const *restrict symbol);

void x86_codegen_u64_symbol(String *restrict buffer,
                            Symbol const *restrict symbol);

void x86_codegen_i8_symbol(String *restrict buffer,
                           Symbol const *restrict symbol);

void x86_codegen_i16_symbol(String *restrict buffer,
                            Symbol const *restrict symbol);

void x86_codegen_i32_symbol(String *restrict buffer,
                            Symbol const *restrict symbol);

void x86_codegen_i64_symbol(String *restrict buffer,
                            Symbol const *restrict symbol);

#endif // !EXP_CODEGEN_X86_VALUE_SCALAR_H
