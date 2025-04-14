We want to represent x86 instructions in C. How can we go about this?

## Ideas

Well, lets imagine:
1. We directly encode instructions:

		```
		struct x86_64_Instruction {
			u8 length;
			u8 data[15];
		};
		```
		or

		```
		struct x86_64_Instruction {
		 u8 data[15];
		};
		```

2. We directly encode the components of instructions:

		```
		struct x86_64_Instruction {
			u8 legacy_prefix_0;
			u8 legacy_prefix_1;
			u8 legacy_prefix_2;
			u8 legacy_prefix_3;
			u8 rex_prefix;
			u8 escape_0;
			u8 escape_1;
			u8 opcode;
			u8 modrm;
			u8 sib;
			u8 displacement[8];
			u8 immediate[8];
		};
		```

3. We encode an abstraction of the instructions, with enough information to produce the actual encoding:

		```
		struct x86_64_Instruction {
			x86_64_Opcode;
			x86_64_Operand[3];
		};
		```

## Discussion

All three options are assumed to be correct encodings. If I made a mistake in the examples this is to be considered a mistake in the example and not in the final encoding. It is also not assumed that these are all of the possible options, it is simply the few that I could think of in this particular moment, not a comprehensive list.
What is up for discussion is the why we want to choose one option instead of the others.

So, to me, option 1 is preferable to option 2 if the goal is directly encoding the instruction as close to how it would appear in memory. There are some advantages to this, when producing output for the linker, we might be able to bypass the assembler. As we already have the correct in-memory-representation of each instruction required by the CPU itself.
The only major work left would be to encode the ELF or COFF formats, required by executables and libraries. There are a few caveats I am probably not seeing, for one, how do we handle addresses, especially of global labels. and for that matter how do we handle labels at all? as these do not exist in actual memory, they only exist to stand in for an address that is used within instructions. A proper solution to this probably puts the IMR of instructions on par with the complexity of an assembler itself. Not that this is bad in and of itself; for one we would then avoid interfacing with an external program, and all of the friction that causes. 

Option 3 is what we have currently. This places the least burden on the compiler to reason correctly about the precise syntax of the assembly code. And leaves more room for worrying about the precise semantics. Something that is critical regardless of how we are encoding the instructions themselves. This is the main advantage I can see for option 3. 

To put on my efficiency cap for a moment, the space overhead per instruction in options 1, 2, and 3 is greater than what it is if we were to lay out the instructions in memory for the CPU to execute. However, this overhead is actually there to give us some flexibility in rearranging and refactoring the x86 instructions. if we always use the maximum space possible for an instruction, then we have the ability to freely replace any instruction with any other within a sequence of instructions. This greatly simplifies the implementation of any code which wants to refactor the sequence of instructions.
I can imagine instead allocating an instruction on the heap, as it's precise sequence of bytes necessary, and then a sequence of instructions becomes a sequence of pointers. However this has traded one form of overhead for another, as now we have a pointers worth of overhead per instruction, and to read the data of any particular instruction we must follow a pointer. which can introduce significant latency in the worst case. From the principle of locality which the cache operates under we know it is much faster on average to have each following instruction appear in the memory directly following the previous instruction. I.e. in an array. not a list of pointers to memory elsewhere. This is not even to mention the overhead of calling malloc/free once per instruction, which is itself non-trivial.

## Descision

Option 3, is in my opinion the best solution pragmatically. So, something like:

	```
	struct x86_64_Instruction {
		x86_64_Opcode opcode;
		x86_64_Operand operand[3];
	};
	```

If I could somehow magically produce perfect code instantaneously then Option 1 would be my choice. Because it reduces the overhead of invoking a separate program for assembling the instructions and potentially even of laying out the assembled instructions into a binary directly, and thus we can potentially produce an executable or static/dynamic library directly, with minimal OS calls, minimal intermediate files, etc. 