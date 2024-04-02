// Copyright (C) 2024 Cade Weinberg
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
// along with exp.  If not, see <http://www.gnu.org/licenses/>.
#ifndef EXP_ENV_OPTIONS_H
#define EXP_ENV_OPTIONS_H

#include "utility/cli_options.h"
#include "utility/path.h"

typedef struct ContextOptions {
  Path source;
  Path output;
} ContextOptions;

ContextOptions context_options_create();
ContextOptions context_options_clone(ContextOptions *restrict options);
void context_options_destroy(ContextOptions *restrict options);

#endif // !EXP_ENV_OPTIONS_H