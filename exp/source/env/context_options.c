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
#include "env/context_options.h"

ContextOptions context_options_create(CLIOptions *restrict cli_options) {
    ContextOptions options = {.flags = cli_options->flags};

    if (!string_empty(&cli_options->source)) {
        string_assign_string(&options.source, &cli_options->source);

        string_assign_string(&options.ir, &cli_options->source);
        string_replace_extension(&options.ir, SV(".expir"));

        string_assign_string(&options.assembly, &cli_options->source);
        string_replace_extension(&options.assembly, SV(".s"));

        string_assign_string(&options.object, &cli_options->source);
        string_replace_extension(&options.object, SV(".o"));
    }

    if (!string_empty(&cli_options->output)) {
        string_assign_string(&options.executable, &cli_options->output);
    }

    return options;
}

void context_options_destroy(ContextOptions *restrict options) {
    string_destroy(&options->source);
    string_destroy(&options->ir);
    string_destroy(&options->assembly);
    string_destroy(&options->object);
    string_destroy(&options->executable);
}

extern bool context_options_prolix(ContextOptions const *restrict options);
extern bool context_options_trace(ContextOptions const *restrict options);
extern bool
context_options_create_ir_artifact(ContextOptions const *restrict options);
extern bool context_options_create_assembly_artifact(
    ContextOptions const *restrict options);
extern bool
context_options_create_object_artifact(ContextOptions const *restrict options);
extern bool context_options_create_executable_artifact(
    ContextOptions const *restrict options);
extern bool
context_options_cleanup_ir_artifact(ContextOptions const *restrict options);
extern bool context_options_cleanup_assembly_artifact(
    ContextOptions const *restrict options);
extern bool
context_options_cleanup_object_artifact(ContextOptions const *restrict options);

#undef CHK_BIT
