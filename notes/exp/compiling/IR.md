

The IR Is designed as a dynamic array of fixed length instructions.

a dynamic array has a favorable memory access pattern when accessing 
each item within the array, especially when comparing to a linked list 
approach.

in order to be as efficient with large allocations of instructions
the size of each instruction is kept as small as possible.
and the percentage of unused bytes to used bytes (padding) within the struct 
is kept to a minimum


I have taken this to a bit of an extreme
	if we set an upper limit to the number of operands to 3
	if we set 16 bits (two bytes) per operand to an instruction 
	and we allow 3 bits for the kind of operand it is
	and 7 bits for which instruction it is
	we can fit 1 instruction in 64 bits = (16 * 3) + (3 * 3) + 7 = 48 + 16
	or a single register in the CPU. (and eight instructions per cache line)
	This layout has the most favorable memory footprint I can imagine.
	equivalent to a single u64, and the size of a single pointer


this has a number of knock-on effects, which may or may not be outweighed by the memory savings.

most dramatic to me is that an operand is only able to represent 2^16 different values.
coinciding with operands only having 7 forms,
coinciding with opcodes only having 128 forms. Where is the room for vector instructions, debugging instructions, all of the binary 
and unary instructions, profiling instructions. 128 is a lot, but is it enough?

profiling instructions might just be composite with existing instructions.

this means that for instance, we currently have:

an an upper limit of 2^16 possible labels in a single translation unit.
the same for globally used constants, (literals, argument lists, etc...)
and an upper limit of 2^16 different static single assignment locals present within a function
immediate i64s can only be in the range of an i16 before they become global constants

now 65,536 is a lot. but it might not be quite enough if the scale of what is being compiled
is very large. However, the current exp compiler itself is well below 65,536 total symbols 
accross the whole project, I think. 
I would guess even including the symbols exported by the standard library.
in any single translation unit.
(there is the benefit that some symbols are not exported in header files, though we have no 
 inclusion mechanism in exp yet)

if your function has 65,536 local values in it, that function is very likely way to large and 
complex for a single function.
(and it might be that if your project has 65,536 labels in a single translation unit it needs to 
 be split into multiple programs.)

The potential gain in the performance of any subroutine that needs to iterate over a given dynamic 
array over the linked list version doesn't even require single word sized instructions, it's just 
that with single word sized instructions, you are the most likely you can be that when you go to 
iterate over all of the instructions in a block you won't cause a cache miss.

I feel like much of what is done by the compiler, such as typechecking, optimization, and code generation,
require iterating over all of the instructions in a given function body. 
the data format of the IR affects the performance of every part of the system that iteracts with the IR.

so it feels very important to make the IR as small as possible, while maintaining enough flexibility that
the language doesn't feel unusable

However, if the current iteration is too extreme, I feel as though a layout like
	same upper limit on number of operands
	4 bytes per operand union
	1 byte for operand tag
	1 byte for instruction tag
	then that's 128 bits, or 16 bytes per instruction.
	this should have a negligible impact on performance, but suddenly we can reference a u32s worth of everything. and we have many more bits to work with in terms of operand kind and opcode. 4 bytes. not 2.
	
which implies you need > 4GiB just to store the array of labels used within the single translation unit.
that is a massive project. it seems like this size is well beyond the point when the project 
could be split into multiple programs. (But show me a counter example)

This reduces the number of instructions that fit in a single cache line to 4. and it doubles the size of the allocation for every array of bytecode.

but, this size of IR is much smaller than a tree based approach. So using it is theoretically much faster than the tree based approach

if we allow for a larger number of operand kinds, we can represent all integral scalar types in the language which fit into an operand, into operands. this includes u8, u16, u32,
i8, i16, i32, f32, bool, and nil.
this is 9 operand kinds, however, we gain much faster interpretation of instructions which 
use these integral types.


