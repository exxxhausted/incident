# "Incident" is a graph library

Header-only C++20 library that allows you to use the graph abstraction in your projects.

This is the initial release. The library will expand over time.

Library architecture consists of some main aspects: 

## 1. Iterator + descriptor design

**Iterators** are used only for traversal and are fully compatible with `std::ranges`.  
They are lightweight and provide only `++`, `==`, and `operator*` (which returns a **descriptor** by value).

**Descriptors** (vertex descriptor, edge/arc descriptor) are the main way to access graph elements.  
They provide methods like `.data()`, `.adjacentArcs()`, `.from()`, `.to()`, `.incidentEdges()`, etc.  
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

# Core concepts

## Directed and undirected abstract graphs
The only one type of graph represented now (30.05.26). There is no theoretical rules and restrictions: a great opportunity to be alone with the topology.
Iterators is only way you can access your data. Now you can bring an iterator singularity to your project).

## Thats all content of initial release. Thanks LSTU for providing me motivation to do this. 
London is the capital of Great Britain
