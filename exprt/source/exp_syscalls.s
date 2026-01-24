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

