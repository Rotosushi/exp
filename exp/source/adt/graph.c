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
 * along with exp.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <assert.h>
#include <stdlib.h>

#include "adt/graph.h"
#include "support/allocation.h"
#include "support/array_growth.h"
#include "support/panic.h"

static Edge *edge_create(u64 target, Edge *next) {
    Edge *edge   = allocate(sizeof(Edge));
    edge->target = target;
    edge->next   = next;
    return edge;
}

static void edge_destroy(Edge *restrict edge) {
    assert(edge != NULL);
    Edge *tmp = NULL;
    while (edge->next != NULL) {
        tmp        = edge->next;
        edge->next = tmp->next;
        deallocate(tmp);
    }
    deallocate(edge);
}

static void edge_prepend(Edge *restrict edge, u64 target) {
    Edge *new  = edge_create(target, edge->next);
    edge->next = new;
}

SparseDigraph sparse_digraph_create() {
    SparseDigraph g;
    g.length   = 0;
    g.capacity = 0;
    g.list     = NULL;
    return g;
}

void sparse_digraph_initialize(SparseDigraph *restrict g) {
    assert(g != NULL);
    g->length   = 0;
    g->capacity = 0;
    g->list     = NULL;
}

void sparse_digraph_destroy(SparseDigraph *restrict g) {
    assert(g != NULL);

    for (u64 i = 0; i < g->length; ++i) {
        Edge *edge = g->list[i];
        if (edge != NULL) { edge_destroy(edge); }
    }

    g->length   = 0;
    g->capacity = 0;
    deallocate(g->list);
    g->list = NULL;
}

static bool sparse_digraph_full(SparseDigraph *restrict graph) {
    return graph->capacity <= (graph->length + 1);
}

static void sparse_digraph_grow(SparseDigraph *restrict graph) {
    Growth_u64 g    = array_growth_u64(graph->capacity, sizeof(Edge *));
    graph->list     = reallocate(graph->list, g.alloc_size);
    graph->capacity = g.new_capacity;
}

u64 sparse_digraph_add_vertex(SparseDigraph *restrict graph) {
    assert(graph != NULL);

    if (sparse_digraph_full(graph)) { sparse_digraph_grow(graph); }

    u64 vertex          = graph->length;
    graph->list[vertex] = NULL;
    graph->length += 1;
    return vertex;
}

void sparse_digraph_add_edge(SparseDigraph *restrict graph,
                             u64 source,
                             u64 target) {
    assert(graph != NULL);
    assert((source < graph->length) && "source vertex does not exist.");
    assert((target < graph->length) && "target vertex does not exist.");

    Edge **edge = graph->list + source;
    if (*edge == NULL) {
        *edge = edge_create(target, NULL);
    } else {
        edge_prepend(*edge, target);
    }
}

void vertex_list_destroy(VertexList *restrict vl) {
    vl->capacity = 0;
    vl->count    = 0;
    deallocate(vl->list);
    vl->list = NULL;
}

static VertexList vertex_list_create() {
    VertexList vl;
    vl.capacity = 0;
    vl.count    = 0;
    vl.list     = NULL;
    return vl;
}

static bool vertex_list_full(VertexList *restrict vl) {
    return vl->capacity <= (vl->count + 1);
}

static void vertex_list_grow(VertexList *restrict vl) {
    Growth_u64 g = array_growth_u64(vl->capacity, sizeof(u64));
    vl->list     = reallocate(vl->list, g.alloc_size);
    vl->capacity = g.new_capacity;
}

static void vertext_list_append(VertexList *restrict vl, u64 vertex) {
    if (vertex_list_full(vl)) { vertex_list_grow(vl); }

    vl->list[vl->count] = vertex;
    vl->count += 1;
}

VertexList sparse_digraph_vertex_fanout(SparseDigraph *restrict graph,
                                        u64 vertex) {
    assert(graph != NULL);
    assert((vertex < graph->length) && "vertex does not exist.");

    VertexList vl = vertex_list_create();
    Edge *edge    = graph->list[vertex];
    while (edge != NULL) {
        vertext_list_append(&vl, edge->target);
        edge = edge->next;
    }
    return vl;
}

static bool list_contains_vertex(Edge *edge, u64 vertex) {
    while (edge != NULL) {
        if (edge->target == vertex) { return 1; }
        edge = edge->next;
    }
    return 0;
}

VertexList sparse_digraph_vertex_fanin(SparseDigraph *restrict graph,
                                       u64 vertex) {
    assert(graph != NULL);

    VertexList vl = vertex_list_create();
    for (u64 i = 0; i < graph->length; ++i) {
        Edge *edge = graph->list[i];
        if ((i != vertex) && (edge != NULL) &&
            list_contains_vertex(edge, vertex)) {
            vertext_list_append(&vl, i);
        }
    }

    return vl;
}
