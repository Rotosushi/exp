// Copyright (C) 2025 Cade Weinberg
// 
// This file is part of exp.
// 
// exp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// exp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with exp.  If not, see <https://www.gnu.org/licenses/>.

/// @file libexprt/exp_syscalls.s

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

