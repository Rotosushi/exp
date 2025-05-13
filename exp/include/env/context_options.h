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
#ifndef EXP_ENV_CONTEXT_OPTIONS_H
#define EXP_ENV_CONTEXT_OPTIONS_H

#include <stdbool.h>

#include "codegen/target.h"

/**
 * @brief holds the options available that affect the compilation
 * of a given context.
 */
typedef struct ContextOptions {
    bool    prolix                     : 1;
    bool    create_assembly_artifact   : 1;
    bool    create_object_artifact     : 1;
    bool    create_executable_artifact : 1;
    bool    cleanup_assembly_artifact  : 1;
    bool    cleanup_object_artifact    : 1;
    Target *target;
    void   *target_context;
} ContextOptions;

#endif // !EXP_ENV_CONTEXT_OPTIONS_H
