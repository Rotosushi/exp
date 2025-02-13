

Survey on Instruction Selection: An Extensive and Modern Review (c. 2013)
https://arxiv.org/abs/1306.4898

-------------------------------------------------------------

Survey on Combinatorial Register Allocation and Instruction Scheduling
https://arxiv.org/abs/1409.7628

We use liveness ranges (called a lifetime) to model the places within the body 
of a function that a particular local value must remain alive.

if two locals lifetimes overlap, then they are said to interfere. because they 
cannot be placed into the same register

some registers are pre-colored by certain constraints. such as the arguments to a function call (this requires following the [[abi]]) or the results of the [[x86-64]] "idiv" instruction, which 
writes its result to rax, rdx.

some register names are aliases for the same register. 
such as (al, ah, ax, eax, rax) in [[x86-64]].

spilling: moving a value from a register to the stack.
	This is slower than just working with the registers themselves, 
	and so we want to minimize spilling.
coalescing: assigning two non-interfering variables to the same register
	this can only be done if they are related by a copy instruction. 
	this can be seen as joining the lifetimes of two variables
live-range splitting: the inverse of coalescing, taking a single live range and splitting
	it into two ranges, by introducing a copy and a new variable. (plus the requisite 
	renaming of usages of the original)

dominator-tree: this is a tree which models the control flow between basic 
	blocks in the program. an edge exists from one basic block to another only if control 
	can flow from one basic block to the other. the immediate-dominator of a basic block 
	is the basic block that comes immediately before a basic block.
	

phi-functions: are functions which join the live ranges of a series of incoming lifetimes
	into a single resulting name. They are used to model assignment in [[ssa]] form.
	the phi function connects a set of incoming names, to a single outgoing name.
	you have one phi function per assigned name, and the number of incoming 
	variables is equal to the number of incoming branches which assign to the 
	incoming name.


the phases of a typical SSA-based register allocator

	Build: the interference graph using liveness information
	Spill: remove live rages while the register pressure is greater than the number of available registers
	MCS: find an ordering of the nodes of the graph that can be optimizally colored by a greedy algorithm.
	Color: assign registers to variables using the trivial greedy algorithm
	Coalesce: exchange registers between variables in order to maximize the number of variables related by copy instructions that share the same register. as these moves are suitable for Dead Code Elimination



-----


The design we had initially is one where we take the Abstract IR and lower that into a target specific IR. then we lower that into the target assembly. This works, really well in fact.
The only issue I have with it is that we have to repeat all of the high level structures of the 
abstract IR, such as functions, in this target specific IR (thus duplicating all of the allocations, and memory overhead, when we already have to allocate the full string of input source text and output assembly). We did remove some high level structure, such as removing any arguments or results from the call instruction, and being explicit about stack usage. But I have this feeling like we aren't getting all that much value from this 
repetition, and there must be some way of lowering from the abstract IR directly. 
For instance, is every target architecture supposed to repeat the IR pattern? 
That seems like way too much work, why can't we just associate an abstract IR instruction 
with a pattern of machine instructions? then we can just describe the patterns of a given 
target architecture and emit directly from the abstract IR. achieving target independence 
for instruction selection and scheduling. 



