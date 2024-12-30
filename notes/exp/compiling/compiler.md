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
can work on in memory. If we were somehow able to perform the 
translation from source code to assembly without allocating any 
extra memory, then this form of the compiler would be the fastest,
and least memory intensive form a compiler could take.
There is a program, which can copy memory from one location to another,
without performing any allocation, and that is the Linux command dd,
or the memcpy c function. However, we can notice that such a program 
also doesn't do anything with the bytes it moves. A compiler on the other hand
transforms one sequence of bytes into another, distinct yet dependent sequence 
of bytes. 
In order to encode meaning into a program, one must create a form of that meaning.
if for instance, we wanted to describe how many processor cycles each CPU instruction 
the compiler was capable of emitting took, such that the compiler was able to reason 
about how many cycles a particular sequence of instructions would take. We would need 
(probably) a table listing each of the instructions associated with their CPU cycles taken.
along with this there is also throughput, latency, and memory dependence which exists per 
instruction. with specific assemblies, the length of the encoding of each instruction can be 
varied. all of this data is pertinent to a discussion of which instructions are going to be used 
to carry out a given task within a computer. Were the task given to people, these are the kinds of considerations we would be making, when creating/writing the given assembly.
In order to ease the development of programs which run on computers, especially the large 
and complex programs which we have come to rely on in daily life (video games, graphical applications, websites, etc, etc.), compilers were created as tools for writing vast amounts of assembly without vast amounts of work on the part of the programmer.
This is a wonderful and admirable goal. It has worked out tremendously.
In order to encode the source text in memory, we want a structure which we (the compiler writers) can use to represent the structure and meaning of the source text, along with all of the meaning which is inferred by the semantics of the source text, and the semantics of the target machine. Essentially, the source code of a programming language is an abstraction 
which must be made concrete by the compiler, such that the program described by the 
source code is rendered in reality, and a physical computer may now behave/execute said program.

We call this structure Intermediate Representation (or [[IR]] for short).

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
#TODO add optimizations

the parser is the function which effectively performs the
	source -> [[IR]]
step of compilation.
and it's design is heavily influenced by the design of the [[clox]] [[parser]] 
in that it is a [recursive descent](https://en.wikipedia.org/wiki/Recursive_descent_parser) parser with
a "Pratt" table driven [operator precedence](https://en.wikipedia.org/wiki/Operator-precedence_parser) parser for binary operators.
(just as in the 'clox' language implementation)

the IR is represented as a fixed length [[bytecode]]. 
this is heavily influenced again by the "crafting interpreters" bytecode, with the major difference being a fixed length instruction. This is so we can index anywhere into the 
bytecode array, and know we are landing at the beginning of a valid instruction. My 
assumption is that this capability will make it much easier to find/replace instructions,
insert instructions into bytecode, and remove instructions from it, without destroying 
the validity of the surrounding instructions.

and as representation of both local constants, and temporary (or intermediate) values present in expressions we use Static Single Assignment form.
this is influenced by the design of LLVM assembly, which as far as i can tell is at least one of the intermediate representations that LLVM uses to compile programs.
(https://llvm.org/docs/LangRef.html)