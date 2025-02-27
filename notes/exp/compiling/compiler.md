a compiler is:
	source -> assembly

that assembly the input to the assembler; which is:
	assembly -> object

that object is the input to the linker; which is
	object -> executable | library

translating source code to assembly can be difficult
so we break the problem up into smaller pieces.

the first of which is "understanding" what the source code "says"
so we have to convert the source code into something which we 
can work on programmatically. We call this something 
Intermediate Representation (or [[IR]] for short).

This IR is taken by the rest of the compiler as input, in order to do the work 
of converting the source code into assembly.

the first task is checking to see that the input source code makes sense according 
to how the pieces of the language fit together. The rules of the language are partially 
stated in the form of the [[type-system]] of the language.

optionally, the compiler can [[optimize]] the IR into a more efficient version or smaller version.

optionally, the compiler can add [[debugging]] information into the IR

optionally, the compiler can add extra runtime validation code into the IR,

the next task is to convert the IR into the target [[assembly]] source code.

then finally invoke the assembler and the linker in order to produce the 
final executable or library.

we are relying on [[as]] and [[ld]] to perform assembling and linking respectively.

so we only need to be concerned with producing assembly.

as a simplification we begin with support for only one assembly language, [[x86-64]]

since this is going to run on an Operating System (that we aren't writing) we don't
have to be concerned with how the executable gets started, 
beyond setting the entry symbol in [[ld]].
Instead we created a default entry point. (specified by the default of ls) and compile that into a library,
and then have that call the programs entry point, which we require to be named main.

as a simplification we begin with support for only one operating system: [[Ubuntu]]

and since we are [[x86-64]] on [[Ubuntu]] we are bound to ([[system-v]] [[elf]] [[abi]])

#TODO add support for [[dwarf]] debug info
#TODO optimize the [[IR]]

the parser is the function which effectively performs the
	source -> [[IR]]
step of compilation.
and it's design is heavily influenced by the design of the [[clox]] [[parser]] 
in that it is a [recursive descent](https://en.wikipedia.org/wiki/Recursive_descent_parser) parser with
a "Pratt" table driven [operator precedence](https://en.wikipedia.org/wiki/Operator-precedence_parser) parser for binary operators.
(just as in the 'clox' language implementation)

the IR is represented as a fixed length [bytecode]. 
this is heavily influenced again by the "crafting interpreters" bytecode, with the major difference being a fixed length instruction. This is so we can index anywhere into the 
bytecode array, and know we are landing at the beginning of a valid instruction.

and as representation of both local constants, and temporary (or intermediate) values present in expressions we use Static Single Assignment form.
this is influenced by the design of LLVM assembly.
(https://llvm.org/docs/LangRef.html)