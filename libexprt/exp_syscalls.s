// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

.file "exp_syscalls.s"

// void exit(u64 status);
// %rdi holds the status
.globl _exp_sysexit
.type _exp_sysexit, @function 
_exp_sysexit:
  mov $60, %rax
  syscall
.size _exp_sysexit, .-_exp_sysexit

.section .note.GNU-stack,"",@progbits

