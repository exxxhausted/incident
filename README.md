# Incident

"Incident" is a header-only C++23 library that allows you to use the graph abstraction in your projects.

Library architecture consists of some main aspects: 

## 1. Iterator + descriptor design

**Iterators** are used only for traversal and are fully compatible with `std::ranges`.  
They are lightweight and provide only `++`, `==`, and `operator*` (which returns a **descriptor** by value).

**Descriptors** (vertex descriptor, edge/arc descriptor) are the main way to access graph elements.  
They provide methods like `.data()`, `.adjacentVertices()`, `.from()`, `.to()`, `.incidentEdges()`, etc.  
Descriptors are cheap to copy and can be stored, compared, and passed around safely as long as the referenced element exists.

This separation gives you:
- Clear and efficient traversal (`for (auto v : graph.vertices())`)
- Direct access to vertex/edge data (`v.data()`)
- No need to dereference iterators manually – the range‑based loop gives you descriptors directly

## 2. Topology protection 

The API guarantees that you cannot accidentally break the graph topology.  
Iterators are only used internally for iteration;
all modifications (adding/removing vertices and edges/arcs) are performed via descriptors returned by the graph’s methods.

## 3. Integration with std::ranges

All graph traversal methods return `std::ranges::subrange` views that are compatible with range-based for.  
Examples:

`for (auto v : graph.vertices()) { ... }`  
`for (auto e : v.incidentEdges()) { ... }`  
`for (auto a : vertex.adjacentArcs()) { ... }` 

It is neсessary to use `auto` without any modifyers. For const-access you should call const-range-returning method.

## 4. Header-only and single‑header edition 
There is no way to precompile difficult template code, so header-only distribution is only possible.  
You always can include all content of "Incident" immediately by using `single_header/incident.hpp` in your `third_party`

## 5. Rust‑style error handling

Operations that can fail (like adding a vertex that already exists, or adding an edge
when vertices are missing or the graph forbids it) do not throw exceptions.
Instead they return `std::optional<T>` and `std::expected<T, Err>`. You get either a valid descriptor or std::nullopt.

This forces you to check the result explicitly. No hidden crashes. No try/catch.

# Core Concepts

The library is built around graph theory concepts. Currently only undirected
graphs are implemented, but the design will be extended to directed graphs.

### Undirected graph 
Edges have no direction. Edge (u,v) is the same as (v,u).

### Directed graph 
Arcs have a direction: from source to target. Arcs are not symmetric.

## All Graphs may be:

### 1. Abstract
No restrictions. Do with topology whatever you want.

### 2. Pseudo
Pseudograph allows loops (edge from a vertex to itself) and multipleedges between the same pair of vertices. No restrictions, except vertex uniqueness

### 3. Multi
Multigraph allows multiple edges between the same vertices, but forbids loops. This matches the definition from my discrete math course.

### 4. Simple
Simple graph forbids both loops and multiple edges.

All three share the same interface where possible. There are ways to switch between them.
