.file "exp_memory.s"

// void __exp_memcpy(u8 *restrict dst, u8 *restrict src, u64 count)
// %rdi - dst
// %rsi - src
// %rdx - count
.globl __exp_memcpy
.type __exp_memcpy, @function
__exp_memcpy:
  // if count == 0: return
  testq %rdx, %rdx
  jz byte_done
  // the rep prefix implicitly uses %rcx as the count register
  movq  %rdx, %rcx
  rep movsb
byte_done:
  ret
.size __exp_memcpy, .-__exp_memcpy

.globl _exp_byte_copy_word
.type _exp_byte_copy_word, @function
_exp_byte_copy_word:
  // if count == 0: return
  testq %rdx, %rdx
  jz word_done

  movq  %rdx, %rcx
  shrq  $3, %rcx
  rep movsq

  movq  %rdx, %rcx
  andq  $7, %rcx
  rep movsb
word_done:
  ret
.size _exp_byte_copy_word, .-_exp_byte_copy_word

.section .note.GNU-stack,"",@progbits
