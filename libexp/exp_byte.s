.file "exp_byte.s"

// #NOTE: #TODO byte_copy (memcpy) on it's own could probably fill 
// a whole semester course on optimization as it touches so much 
// how how to make computers go fast. We are not doing any of that 
// here. saving the optimization for another time.

// void _exp_byte_copy(u8 *restrict dst, u8 *restrict src, u64 count)
// %rdi - dst
// %rsi - src
// %rdx - count
.globl _exp_byte_copy
.type _exp_byte_copy, @function
_exp_byte_copy:
  // the rep prefix implicitly uses %rcx as the count register
  movq  %rdx, %rcx
  rep movsb
  ret
.size _exp_byte_copy, .-_exp_byte_copy

.section .note.GNU-stack,"",@progbits
