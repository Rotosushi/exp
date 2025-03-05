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
