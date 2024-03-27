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
#include <stdlib.h>

#include "backend/emit_x64_linux_assembly.h"
#include "core/compile.h"
#include "core/interpret.h"
#include "frontend/parser.h"

int compile(Context *restrict context) {

  String buffer = context_buffer_source(context);
  if (parse(buffer.buffer, context) == EXIT_FAILURE) {
    return EXIT_FAILURE;
  }

  if (interpret(context) == EXIT_FAILURE) {
    return EXIT_FAILURE;
  }

  emit_x64_linux_assembly(context);
  return EXIT_SUCCESS;
}