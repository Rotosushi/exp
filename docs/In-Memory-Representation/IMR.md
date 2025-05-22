So, after watching [ThePrimeTime](https://www.youtube.com/watch?v=cvZArAipOjo&t=725s) react to a snippet of a lecture given by Bjarne Stroustrup on why vectors are almost always faster than linked lists, I had a thought,
The first phase of a compiler is nearly always a parser constructing an Abstract Syntax Tree (AST). now Tree data structures are essentially linked lists (or linked lists are essentially trees, whichever you like). So they come with the same performance caveats as linked lists. After watching [A](https://www.youtube.com/watch?v=rX0ItVEVjHc) [few](https://www.youtube.com/watch?v=IroPQ150F6c) talks on Data-Oriented-Design I wanted to try my hand at applying it's principles to compiler implementation/Architecture.

So, I asked myself, what is the compilers internal representation of the Source Code?
And how can this be improved in terms of the speed with which it can be read, modified, and written? Since ASTs are so "slow" (again, I feel it important to reiterate, that this is not a tested performance claim. This is a rule-of-thumb design choice, an expectation/assumption about how the preformace will play out in the future. Because I don't have two equivalent compilers for the same language with different implementations to make a real, scientific method style, comparison, I am setting out to write one compiler, If i have the time I'll write another ), what is a viable alternative? Well, after working through [Crafting Interpreters](https://craftinginterpreters.com/contents.html) 'clox' implementation, I learned that bytecode is a viable alternative to ASTs in terms of expressive power. And given that bytecode is stored in a single dynamic vector, I thought this would be a good start in terms of performance. 
The main Ideas so far are 
1. Don't generate an AST at all. Trees are slow, so the less time we have to spend with them the better, in terms of performance. (Note: They are unavoidable, as far as I know, once we get to the latter half of the compiler, but I get ahead of myself)
2. Use Bytecode as the In Memory Representation (IMR) of the source code within the compiler.

So, what should the bytecode look like? Well, I looked at the [X86](https://www.amd.com/content/dam/amd/en/documents/processor-tech-docs/programmer-references/40332.pdf) and [RISCV](https://lf-riscv.atlassian.net/wiki/spaces/HOME/pages/16154769/RISC-V+Technical+Specifications) specifications for inspiration, seeing as they are the in memory representation of what the computer actually executes. And I thought, when we are analyzing the IMR, as in Typechecking and Verification we are going to be iterating through it. when we optimize the program, we are going to be replacing instructions with other instructions, or replacing sections of instructions with other sections of instructions. For instance, function inlining is essentially taking a single call instruction and replacing it with the body of the called function, this can be a single instruction or many, depending upon the complexity of the called function. This means in terms of the IMR, iterating through the bytecode and updating arbitrary segments of it. either replacing instructions individually, (like with peephole optimizations. such as division/multiplication being replaced with bitshifts)
or expanding/contracting sections of bytecode, as with larger scale transformations.
So, to me, this rules out a variable length encoding, as in the 'clox' interpreter and the x86 language. Because replacing a single instruction now becomes more complex than 
`a[i] = x;` which is the fastest it could be. So, we want a fixed length encoding. 
With a fixed length encoding, replacing a section of instructions is as complex as an implementation of std::vector::insert_range,
which is not without challenge, it's just much less challenging than doing the same with a variable length encoding.
Now, the next question is how do we lay out the bytecode? Well, looking to the 'clox' interpreter we know that we can use numbers as indices into an array to represent local variables. And after reading about the [LLVM IR](https://llvm.org/docs/LangRef.html#syntax) and [Static Single-Assignment](https://en.wikipedia.org/wiki/Static_single-assignment_form) I thought we can use SSA integers to represent local variables. And after comparing and contrasting
x86 and riscv, I thought a three operand instruction set is far simpler to work with, I wanted each instruction to have a maximum of three operands.
And thus, my first attempt at a layout is:
```
// in "operand.h"
typedef enum OperandKind {
	OPERAND_KIND_SSA,
	OPERAND_KIND_REGISTER,
	// Q: with only 7 kinds available, is is really worth having 
	// 4 devoted to immediates?
	OPERAND_KIND_u8,
	OPERAND_KIND_i8,
	OPERAND_KIND_u16,
	OPERAND_KIND_i16,
} OperandKind;

typedef struct Operand {
	unsigned kind : 3; // 7 kinds maximum. (kinda small)
	unsigned data : 16;
} Operand;

...

// in "instruction.h"

typedef enum Opcode {
	OPCODE_RETURN,
	OPCODE_CALL,
	OPCODE_ADD,
	OPCODE_SUB,
	...
} Opcode;

typedef struct Instruction {
	unsigned opcode : 7;
	unsigned a_kind : 3;
	unsigned b_kind : 3;
	unsigned c_kind : 3;
	unsigned a_data : 16;
	unsigned b_data : 16;
	unsigned c_data : 16;
} Instruction;

...
// in "bytecode.h"
typedef struct Bytecode {
	u32 length;
	u32 capacity;
	Instruction *data;
} Bytecode;

...

```
Some benefits of this layout of an instruction is that sizeof(Instruction) == 8, which on 64 bit systems is equivalent to the word size of the CPU. meaning an instruction can fit inside of a register. And on my CPU, (AMD Ryzen 7 5800) the cache size is 64, meaning 8 instructions can fit inside of a single cache lane. This has a favorable impact on performance. If we use a register based interpreter, we can store values of whatever size we want in the registers, I was thinking something like 
```
typedef struct Register {
	unsigned kind : 16;
	unsigned data : 48;
} Register;

// sizeof == 8 again, which is good imo. plus we can use the same trick
// of integer tags into arrays to reference any data > 48 bits. and we 
// have 65535 kinds of data we can store in the register, so we have 
// plenty of room for a different array for each kind of type within the 
// compiler.
// OR

typedef struct Register {
	RegisterKind kind;
	RegisterData data;
} Register;

// were RegisterKind is an enum, and RegisterData is 
// another struct, and we don't worry as much about the size.
// because whatever data we want to access. (Pointers, Immediates,
// etc.) is right there when we load the memory holding the registers 
// array. 
```
I don't know which is better a-priori. Only by implementing both and
running non-trivial perf testing could I figure out which. Because we 
have to "dereference" the integer handle into another array of data 
with the first approach, and we have the data directly within the second
option, with the drawback that we might be wasting a lot of memory 
if the data we want to store is significantly smaller than the largest 
data it must be possible to store. There are time and space overheads to 
consider together. For instance, how many registers are there?
We could go with the LLVM approach of "infinite" but due to the IMR we are actually limited to 65535. however it is expensive to allocate that many registers if we are going to be 
using significantly fewer than that. Especially if the layout of a register itself is significantly greater than 8 bytes. 

additionally, We gain performance by having immediate values as potential values in operands, and not having to chase down the 
actual value through a chain of pointers. Especially with the most common operations we perform in the compiler.
So, If we allow ourselves more data per operand, while we increase the size of each instruction, we should gain a bit of performance 
when working with immediates.

So, as it stands now, the actual form of our IMR is:

```
typedef enum OperandKind : u8 {
	OPERAND_KIND_SSA,
	OPERAND_KIND_CONSTANT,
	OPERAND_KIND_LABEL,
	OPERAND_KIND_U8,
	OPERAND_KIND_U16,
	OPERAND_KIND_U32,
	OPERAND_KIND_U64,
	OPERAND_KIND_I8,
	OPERAND_KIND_I16,
	OPERAND_KIND_I32,
	OPERAND_KIND_I64,
} OperandKind;

typedef union OperandData {
	u32 ssa;
	Value const * constant;
	ConstantString const * label;
	u8 u8_;
	u16 u16_;
	u32 u32_;
	u64 u64_;
	i8 i8_;
	i16 i16_;
	i32 i32_;
	i64 i64_;
} OperandData;

typedef enum Opcode : u16 {
	OPCODE_RET,
	OPCODE_CALL,
	OPCODE_DOT,
	OPCODE_LOAD,
	OPCODE_NEG,
	OPCODE_ADD,
	OPCODE_SUB,
	OPCODE_MUL,
	OPCODE_DIV,
	OPCODE_MOD,
} Opcode;

typedef struct Instruction {
	Opcode opcode;
	OperandKind A_kind;
	OperandKind B_kind;
	OperandKind C_kind;
	OperandData A_data;
	OperandData B_data;
	OperandData C_data;
} Instruction;

```
This is a slight expansion in terms of the memory footprint, we use 32 bytes for a single instruction, which is 4 times larger. The main rationale behind this is that when we are expressing arithmetic expressions from user code, we very often use integers, so it makes sense to allow for each kind of integer to be present directly within the Instruction. This makes it very simple to add two i64's within a hypothetical evaluator. or, as it is now, to know which two i64 values are being added when we emit the x86-64 add operation, between them. additionally it can be seen that any datum which is 64 bits large can be fit into a Operand, which expands the breadth to include floats and doubles, as well as C pointers directly Which allows for a speedup in other areas, Labels used in operands can be a pointer to a ConstantString, which holds the data of the string directly, meaning we are one pointer away from the character data. Additionally, given the excess padding within the Instruction structure we can allow for a u16 for each OperandKind without adding any size overhead,
allowing for as many kinds of Operand as we need. (Honestly I would be surprised if we ever need more than 255 kinds of Operand) either that, or we have 4 bytes of padding to play around with in the Instruction, this can be used for anything we need in the future.
I think that the trade-off is worth it. We aren't bloating the instruction size by too much, and given that production quality representations of instructions are regulary much larger. I am specifically thinking of the zydis x86 disassembler where a single x86 instruction along with it's operands are collectively hundreds of bytes large, and this library is very performant, and not all that memory intensive either. Computers are simply very fast at moving data around these days, and while it is noble to attempt to push the representation to its absolute limit, it simply isn't necessary to allow for performance and ease development difficulty.

We still use a classic Environment for global declarations. Currently I have a open addressing linear-probed hash-table, with a load factor of 0.75. My thoughts on this are to somehow allow for a single hash table to be filled with all of the symbols visible to a single translation unit. While allowing for use-before-definition across separate modules defined across multiple files. This is because my plan is to have a separate thread (or process) each with it's own environment for multi-threading compiling multiple source files. 
The alternative would be to somehow have all the threads share a single environment and compile only the symbols defined within a given source file. I don't know which would be faster, given that resource sharing is such a difficult problem, but with multiple environments we have to process the same files over and over again. 

And then we have the backend to consider. Which from my beginners understanding. 
([register allocation](http://compilers.cs.ucla.edu/fernando/publications/drafts/survey.pdf) [instruction selection](https://link.springer.com/book/10.1007/978-3-319-34019-7) [combinatorial techniques](https://arxiv.org/abs/1409.7628) [compiler design](https://link.springer.com/book/10.1007/978-1-4612-5192-7))
There is a lot. like, a lot a lot to understand. it seems that the central data structure used to represent the flow of the program at both micro and macro levels is the Graph. There is the Call Flow Graph which details which functions call each other. (Given recursion this is a Directed Graph, without recursion it can be a Directed Acyclic Graph) There is the Control Flow Graph that details how control flows through the body of a procedure. (Given loops, this is a Directed Graph. if you somehow special case loops, you might be able to make it 
Directed Acyclic. But it's natural form is cyclic, so that seems more appropriate.) It can be argued that the control flow graph should include function calls, in which case it becomes a global control flow graph, and can be used for inter-procedural optimizations.
There is the Data-Flow Graph that details how data flows between the instructions within a block of bytecode. (If you are using SSA form, then this is Directed Acyclic.) ((or, between the representation of operations, such as binary operators, calls, and looping constructs, within whatever IR is being used))
There may be more that I am unaware of as well.
Then, since we want a modern, optimizing compiler, there are all of the data structures and algorithms for combinatorial instruction selection, register allocation, and instruction scheduling. Which are three algorithms all interdependent on one another.

Instruction Selection is choosing which instructions to use to represent the given IMR.
Instruction Scheduling is choosing the ordering of the selected instructions
Register Allocation is choosing which registers the selected instructions are going to operate upon.


As it stands now, we are using one the oldest, if not the oldest technique for instruction selection and scheduling, which is 
direct replacement, Or macro expansion as it has been called in the literature. I say direct replacement because that gives a 
better idea as to what is happening, and I don't have macros at all, this is handrolled C. 
And to be more specific direct replacement means that when for instance the codegeneration algorithm encounters an 
instruction like `add %0, 1, 2` where SSA local %0 is being assigned the result of adding 1 and 2, this is directly translated 
into an equivelent set of x86 instructions, say for instance 
```
	mov rax, 1
	add rax, 2
```
There are a lot of considerations here, first and foremost is register selection, and for that we use another classic, the 
linear-scan register allocation strategy, which is loosely speaking, "just allocate to the first, next available register; otherwise 
spill the longest lived allocation to the stack" and we enumerate the available registers 0-15 for x86, and go from there.
Notice that a single instruction is translated into two, that is standard for this approach, essentially you can think of it like
"What is the sequence of x86 assembly instructions which when emitted at this point will perform the behavior of the given 
instruction"
So, if the instruction was instead `add %2, %1, %0` the algorithm would first need to lookup where SSA locals %0 and %1 were allocated,
and use their locations for the operands to the x86 add instruction. I hope that is enough to give the gist of what's currently going on.

As should be obvious this style of instruction selection is not going to produce optimal x86 code. I am only aiming for correctness at 
this point. What is interesting is that from this perspective we have two places to consider optimizing the set of instructions.
We can optimize the in memory instructions, and we can optimize the target language instructions.
for instance, if we consider the peephole hoptimization which replaces a division by a power of two with a right bitshift, this can 
be done at the level of the in memory instructions, or it could be done while we are selecting the target language 
instructions. if done at the level of the in memory instructions then the optimization is target-independant, and it is target-dependent 
if done at the level of the target language instructions.











































