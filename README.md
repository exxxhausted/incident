# "Incident" is a graph library

Header-only C++20 library that allows you to use the graph abstraction in your projects.

This is the initial release. The library will expand over time.

Library architecture consists of some main aspects: 

## 1. Iterator-based design

Iterators is a main source of truth, moreover I decided that it is most powerful and efficient way to interact with internals of your graphs.
This principle became the basis of current API. 

## 2. Topology protection 

API are based on idea of access through proxy-objects that protects the internal information about topology.
"Incident" provides a guarantee guarantee that you won't break the topology by dereferencing an iterator.  
Unfortunately there is no overload of `operator ->`.

## 3. Integration with std::ranges

All graph traversal methods return `std::ranges::subrange` views that are compatible with range-based for.  
Examples:

`for (auto v : graph.vertices()) { ... }`  
`for (auto e : v.incidentEdges()) { ... }`  
`for (auto a : vertex.adjacentArcs()) { ... }`  

## Header-only and single‑header edition 
There is no options to precompile difficult template code, so header-only distribution is only possible.  
You always can include all content of "Incident" immediately with usage of `single_header/incident.hpp` in your `third_party`

# Core concepts

## Directed and undirected abstract graphs
The only one type of graph represented now (30.05.26). There is no theoretical rules and restrictions: a great opportunity to be alone with the topology.
Iterators is only way you can access your data. Now you can bring an iterator singularity to your project).

## Thats all content of initial release. Thanks LSTU for providing me motivation to do this. 
London is a capital of Great Britain
