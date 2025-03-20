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
#include "support/log.h"
#include "support/panic.h"
#include "support/process.h"

static void log_command(char const *cmd, i32 argc, char const *argv[]) {
    exp_log(LOG_STATUS, NULL, 0, SV("command: "), stderr);
    exp_log(LOG_STATUS, NULL, 0, string_view_from_cstring(cmd), stderr);
    exp_log(LOG_STATUS, NULL, 0, SV("args: "), stderr);
    for (i32 i = 0; argv[i] != NULL && (i < argc); ++i) {
        exp_log(LOG_STATUS, NULL, 0, string_view_from_cstring(argv[i]), stderr);
    }
}

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

        PANIC_ERRNO("execvp failed");
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
            exp_log(LOG_ERROR, NULL, 0, SV("child killed by signal."), stderr);
            log_command(cmd, argc, argv);
            return EXIT_FAILURE;
        }

        case CLD_DUMPED: {
            exp_log(LOG_ERROR, NULL, 0, SV("child dumped core."), stderr);
            log_command(cmd, argc, argv);
            return EXIT_FAILURE;
        }

        case CLD_STOPPED: {
            exp_log(LOG_ERROR, NULL, 0, SV("child stopped."), stderr);
            log_command(cmd, argc, argv);
            return EXIT_FAILURE;
        }

        case CLD_TRAPPED: {
            exp_log(LOG_ERROR, NULL, 0, SV("child trapped."), stderr);
            log_command(cmd, argc, argv);
            return EXIT_FAILURE;
        }

        case CLD_CONTINUED: {
            exp_log(LOG_ERROR, NULL, 0, SV("child continued."), stderr);
            log_command(cmd, argc, argv);
            return EXIT_FAILURE;
        }

        default: {
            exp_log(LOG_ERROR, NULL, 0, SV("unknown child status."), stderr);
            log_command(cmd, argc, argv);
            return EXIT_FAILURE;
        }
        }
    }
}

#else
#error "unsupported host OS"
#endif
