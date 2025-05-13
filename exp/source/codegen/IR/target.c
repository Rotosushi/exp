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

#include "codegen/IR/target.h"
#include "codegen/IR/codegen.h"

void *ir_context_allocate() { return NULL; }
void  ir_context_deallocate([[maybe_unused]] void *restrict context) { return; }

Target ir_target_info = {.tag                  = SV("ir"),
                         .triple               = SV("exp-ir"),
                         .assembly_extension   = SV("eir"),
                         .object_extension     = SV(""),
                         .library_extension    = SV(""),
                         .executable_extension = SV(""),
                         .header               = ir_header,
                         .codegen              = ir_codegen,
                         .footer               = ir_footer,
                         .context_allocate     = ir_context_allocate,
                         .context_deallocate   = ir_context_deallocate};

Target *ir_target = &ir_target_info;
