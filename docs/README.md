# README

exp is designed as a fairly standard compiler.
compilation takes place in a few stages

1. scanning
2. analysis
3. codegen
4. assembling
5. linking

[[Scanning]] consists of:

1. [[lexing]]
2. [[parsing]]

[[Analysis]] consists of:

1. [[typechecking]]

[[Codegen]] consists of:

1. [[register allocation]]
2. [[instruction selection]]

assembling consists of:

* a fork/exec which dispatches the task to [[as]]
	* generally provided by GNU [[as]] from the GNU [[binutils]]

linking consists of

 * a fork/exec which dispatches the task to [[ld]]
	 * generally provided by GNU [[ld]] from the GNU [[binutils]]

## design differences

One of the major differences between this compiler and the other compilers I have 
written is the lack of an abstract syntax tree. Instead the parser directly creates [[bytecode]] from the input [[source]]. This has implications on what kind of 
language constructs can be supported. However, in return we completely avoid the 
spacetime overhead of the tree like structure of the [[AST]].
If it is not too restrictive a constraint I am keeping this design decision going forward.

## [[Language]]

## [[Codegen]]

Codegen is handled with a variant of the classical substitution technique (also called macro substitution) whereby each bytecode instruction is simply substituted by an equivalent or series of equivalent target language instructions. This is what passes for "Instruction Selection" within the compiler. Given that data must be worked on in registers we must provide "Register Allocation" and for that we use an adaptation of the Linear Scan technique. which can loosely be described as "use the next available register." All local variables are allocated on the heap, including temporaries which are 
too large to fit within a single register.

There are a few points I am considering
* using the bytecode representation allows us to define target independent optimizations. 
* given we use a similar bytecode representation of x86_64 assembly, we can define target specific optimizations.
* I see no theoretical limitations of the bytecode format compared to a tree like format in terms of what can be expressed in a programming language. However so many languages use ASTs and multi-stage lowering that I feel I must simply be ignorant of precisely what it is that cannot be expressed.
* It is arduous to define our own code generation. Error prone, Highly delicate and extremely complex, it holds not one but three NP hard problems and they are interdependent. This is both the reason why I am doing this project, and the reason why any serious compiler should depend on LLVM to use the work that others have already done. The best way to learn is by doing, and that is what I intend to do. I have no expectations that the code quality will approach anything as good as what LLVM can do, unless I somehow spend several lifetimes working on just this aspect of the compiler. And doing that for my own bespoke language is actually pointless, when languages like Zig, Rust and Nim exist which fill in the inadequacies of C better than, or equivalent to what I can come up with myself.
* I am using C simply due to it's ubiquitous support in the Linux ecosystem. It is also a language I am already comfortable with using. Also it forces me into the position of writing much of what I need from scratch, which is the best way to learn. I really like Zig for it's simplicity. I appreciate what Rust is doing as well. I haven't looked at other languages nearly as much as I have studied these two, and neither of those as much as I have C/C++.
* Why not C++? simply because I wanted to have less support, and thus more that I had to support myself, and thus more that I had to learn. Was this a smart choice? from the perspective of a compiler that other people would be interested in using, I would say no, absolutely not. The thing barely works and has nearly zero features. From the perspective of what I have learned, I would say yes, though mostly in the realm of the code generation aspects, and the finer details of how we lower the IR. Having written multiple compilers using C++ It is far too easy to reach for large expensive abstractions to accomplish a task, whereas in C construction of the large complex abstraction is itself a large complex task, and thus dissuades one from taking that route. 

## [[Targets]]

We only support x86-64 Linux System-V ELF

## Curiosities

Why the name "exp"?

in short, because I am writing this program for the "exp"erience.
But it amuses me that it is also a language full of "exp"ressions.
and I can use it to "exp"lore the design space of programming languages.
I don't consider myself an "exp"ert, but working on this project might 
just "exp"idite the process. Putting this project out there for anyone to 
see and critique does make me feel "exp"osed.

