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
// along with exp.  If not, see <https://www.gnu.org/licenses/>.
#ifndef EXP_UTILITY_GRAPH_H
#define EXP_UTILITY_GRAPH_H

#include "support/scalar.h"

typedef struct Edge {
    u64 target;
    struct Edge *next;
} Edge;

typedef struct Graph {
    u64 length;
    u64 capacity;
    Edge **list;
} SparseDigraph;

SparseDigraph graph_create();
void graph_destroy(SparseDigraph *restrict graph);

u64 graph_add_vertex(SparseDigraph *restrict graph);
void graph_add_edge(SparseDigraph *restrict graph, u64 source, u64 target);

typedef struct VertexList {
    u64 count;
    u64 capacity;
    u64 *list;
} VertexList;

void vertex_list_destroy(VertexList *restrict vl);

/**
 * @brief compute the list of verticies that the given
 *  vertex has an edge to.
 *
 * @param graph
 * @param vertex
 * @return VertexList
 */
VertexList graph_vertex_fanout(SparseDigraph *restrict graph, u64 vertex);

/**
 * @brief compute the list of verticies that have an edge
 * to the given vertex.
 *
 * @param graph
 * @param vertex
 * @return VertexList
 */
VertexList graph_vertex_fanin(SparseDigraph *restrict graph, u64 vertex);

#endif // !EXP_UTILITY_GRAPH_H
