
exp is designed as a fairly standard compiler. 
compilation takes place in a few stages

1. scanning
2. analysis
3. codegen
4. assembling
5. linking

scanning consists of

1. lexing, a.k.a tokenization
2. parsing

analysis consists of 

1. typechecking

codegen consists of 

* greedy register allocation
* substitution based instruction selection

assembling consists of 

* a fork/exec which dispatches the task to [[as]]
	* generally provided by GNU [[as]] from the GNU [[binutils]]

linking consists of

 * a fork/exec which dispatches the task to [[ld]]
	 * generally provided by GNU [[ld]] from the GNU [[binutils]]

One of the major differences between this compiler and the other compilers I have 
written is the lack of an abstract syntax tree. Instead the parser directly creates [[bytecode]] from the input [[source]]. This has implications on what kind of 
language constructs can be supported. However, in return we completely avoid the 
spacetime overhead of the tree like structure of the [[AST]].
If it is not too restrictive a constraint I am keeping this design decision going forward.

The type system of the compiler is a standard static type system.
We don't currently support user defined types, other than functions and tuples.
We don't currently support polymorphism. (Though strictly speaking tuples are a polymorphic type.)
we only support local constants.
we only support a single integer type "i64".
we only support the five basic arithmetic operations "+ - * / %"
we only support a two control flow structures call and return
we support functions
we support tuples
we do not support assignment currently.
we do not support global constants.

codegen, or more formally code generation is handled with the substitution technique (also called macro substitution) whereby each bytecode instruction is simply substituted by an equivalent or series of equivalent target language instructions. This is what passes for "Instruction Selection" within the compiler. Given that data must be worked on in registers we must provide "Register Allocation" and for that we use an adaptation of the Linear Scan technique. which can loosely be described as "use the next available register." All local variables are allocated on the heap.


