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

/// @file libexprt/exp_start.s

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
