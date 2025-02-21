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

/// @file libexprt/exp_copy.s

.file "exp_copy.s"

// #NOTE: #TODO _exp_copy (memcpy) on it's own could probably fill 
//  a whole semester course on optimization as it touches so much 
//  how how to make computers go fast. We are not doing any of that 
//  here. saving the optimization for another time.

// void _exp_copy(u8 *restrict dst, u8 *restrict src, u64 count)
// %rdi - dst
// %rsi - src
// %rdx - count
.globl _exp_copy
.type _exp_copy, @function
_exp_copy:
  // the rep prefix implicitly uses %rcx as the count register
  movq  %rdx, %rcx
  rep movsb
  ret
.size _exp_copy, .-_exp_copy

.section .note.GNU-stack,"",@progbits
