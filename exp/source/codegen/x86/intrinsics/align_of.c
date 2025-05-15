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

#include "codegen/x86/intrinsics/align_of.h"
#include "codegen/x86/env/context.h"
#include "support/assert.h"

u64 x86_align_of(Context *restrict context, Type const *restrict type) {
    exp_assert(context != NULL);
    exp_assert(type != NULL);
    x86_Layout const *layout = x86_context_layout_of_type(context, type);
    return x86_layout_align_of(layout);
}
