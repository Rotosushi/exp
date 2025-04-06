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

#include "env/cli_options.h"

/**
 * @brief holds the options relevant for the given
 * context.
 *
 * @todo we can save on memory by generating the filenames
 * for the output files on the fly. The only issue I can forsee
 * is that this reduces our ability to allow the user to customize
 * the name of the output files individually. And since I would like
 * to allow the user to specify multiple build artifacts from one
 * call to the compiler, this may be useful to them.
 * (specifically, I want a flag that produces as assembly listing
 * in addition to the rest of the compilation process. As I think
 * inspecting the assembly is a useful tool, and if the compiler
 * only had to be called once during a build, it would be a lot faster
 * than having to call the compiler once for each build artifact.)
 */
typedef struct ContextOptions {
    Bitset flags;
    String source;
    String assembly;
    String object;
    String output;
} ContextOptions;

ContextOptions context_options_create(CLIOptions *restrict cli_options);
void           context_options_destroy(ContextOptions *restrict options);

inline bool context_options_prolix(ContextOptions const *restrict options) {
    return bitset_check(&options->flags, CLI_PROLIX);
}

inline bool context_options_trace(ContextOptions const *restrict options) {
    return bitset_check(&options->flags, CLI_TRACE);
}

inline bool
context_options_create_ir_artifact(ContextOptions const *restrict options) {
    return bitset_check(&options->flags, CLI_CREATE_IR_ARTIFACT);
}

inline bool context_options_create_assembly_artifact(
    ContextOptions const *restrict options) {
    return bitset_check(&options->flags, CLI_CREATE_ASSEMBLY_ARTIFACT);
}

inline bool
context_options_create_object_artifact(ContextOptions const *restrict options) {
    return bitset_check(&options->flags, CLI_CREATE_OBJECT_ARTIFACT);
}

inline bool context_options_create_executable_artifact(
    ContextOptions const *restrict options) {
    return bitset_check(&options->flags, CLI_CREATE_EXECUTABLE_ARTIFACT);
}

inline bool
context_options_cleanup_ir_artifact(ContextOptions const *restrict options) {
    return bitset_check(&options->flags, CLI_CLEANUP_IR_ARTIFACT);
}

inline bool context_options_cleanup_assembly_artifact(
    ContextOptions const *restrict options) {
    return bitset_check(&options->flags, CLI_CLEANUP_ASSEMBLY_ARTIFACT);
}

inline bool context_options_cleanup_object_artifact(
    ContextOptions const *restrict options) {
    return bitset_check(&options->flags, CLI_CLEANUP_OBJECT_ARTIFACT);
}

#endif // !EXP_ENV_OPTIONS_H
