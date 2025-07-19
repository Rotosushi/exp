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
#ifndef EXP_CODEGEN_X86_IMR_INCOMING_ARGUMENT_ALLOCATOR_H
#define EXP_CODEGEN_X86_IMR_INCOMING_ARGUMENT_ALLOCATOR_H

#include "codegen/x86/imr/local_allocator/stack_allocator.h"

/*
 * NOTE: System-V x86-64 ABI:
 * "This is a 64-bit platform. The stack grows downwards. Parameters to
 * functions are passed in via the registers rdi, rsi, rdx, rcx, r8, and r9.
 * Floating-point parameters are passed in via xmm0 through xmm7. Unlike the
 * standard parameter passing registers, floating point registers are not
 * preserved between function calls. Any additional arguments that do not fit in
 * these registers are passed on the stack in reverse order. Parameters passed
 * via the stack may be modified by the called function. Functions are called
 * using the call instruction, which pushes the address of the next instruction
 * onto the stack and jumps to the operand. Functions return to the caller using
 * the ret instruction, which pops the return address from the stack and jumps
 * to it. The stack is 16-byte aligned just before the call instruction is
 * executed.
 *
 * Functions preserve the registers rbx, rsp, rbp, r12, r13, r14, and r15; while
 * rax, rdi, rsi, rdx, rcx, r8, r9, r10, r11 are scratch registers. The return
 * value is stored in the rax register, or if it is a 128-bit value, then the
 * higher 64-bits go in rdx. Optionally, functions push rbp such that the
 * caller-return-rip is 8 bytes above it, and set rbp to the address of the
 * saved rbp. This allows iterating through the existing stack frames. This can
 * be eliminated by specifying the -fomit-frame-pointer GCC option.
 *
 * Signal handlers are executed on the same stack, but 128 bytes known as the
 * red zone is subtracted from the stack before anything is pushed to the stack.
 * This allows small leaf functions to use 128 bytes of stack space without
 * reserving stack space by subtracting from the stack pointer. The red zone is
 * well-known to cause problems for x86-64 kernel developers, as the CPU itself
 * doesn't respect the red zone when calling interrupt handlers. This leads to a
 * subtle kernel breakage as the ABI contradicts the CPU behavior. The solution
 * is to build all kernel code with -mno-red-zone or by handling interrupts in
 * kernel mode on another stack than the current (and thus implementing the
 * ABI)."
 */

/*
 * #NOTE: so, in order to meet the ABI and be able to interoperate with C
 * (and by extension the Linux Kernel we are running on) we write this
 * data structure.
 *
 * register allocations are straightforward, we just allocate to the register.
 * Though LocalAllocator will have to mark the register down as used within the
 * RegisterAllocator, making this DS tightly semantically coupled to the
 * LocalAllocator.
 *
 * stack allocations are tricky.
 * - We have to allocate as if we were allocating onto the stack,
 *   taking into account alignment requirements
 * - and we have to allocate stack based arguments in reverse order
 *   of their appearance in the argument list.
 * - and since we cannot expect register allocated arguments to appear
 *   in the first six arguments exclusively there is no correcspondence
 *   between which argument we are being passed and if it is stack or
 *   register allocated. The first argument could be stack, the second
 *   a register, and the third a stack argument again, and these would be
 *   passed as 2nd argument in the first available register, 3rd arg on
 *   stack just above call frame and 1st arg just above that.
 */

typedef struct x86_IncomingArgumentAllocator {
    u8                 registers_used;
    x86_StackAllocator stack_arguments;
} x86_IncomingArgumentAllocator;

void x86_incoming_argument_allocator_create(
    x86_IncomingArgumentAllocator *restrict incoming_argument_allocator);

void x86_incoming_argument_allocator_destroy(
    x86_IncomingArgumentAllocator *restrict incoming_argument_allocator);

bool x86_incoming_argument_allocator_allocate_register(
    x86_IncomingArgumentAllocator *restrict incoming_argument_allocator,
    x86_Allocation *restrict allocation);

void x86_incoming_argument_allocator_allocate_stack(
    x86_IncomingArgumentAllocator *restrict incoming_argument_allocator,
    x86_Allocation *restrict allocation);

#endif // !EXP_CODEGEN_X86_IMR_INCOMING_ARGUMENT_ALLOCATOR_H
