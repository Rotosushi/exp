cmake_minimum_required(VERSION 3.20)

add_test (NAME "smoke test" COMMAND exp -v)

add_subdirectory(libexp_test)
add_subdirectory(tests)
