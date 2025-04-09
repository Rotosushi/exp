# Instruction Encoding

An x86-32/64 instruction is a string between 1 and 15 bytes in length.

From the Intel64 Architecture Software Developers Manual
![[IA64_ASDM_V2_F2-1_InstructionFormat.png]]

From the AMD64 Architecture Programmer's Manual
![[AMD64_APM_V3_F1-1_InstructionEncodingSyntax.png]]

## REX Prefix

An Instruction is required to use the REX prefix iff:
* the instruction uses one of the extended (64 bit) registers
* the instruction uses one of the byte registers (SPL, BPL, SIL, DIL)
* the instruction uses a 64 bit operand


## References

* AMD64 Architecture Programmer's Manual (Combined Volumes 1-5)
	* Volume 3: General-Purpose and System Instructions
		* Section 1: Instruction Encoding (p.1277)
* Intel 64 and IA32 Architectures Software Developers Manual (Combined Volumes 1-4)
	* Volume 2: Instruction Set Reference
		* Chapter 2: Instruction Format (p.521)
 
