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

set(LIBEXP_START_SOURCES
  ${LIBEXP_SOURCE_DIR}/exp_start.s
)

add_library(libexp_start
  ${LIBEXP_START_SOURCES}
)

set_target_properties(libexp_start
  PROPERTIES
  ARCHIVE_OUTPUT_NAME exp_start
)


set(LIBEXP_SOURCES
  ${LIBEXP_SOURCE_DIR}/exp_syscalls.s
)

add_library(libexp
  ${LIBEXP_SOURCES}
)

set_target_properties(libexp
  PROPERTIES
  ARCHIVE_OUTPUT_NAME exp
)


