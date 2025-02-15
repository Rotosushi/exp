// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

.file "exp_start.s"

// void _start(void)
.globl _start
.type _start, @function
_start:
  call main
  mov %rax, %rdi
  call _exp_sysexit
.size _start, .-_start

.section .note.GNU-stack,"",@progbits
