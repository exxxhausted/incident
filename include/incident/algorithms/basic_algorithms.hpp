#ifndef EXX_BASIC_ALGORITHMS_HPP
#define EXX_BASIC_ALGORITHMS_HPP

#include <algorithm>

#include "../details/graph_concepts.hpp"

namespace exx::incident {

template<Graph G>
bool containsVertex(const G& g, const typename G::VertexValueType& data) {
    auto it = std::ranges::find_if(g.vertices(), [&](auto vd){ return vd.data() == data; });
    return it != g.vertices().end();
}

template<Graph G, typename Acc>
bool containsVertex(const G& g,
                     const typename G::VertexValueType& data,
                     const Acc& acc)
{ return acc.contains(data); }

template<Graph G>
auto findVertex(G& g, const typename G::VertexValueType& data)
    ->std::optional<typename G::VertexDescriptor>
{
    auto it = std::ranges::find_if(g.vertices(), [&](auto vd){ return vd.data() == data; });
    if (it != g.vertices().end())
        return *it;
    return std::nullopt;
}

template<Graph G>
auto findVertex(const G& g, const typename G::VertexValueType& data)
    ->std::optional<typename G::ConstVertexDescriptor>
{
    auto it = std::ranges::find_if(g.vertices(), [&](auto vd){ return vd.data() == data; });
    if (it != g.vertices().end())
        return *it;
    return std::nullopt;
}

template<Graph G, typename Acc>
auto findVertex(const G& g,
                 const typename G::VertexValueType& data,
                 const Acc& acc)
{ return acc.map(data); }

/*
template<Graph G, typename Pred>
auto findVertexIf(G& g, Pred pred)
    ->std::optional<typename G::VertexDescriptor>
{
    auto range = g.vertices()
                 | std::views::transform([](auto v) { return v.data(); });
    auto it = std::ranges::find(range, pred);
    if (it != range.end())
        return *it;
    return std::nullopt;
}

template<Graph G, typename Pred>
auto findVertexIf(const G& g, Pred pred)
    ->std::optional<typename G::ConstVertexDescriptor>
{
    auto range = g.vertices()
                 | std::views::transform([](auto v) { return v.data(); });
    auto it = std::ranges::find(range, pred);
    if (it != range.end())
        return *it;
    return std::nullopt;
}
*/
} // namespace exx::incident

#endif // EXX_BASIC_ALGORITHMS_HPP
