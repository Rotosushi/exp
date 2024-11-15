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
