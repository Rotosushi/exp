.file "exp_byte.s"

// void _exp_byte_copy(u8 *restrict dst, u8 *restrict src, u64 count)
// %rdi - dst
// %rsi - src
// %rdx - count
.globl _exp_byte_copy
.type _exp_byte_copy, @function
_exp_byte_copy:
  // if count == 0: return
  testq %rdx, %rdx
  jz done
  // the rep prefix implicitly uses %rcx as the count register
  movq  %rdx, %rcx
  rep movsb
done:
  ret
.size _exp_byte_copy, .-_exp_byte_copy

.globl _exp_byte_copy_word
.type _exp_byte_copy_word, @function
_exp_byte_copy_word:
  // if count == 0: return
  testq %rdx, %rdx
  jz done

  movq  %rdx, %rcx
  shrq  $3, %rcx
  rep movsq

  movq  %rdx, %rcx
  andq  $7, %rcx
  rep movsb
done:
  ret
.size _exp_byte_copy_word, .-_exp_byte_copy_word

.section .note.GNU-stack,"",@progbits
