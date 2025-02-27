
#ifndef EXP_ENV_LOCAL_CONTROL_FLOW_GRAPH_H
#define EXP_ENV_LOCAL_CONTROL_FLOW_GRAPH_H

// A graph represents a set of
// verticies conntected by edges
// the local control flow graph
// models how control flows through
// the bytecode of a given block
// the vertices are thus the
// Instructions, the edges point to
// the next Tnstruction to be executed.
// and the edge holds the condition which
// causes that path to be taken. where the
// default condition is just fallthrough.
// if you draw out this graph, you can see
// how the control flows through the function.
// currently the local control flow is only
// the default fallthrough. So there isn't much
// point to a local control flow graph.
//
// you can also make a graph where the
// Instructions Operands are the vertices
// and you connect the incoming arguments
// of an Instruction to it's output operand(s).
// and this shows you the data flow.
//
// you can consider a graph where the functions
// are the vertices, and the edges represent
// which functions call another function.
// this is the global control flow graph.
//
// and I suppose you could consider a global
// data flow graph, but I don't know what it's
// vertex would be... it seems like it would need
// to be the same kind as a local data flow graph.
// because they are incoming function arguments.
// and the functions results.
// only, these results always occur at a local
// level, in another function.
// So an interprocedural data flow graph, is
// then equivalent to the interprocedural control
// flow graph, because data only flows between functions.
// it would allow you to see the
// fan-in of any functions argument. and would
// allow you to see the fan-in of some local result
// of a function call. You could also see the fan-out
// of a functions result data.

#endif // EXP_ENV_LOCAL_CONTROL_FLOW_GRAPH_H
