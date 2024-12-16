cmake_minimum_required(VERSION 3.20)

add_test (NAME "smoke test" COMMAND exp -v)

add_subdirectory(unit_tests)
#add_subdirectory(resource_tests)

if (CMAKE_BUILD_TYPE STREQUAL "Fuzzing")
  add_subdirectory(fuzzing_tests)
endif()
