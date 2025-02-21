# Copyright (C) 2025 Cade Weinberg
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

add_test (NAME "smoke test" COMMAND exp -v)

add_subdirectory(unit_tests)
#add_subdirectory(resource_tests)

if (CMAKE_BUILD_TYPE STREQUAL "Fuzzing")
  add_subdirectory(fuzzing_tests)
endif()
