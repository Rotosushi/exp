# Copyright (C) 2024 Cade Weinberg
# 
# This file is part of exp.
# 
# exp is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# exp is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with exp.  If not, see <https://www.gnu.org/licenses/>.
cmake_minimum_required(VERSION 3.20)

create_test_sourcelist(Tests exp_unit_tests.c
  bitset_tests.c
  cli_options_tests.c
  constants_tests.c
  graph_tests.c
  lexer_tests.c
  number_conversion_tests.c
  parse_tests.c
  string_interner_tests.c
  string_tests.c
  string_view_tests.c
  symbol_table_tests.c
  type_interner_tests.c
)

add_executable (exp_unit_tests ${Tests})
target_include_directories(exp_unit_tests PUBLIC ${EXP_INCLUDE_DIR})
target_compile_options(exp_unit_tests PUBLIC ${EXP_COMPILE_OPTIONS})
target_link_options(exp_unit_tests PUBLIC ${EXP_LINK_OPTIONS})
target_link_libraries(exp_unit_tests PUBLIC exp_common m)


set (TestsToRun ${Tests})
remove (TestsToRun exp_unit_tests.c)

foreach (test ${TestsToRun})
  get_filename_component (TestName ${test} NAME_WE)
  add_test (NAME ${TestName} COMMAND exp_unit_tests ${TestName})
endforeach()
