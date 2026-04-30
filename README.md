# exp

I am writing this to learn about programming language code generation.

## Build

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
