https://en.wikipedia.org/wiki/Bytecode


a dynamic array has a favorable memory access pattern when accessing 
each item within the array, especially when comparing to a linked list 
approach. [Bjarne](https://www.youtube.com/watch?v=YQs6IC-vgmo&t=41s)
and given that analyzing the body of a function means analyzing 
the instructions which compose said function, I hypothesize that accessing 
instructions stored as [[bytecode]] will be faster than accessing instructions 
stored as a linked list of objects.

in order to be as efficient with large allocations of instructions. (i.e. large functions)
the size of each instruction is kept as small as possible.
and the percentage of unused bytes to used bytes (padding) within the struct 
is kept to a minimum

#### Instructions

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
coinciding with opcodes only having 128 forms. Where is the room for vector instructions, debugging instructions, all of the binary and unary instructions, profiling instructions, etc, etc... 
128 is a lot, but is it enough?

profiling instructions might just be composite with existing instructions.
and that might hold for other kinds of instruction. 

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

The potential gain in the performance of any subroutine that needs to iterate over a given dynamic array over the linked list version doesn't even require single word sized instructions, it's just that with single word sized instructions, you are the most likely you can be that when you go to iterate over all of the instructions in a block you won't cause a cache miss.

I feel like much of what is done by the compiler, such as typechecking, optimization, and code generation, require iterating over some of or all of the instructions in a given function.
Thus it becomes clear that the in memory representation of the IR affects the performance of every part of the system that uses the IR.

so it feels very important to make the IR as small as possible, while maintaining enough flexibility that the language doesn't feel unusable, nor the compiler impossible to write.

However, if the current iteration is too extreme, I feel as though a layout like

	4 bytes per operand data
	1 byte for operand kind
	1 byte for instruction kind

then that's (4 * 3) + (2 bytes padding) + 1 + 1 = 12 + 4 = 16 bytes, or two 8 byte words per instruction. Which on my machine means 4 instructions per cache line.

this should have a negligible impact on performance (we aren't jumping to 8X or 16X size), but suddenly we can reference a u32s worth of everything. and a u8s worth of kinds of everything.


This reduces the number of instructions that fit in a single cache line to 4. and it doubles the size of the allocation for every array of bytecode. 

if we allow for a larger number of operand kinds, we can represent all integral scalar types in the language which fit into an operand, into operands. this includes 
u8, u16, u32, 
i8, i16, i32,
f32, 
bool, and nil.
this is 9 operand kinds, however, we gain much faster interpretation of instructions which 
use these integral types. as all of their data is stored directly within the instruction using said data. virtually guaranteeing a cache hit when accessed.


if we allow operands to be 64 bits, then we can place pointers into them. and due to the padding added we can expand kinds to two bytes each. or keep them 1 byte each and 
get 4 bytes of padding. which can be used for other purposes.
an instruction becomes at least 2 + 2 * 3 + 8 * 3 = 64 bytes

which is 4 words long, so it's 4X size. and with a 64 byte cache line, we can only fit a single instruction per cache line, but there are many cache lines available. 

The benefit of being able to use pointers directly within instructions is the removal of 
'integer handles'. such as the one we use for names appearing in instructions. To access a 
name we first use the integer stored in the instruction to access an array, which itself stores 
the pointer to the actual textual data. if we could use a pointer in the instruction, we could 
just store the pointer to the actual textual data instead. which removes a layer of indirection,
as well as removing any limit to the number of names available to be used within the compiler. (beyond the 2^48 limit on the capacity of a single symbol table)
This has the same effect, on each kind of operand we use integer handles for.
which is, values and names.







