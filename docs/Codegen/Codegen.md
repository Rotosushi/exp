Code generation is the main function of the latter half of the compiler. It consists of 3 major parts, [[Instruction-Selection]], [[Instruction-Scheduling]], and [[Register-Selection]]. Which are all interdependent. And most modern techniques are known to be NP-complete.
The three problems are interdependent in that the solving of one requires the solving of the others. This can be observed with the AMD64 instruction [[IDIV]]. Consider generating the instruction from a given division instruction in the input bytecode.
Firstly, the division instruction has inputs which range over immediate values, [[SSA]] local variables, constant values, and global variables. Now, the major functionality of the division instruction can be implemented using the IDIV instruction. (mostly because we only allow natural number inputs to the division instruction, and IDIV handles both signed and unsigned division, at least the opcode is the same for both.) However, there are important things to note here, the IDIV instruction only takes as input a single register or memory location specifying the divisor. And thus the dividend must always be placed into rAX.
And the result is always placed in rDX:rAX. This means that in order to select the IDIV instruction, we have to schedule instructions prior to it which ensure the inputs are allocated to the correct registers. and we have to allocate the result to the predefined register. if we want the result stored into a local variable we have to schedule a copy after the IDIV instruction. This sort of thinking around the inputs and outputs of any given instruction must be handled by the generation of each instruction in the body of each function to be generated. 
## Strategy
As of right now, we only support un-optimized code generation (O0). To achieve this
We use a variant of a classic (circa 1960) strategy for code generation. The "Macro Expansion" technique. Whereby a given [[bytecode]] instruction is simply expanded into one or more target assembly instructions. We have explicitly programmed the expansion, and one technique for making the compiler support multiple targets would be to generate the expansions programmatically. However, given the historically known limitations of this technique, it seems pointless to put in all of that work for something so limited. If we were to upgrade the approach to codegen I think shifting to a more modern technique would be smarter.

The code generation portion of this project is really getting away from me.
and becoming a tar-pit. 

So, lets do a biopsy. what is so challenging about code generation?

well, the sheer magnitude of code that must be written to support a given 
target. I am having to juggle the assembly generated for a particular target
and the semantics of that assembly in the Intermediate Representation.
essentially I am defining the mapping for each instruction by hand.
except that this mapping is described in terms of more primitive "building blocks"
of assembly language.
Take for instance the current problem being worked on, Local Variable Initialization. The way we initialize primary types is very easy. since 
primary types are all register sized we can easily map to a single instruction
"mov." This breaks down when we consider larger types, such as tuples. 




## Codegen using Graph Covering and PBQP
