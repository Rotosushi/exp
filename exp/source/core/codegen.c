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

/**
 * @file core/codegen.c
 */

#include "core/codegen.h"
#include "targets/ir/emit.h"
#include "utility/assert.h"

void codegen(Context *context) {
    EXP_ASSERT(context != nullptr);

    if (context_emit_ir_assembly(context)) { emit_ir_assembly(context); }
}
