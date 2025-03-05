# exp

I am writing this to learn about programming language implementation.

## build

There are no build dependencies other than the C standard library.

The only supported Operating System is Linux currently.
The only supported Host Architecture is x86-64 currently.
We do not support debugging information currently.
Assembling is done with a fork/exec of "as", and this is hardcoded for now.
Linking is done with a fork/exec of "ld", and this is also hardcoded for now.

Standard CMake usage should work:

    cmake -S . -B build -DCMAKE_BUILD_TYPE={Release|Debug|RelWithDebInfo|MinSizeRel}
    cmake --build build

There is no install target currently
The tests are always built, they can be run with:

    ctest --test-dir build

## Details

see [docs/README.md]
