cmake_minimum_required(VERSION 3.20)

add_test (NAME "smoke test" COMMAND exp -v)

add_subdirectory(unit_tests)
add_subdirectory(end_to_end_tests)
