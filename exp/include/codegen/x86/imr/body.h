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
#ifndef EXP_CODEGEN_X86_IMR_BODY_H
#define EXP_CODEGEN_X86_IMR_BODY_H

#include "codegen/x86/imr/block.h"

typedef struct x86_Body {
    u32        length;
    u32        capacity;
    x86_Block *buffer;
} x86_Body;

void x86_body_create(x86_Body *restrict body);
void x86_body_destroy(x86_Body *restrict body);

u32 x86_body_length(x86_Body const *restrict body);

void       x86_body_insert(x86_Body *restrict body, u32 index);
u32        x86_body_append(x86_Body *restrict body);
void       x86_body_prepend(x86_Body *restrict body);
x86_Block *x86_body_at(x86_Body *restrict body, u32 index);

void print_x86_body(String *restrict string, x86_Body const *restrict body);

#endif // !EXP_CODEGEN_X86_IMR_BODY_H
