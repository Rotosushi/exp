// Copyright (C) 2024 Cade Weinberg
//
// This file is part of exp.
//
// exp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// exp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with exp.  If not, see <http://www.gnu.org/licenses/>.
/**
 * @brief provides functions for computing the
 * min or max of two values of integer and unsigned
 * integer types.
 *
 */
#pragma once

int imin(int a, int b);
long lmin(long a, long b);
long long llmin(long long a, long long b);

unsigned umin(unsigned a, unsigned b);
unsigned long ulmin(unsigned long a, unsigned long b);
unsigned long long ullmin(unsigned long long a, unsigned long long b);

int imax(int a, int b);
long lmax(long a, long b);
long long llmax(long long a, long long b);

unsigned umax(unsigned a, unsigned b);
unsigned long ulmax(unsigned long a, unsigned long b);
unsigned long long ullmax(unsigned long long a, unsigned long long b);
