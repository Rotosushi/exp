/**
 * Copyright (C) 2024 Cade Weinberg
 *
 * This file is part of exp.
 *
 * exp is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * exp is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with exp.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "utility/minmax.h"

int imin(int a, int b) { return (a > b) ? b : a; }

long lmin(long a, long b) { return (a > b) ? b : a; }

long long llmin(long long a, long long b) { return (a > b) ? b : a; }

unsigned umin(unsigned a, unsigned b) { return (a > b) ? b : a; }

unsigned long ulmin(unsigned long a, unsigned long b) {
  return (a > b) ? b : a;
}

unsigned long long ullmin(unsigned long long a, unsigned long long b) {
  return (a > b) ? b : a;
}

int imax(int a, int b) { return (a > b) ? a : b; }

long lmax(long a, long b) { return (a > b) ? a : b; }

long long llmax(long long a, long long b) { return (a > b) ? a : b; }

unsigned umax(unsigned a, unsigned b) { return (a > b) ? a : b; }

unsigned long ulmax(unsigned long a, unsigned long b) {
  return (a > b) ? a : b;
}

unsigned long long ullmax(unsigned long long a, unsigned long long b) {
  return (a > b) ? a : b;
}