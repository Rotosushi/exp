// Copyright 2025 Cade Weinberg. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
#ifndef EXP_UTILITY_GRAPH_H
#define EXP_UTILITY_GRAPH_H
#include "utility/int_types.h"

typedef struct Edge {
    u64 target;
    struct Edge *next;
} Edge;

typedef struct Graph {
    u64 length;
    u64 capacity;
    Edge **list;
} Graph;

Graph graph_create();
void graph_destroy(Graph *restrict graph);

u64 graph_add_vertex(Graph *restrict graph);
void graph_add_edge(Graph *restrict graph, u64 source, u64 target);

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
VertexList graph_vertex_fanout(Graph *restrict graph, u64 vertex);

/**
 * @brief compute the list of verticies that have an edge
 * to the given vertex.
 *
 * @param graph
 * @param vertex
 * @return VertexList
 */
VertexList graph_vertex_fanin(Graph *restrict graph, u64 vertex);

#endif // !EXP_UTILITY_GRAPH_H
