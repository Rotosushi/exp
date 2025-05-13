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

#include "codegen/x86/value/tuple.h"
#include "codegen/GAS/directives.h"
#include "codegen/x86/env/context.h"

void print_x86_tuple_layout(String *restrict buffer,
                            x86_TupleLayout const *restrict x86_tuple_layout);

void x86_codegen_tuple_symbol(String *restrict buffer,
                              Symbol const *restrict symbol,
                              Context *restrict context) {
    x86_Tuple       *tuple  = x86_context_lookup_tuple(context, symbol->type);
    x86_TupleLayout *layout = &tuple->layout;
    u64              size   = x86_tuple_layout_size_of(layout);
    u64              align  = x86_tuple_layout_align_of(layout);
    gas_directive_data(buffer);
    gas_directive_globl(symbol->name, buffer);
    gas_directive_balign(align, buffer);
    gas_directive_type(symbol->name, STT_OBJECT, buffer);
    gas_directive_size(symbol->name, size, buffer);
    gas_directive_label(symbol->name, buffer);
    print_x86_tuple_layout(buffer, layout);
}
