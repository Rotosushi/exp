
# IDIV - Signed Divide

Intel IA64-32 manual Volume 2.3.3:IDIV - Signed Divide

"Divides the (signed) value in the AX, DX:AX, or EDX:EAX (dividend) by the
 source operand (divisor) and stores the result in the AX (AH:AL), DX:AX, or
 EDX:EAX registers. The source operand can be a general-purpose register or a
 memory location. The action of this instruction depends on the operand size
 (dividend/divisor).

Non-integral results are truncated (chopped) towards 0. The remainder is
always less than the divisor in magnitude. Overflow is indicated with the
"#DE (divide error)" exception rather than with the CF flag.

the REX.R prefix permits access to additional registers (R8-R15). Use of
the REX.W prefix promotes operation to 64 bits. In 64-bit mode when REX.W is
applied, the instruction divides the signed value in RDX:RAX by the source
operand. RAX contains a 64-bit quotient; RDX contains a 64-bit remainder."

AMD64 Architecture Programmer's Manual Volume 3.3:IDIV - Signed Divide

"Divides the signed value in a register by the signed value in the specified
register or memory location. The register to be divided depends on the size
of the divisor.

When dividing a word, the dividend is in the AX register. The
instruction stores the quotient in the AL register and the remainder in the
AH register.

When dividing a doubleword, quadword, or double quadword, the
most-significant word of the dividend is in the rDX register and the
least-significant word is in the rAX register. After the division, the
instruction stores the quotient in the rAX register and the remainder in the
rDX register.

The instruction truncates non-integral results towards 0. The
sign of the remainder is always the same as the sign of the dividend, and the
absolute value of the remainder is less than the absolute value of the
divisor. An overflow generates a #DE (divide error) exception, rather than
setting the OF flag.

To avoid overflow problems, precede this instruction
with a CBW, CWD, CDQ, or CQO instruction to sign-extend the dividend."

# Encoding

1. 0xF6 /7 
2. REX 0xF6 /7
3. 0xF7 /7
4. 0xF7 /7
5. REX.W 0xF7 /7

note that 7 is the value of 

1 - is used for the byte form of the instruction.
