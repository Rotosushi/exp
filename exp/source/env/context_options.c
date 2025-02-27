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

void context_options_initialize(ContextOptions *context_options,
                                CLIOptions *cli_options) {
    context_options->flags = cli_options->flags;
    string_initialize(&context_options->source);
    string_initialize(&context_options->assembly);
    string_initialize(&context_options->object);
    string_initialize(&context_options->output);

    if (!string_empty(&cli_options->source)) {
        string_assign_string(&context_options->source, &cli_options->source);

        string_assign_string(&context_options->assembly, &cli_options->source);
        string_replace_extension(&context_options->assembly, SV(".s"));

        string_assign_string(&context_options->object, &cli_options->source);
        string_replace_extension(&context_options->object, SV(".o"));
    }

    if (!string_empty(&cli_options->output)) {
        string_assign_string(&context_options->output, &cli_options->output);
    }
}

void context_options_destroy(ContextOptions *options) {
    string_destroy(&options->source);
    string_destroy(&options->assembly);
    string_destroy(&options->object);
    string_destroy(&options->output);
}

bool context_options_do_assemble(ContextOptions *options) {
    return bitset_check_bit(&options->flags, CLI_DO_ASSEMBLE);
}

bool context_options_do_link(ContextOptions *options) {
    return bitset_check_bit(&options->flags, CLI_DO_LINK);
}

bool context_options_do_cleanup(ContextOptions *options) {
    return bitset_check_bit(&options->flags, CLI_DO_CLEANUP);
}
