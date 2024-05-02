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
 * along with exp.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <stdlib.h>

#include "core/assemble.h"
#include "utility/process.h"

i32 assemble(Context *restrict context) {
  StringView asm_path = context_assembly_path(context);
  StringView obj_path = context_object_path(context);

  char const *args[] = {
      "as", asm_path.ptr, "-o", obj_path.ptr, NULL,
  };

  return process("as", args);
}