// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

/**
 * @file core/assemble.h
 */

#ifndef EXP_CORE_ASSEMBLE_H
#define EXP_CORE_ASSEMBLE_H
#include "env/context.h"
#include "utility/result.h"

/**
 * @brief Assembles the given context into an executable.
 *
 * calls the system assembler "as" to assemble the file
 * given by context_assemly_path(context) into an object file
 * located at context_object_path(context).
 *
 * @note we could also embed an assembler into the compiler, something like
 * zydis. I am trying to reduce the number of dependencies in the
 * compiler, and calling "as" allows a user to supply a different
 * assembler. Though we only emit AT&T syntax.
 *
 * @warning expects the context to contain a main subroutine.
 * currently the linker produces an error if there isn't a
 * main subroutine.
 */
ExpResult assemble(Context *context);

#endif // !EXP_CORE_ASSEMBLE_H
