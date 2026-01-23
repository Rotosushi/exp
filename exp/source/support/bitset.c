/*
 * File: bitset.c                                                              *
 * Project: support                                                            *
 * Created on: Wednesday, March 26th 2025                                      *
 * Author: Cade Weinberg                                                       *
 */

#include "support/bitset.h"

extern void bitset_initialize(Bitset *restrict bitset);
extern void bitset_set(Bitset *restrict bitset, u8 index);
extern void bitset_clear(Bitset *restrict bitset, u8 index);
extern bool bitset_check(Bitset const *restrict bitset, u8 index);
