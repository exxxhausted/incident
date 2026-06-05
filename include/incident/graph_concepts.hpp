#ifndef EXX_GRAPHCONCEPTS_HPP
#define EXX_GRAPHCONCEPTS_HPP

#include <concepts>
#include <ranges>

namespace exx::incident {

template<typename G>
concept TraversableGraph = requires(G& g, const G& cg, typename G::VertexDescriptor v, typename G::ConstVertexDescriptor cv) {
    typename G::VertexValueType;
    typename G::VertexDescriptor;
    typename G::ConstVertexDescriptor;
    typename G::VertexIterator;
    typename G::ConstVertexIterator;

    { g.beginVertices() } -> std::forward_iterator;
    { g.endVertices() }   -> std::forward_iterator;

    { cg.beginVertices() } -> std::forward_iterator;
    { cg.endVertices() }   -> std::forward_iterator;

    { g.vertexCount() } -> std::integral;
    { cg.vertexCount() } -> std::integral;

    { v.adjacentVertices() } -> std::ranges::forward_range;
    requires std::convertible_to<
        std::ranges::range_reference_t<decltype(v.adjacentVertices())>,
        typename G::VertexDescriptor
        >;

    { cv.adjacentVertices() } -> std::ranges::forward_range;
    requires std::convertible_to<
        std::ranges::range_reference_t<decltype(cv.adjacentVertices())>,
        typename G::ConstVertexDescriptor
        >;
};

} // namespace exx::incident

#endif // EXX_GRAPHCONCEPTS_HPP
