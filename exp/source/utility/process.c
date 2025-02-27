/**
 * Copyright 2025 Cade Weinberg. All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#include "utility/process.h"
#include "utility/config.h"
#include "utility/panic.h"

#if defined(EXP_HOST_SYSTEM_LINUX)
    #include <sys/types.h>
    #include <sys/wait.h>
    #include <unistd.h>

i32 process(char const *cmd, char const *args[]) {
    pid_t pid = fork();
    if (pid < 0) {
        PANIC_ERRNO("fork failed");
    } else if (pid == 0) {
        // child process
        execvp(cmd, (char *const *)args);

        PANIC_ERRNO("execvp failed");
    } else {
        // parent process
        siginfo_t status = {};
        if (waitid(P_PID, (id_t)pid, &status, WEXITED | WSTOPPED) == -1) {
            PANIC_ERRNO("waitid failed");
        }

        if (status.si_code == CLD_EXITED) {
            i32 result = status.si_status;
            return result;
        } else {
            PANIC("child possibly killed by signal.");
        }
    }
}

#else
    #error "unsupported host OS"
#endif
