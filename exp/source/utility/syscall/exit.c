

#include "utility/syscall/exit.h"
#include "utility/config.h"
#include "utility/unreachable.h"

#ifndef EXP_HOST_SYSTEM_LINUX
#error "unsupported host os"
#endif

#include <sys/syscall.h>
#include <unistd.h>

[[noreturn]] void sysexit([[maybe_unused]] i32 code) {
    syscall(SYS_exit, code);
    EXP_UNREACHABLE();
}
