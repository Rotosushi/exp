
exp is designed as a fairly standard compiler. 
compilation takes place in a few stages

1. scanning
2. analysis
3. codegen
4. assembling
5. linking

scanning consists of

1. [[lexing]]
2. [[parsing]]

analysis consists of 

1. [[typechecking]]

codegen consists of 

* [[register allocation]]
* [[instruction selection]]

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

There are a few points I am considering
* using the bytecode representation allows us to define target independent optimizations. 
* given we use a similar bytecode representation of x86_64 assembly, we can define target specific optimizations.
* I see no theoretical limitations of the bytecode format compared to a tree like format in terms of what can be expressed in a programming language. 
* It is arduous to define our own code generation. Error prone, Highly delicate and extremely complex, it holds not one but three NP hard problems and they are interdependent. This is both the reason why I am doing this project, and the reason why I should depend on LLVM to use the work that others have already done. The best way to learn is by doing, and that is what I intend to do. I have no expectations that the code quality will approach anything as good as what LLVM can do, unless I somehow spend several lifetimes working on just this aspect of the compiler. And doing that for my own bespoke language is actually pointless, when languages like Zig, Rust and Nim exist which fill in the inadequacies of C better than, or equivalent to what I can come up with myself.
* I am using C simply due to it's ubiquitous support in the Linux ecosystem. It is also a language I am already comfortable with using. I really like Zig for it's simplicity. I appreaciate what Rust is doing. I haven't looked at other languages nearly as much as I have studied these two, and neither of those as much as I have C/C++.
* Why not C++? simply because I wanted to have less support, and thus more that I had to support myself, and thus more that I had to learn. Was this a smart choice? from the perspective of a compiler that other people would be interested in using, I would say no, absolutely not. The thing barely works and has nearly zero features. From the perspective of what I have learned, I am unsure, I would have to compare what I would have learned doing it the other way and then I could say, but I can't just turn around and do that. I already learned what I learned.
