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

#include <stdlib.h>

#include "support/config.h"
#include "support/message.h"
#include "support/panic.h"
#include "support/process.h"

#if defined(EXP_HOST_SYSTEM_LINUX)
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

i32 process(char const *cmd, i32 argc, char const *argv[]) {
    pid_t pid = fork();
    if (pid < 0) {
        PANIC_ERRNO("fork failed");
    } else if (pid == 0) {
        // child process
        execvp(cmd, (char *const *)argv);

        message(MESSAGE_ERROR, NULL, 0, SV("execvp failed"), stderr);
        trace_command(string_view_from_cstring(cmd), argc, argv, stderr);
        abort();
    } else {
        // parent process
        siginfo_t status = {};
        if (waitid(P_PID, (id_t)pid, &status, WEXITED | WSTOPPED) == -1) {
            PANIC_ERRNO("waitid failed");
        }

        switch (status.si_code) {
        case CLD_EXITED: {
            i32 result = status.si_status;
            return result;
        }

        case CLD_KILLED: {
            message(
                MESSAGE_ERROR, NULL, 0, SV("child killed by signal."), stderr);
            trace_command(string_view_from_cstring(cmd), argc, argv, stderr);
            return EXIT_FAILURE;
        }

        case CLD_DUMPED: {
            message(MESSAGE_ERROR, NULL, 0, SV("child dumped core."), stderr);
            trace_command(string_view_from_cstring(cmd), argc, argv, stderr);
            return EXIT_FAILURE;
        }

        case CLD_STOPPED: {
            message(MESSAGE_ERROR, NULL, 0, SV("child stopped."), stderr);
            trace_command(string_view_from_cstring(cmd), argc, argv, stderr);
            return EXIT_FAILURE;
        }

        case CLD_TRAPPED: {
            message(MESSAGE_ERROR, NULL, 0, SV("child trapped."), stderr);
            trace_command(string_view_from_cstring(cmd), argc, argv, stderr);
            return EXIT_FAILURE;
        }

        case CLD_CONTINUED: {
            message(MESSAGE_ERROR, NULL, 0, SV("child continued."), stderr);
            trace_command(string_view_from_cstring(cmd), argc, argv, stderr);
            return EXIT_FAILURE;
        }

        default: {
            message(
                MESSAGE_ERROR, NULL, 0, SV("unknown child status."), stderr);
            trace_command(string_view_from_cstring(cmd), argc, argv, stderr);
            return EXIT_FAILURE;
        }
        }
    }
}

#else
#error "unsupported host OS"
#endif
