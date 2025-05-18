# exp

I am writing this to learn about programming language implementation.

## Build

There are no build dependencies other than the C standard library.
Though I have some inclination to use mi_malloc, simply to get the runtime of the language 
up off the ground with a stable and secure malloc implementation.

Notes:
* The only supported Host Operating System is Linux.
  * Windows support is the next target
* The only supported Host Architecture is x86-64.
  * RISCV64 and ARM64 are the next targets
* we do not optimize the assembly
  * I have an idea for how to do constant folding, and maybe function inlining but not much else
    I am familiar with the concept that graph covering is how modern instruction selection/scheduling 
    and optimization is performed, but how that plays out in practice I have no real idea.
* We do not support debugging information, 
  * the plan is to support DWARF (probably version 4 as I assume it's more widely supported than 
    DWARF5, the latest revision, and still rather recent. I think we can mostly pretend to be C from 
    the perspective of the debugger.).
* Assembling is done with a fork/exec of "as", and this is hardcoded for now.
  * The plan is to allow this to be an injection point, to replace with another assembler that 
    supports x86 assembly. Though the syntax accepted by different assemblers varies widely so 
    that seems rather difficult. Additionally, as with other major compilers allowing parameters 
    to be passed through to the assembler seems very reasonable
* Linking is done with a fork/exec of "ld", and this is also hardcoded for now.
  * The plan is to allow this to be an injection point as well, and this seems like something a 
    bit easier to allow for more flexibility, just given how it seems like most linkers available 
    for linux aim for compatibility with GNU ld. Additionally, as with the assembler it seems rather 
    straightforward to allow passing through parameters to the linker.


Standard CMake usage should work:

    cmake -S . -B build -DCMAKE_BUILD_TYPE={Release|Debug|RelWithDebInfo|MinSizeRel}
    cmake --build build

There is no install target currently
The tests are always built, they can be run with:

    ctest --test-dir build

## Details

see [docs/README.md]

## Thanks

Thank you to:

* [LLVM](https://llvm.org/)
  * particularly [Kaleidoscope](https://llvm.org/docs/tutorial/)
* [cppreference](https://en.cppreference.com/w/)
* Robert Nystrom [Crafting Interpreters](https://craftinginterpreters.com/)
  * [clox](https://github.com/munificent/craftinginterpreters)
* AMD [AMD64 Architecture Programmers Manual](https://www.amd.com/content/dam/amd/en/documents/processor-tech-docs/programmer-references/40332.pdf)
* GNU [Binutils](https://www.gnu.org/software/binutils/)
* Aho, Lam, Sethi, Ullman [Compilers: Principles, Techniques, and Tools](https://en.wikipedia.org/wiki/Compilers:_Principles,_Techniques,_and_Tools)
* Benjamin Pierce [Types and Programming Languages](https://www.cis.upenn.edu/~bcpierce/tapl/)
* Everyone else who has supported me while I have been working on this
