.file "exp_start.s"

// void _exp_start(void)
.globl _exp_start
.type _exp_start, @function
_exp_start:
  call main
  mov %rax, %rdi
  call _exp_sysexit
.size _exp_start, .-_exp_start


