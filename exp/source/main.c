/**
 * Copyright (C) 2024 cade
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
#include <stdlib.h>

#include "env/cli_options.h"
#include "env/context.h"
#include "support/message.h"

i32 main(i32 argc, char const *argv[], [[maybe_unused]] char *envv[]) {
    CLIOptions cli_options;
    cli_options_init(&cli_options);
    parse_cli_options(argc, argv, &cli_options);

    Context context;
    context_create(&context, &cli_options.context_options);

    if (context_shall_prolix(&context)) {
        message(MESSAGE_STATUS, NULL, 0, SV("prolix mode enabled"), stdout);
        context_print_compile_actions(&context);
    }

    i32 result =
        context_compile_source(&context, string_to_view(&cli_options.source));

    if ((result != EXIT_FAILURE) &&
        context_shall_create_assembly_artifact(&context)) {
        result |= context_create_assembly_artifact(&context);
    }

    if ((result != EXIT_FAILURE) &&
        context_shall_create_object_artifact(&context)) {
        result |= context_create_object_artifact(&context);
    }

    if ((result != EXIT_FAILURE) &&
        context_shall_create_executable_artifact(&context)) {
        result |= context_create_executable_artifact(&context);
    }

    if (context_shall_cleanup_assembly_artifact(&context) &&
        (result != EXIT_FAILURE)) {
        context_cleanup_assembly_artifact(&context);
    }

    if (context_shall_cleanup_object_artifact(&context) &&
        (result != EXIT_FAILURE)) {
        context_cleanup_object_artifact(&context);
    }

    context_destroy(&context);
    cli_options_destroy(&cli_options);
    return result;
}
