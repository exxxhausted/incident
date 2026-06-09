#ifndef EXX_GRAPHCONCEPTS_HPP
#define EXX_GRAPHCONCEPTS_HPP

#include <concepts>
#include <ranges>

namespace exx::incident {

template<typename G>
concept GraphConcept = requires(G& g, const G& cg, typename G::VertexDescriptor v) {
    typename G::VertexValueType;
    typename G::VertexDescriptor;
    typename G::ConstVertexDescriptor;

    requires std::copyable<typename G::VertexDescriptor>;
    requires std::equality_comparable<typename G::VertexDescriptor>;
    requires std::convertible_to<typename G::VertexDescriptor, typename G::ConstVertexDescriptor>;

    { v.data() } -> std::convertible_to<const typename G::VertexValueType&>;

    { g.vertices() } -> std::ranges::range;
    { cg.vertices() } -> std::ranges::range;

    requires std::convertible_to<
        std::ranges::range_reference_t<decltype(g.vertices())>,
        typename G::VertexDescriptor
        >;
    requires std::convertible_to<
        std::ranges::range_reference_t<decltype(cg.vertices())>,
        typename G::ConstVertexDescriptor
        >;

    typename G::VertexIterator;
    typename G::ConstVertexIterator;

    { g.beginVertices() } -> std::forward_iterator;
    { g.endVertices() }   -> std::forward_iterator;
    { cg.beginVertices() } -> std::forward_iterator;
    { cg.endVertices() }   -> std::forward_iterator;

    { g.vertexCount() } -> std::integral;
    { cg.vertexCount() } -> std::integral;
};

template<typename G>
concept UndirectedGraphConcept = GraphConcept<G> &&
                                 requires(G& g, const G& cg, typename G::EdgeDescriptor e)
{
    typename G::EdgeValueType;
    typename G::EdgeDescriptor;
    typename G::ConstEdgeDescriptor;

    requires std::copyable<typename G::EdgeDescriptor>;
    requires std::equality_comparable<typename G::EdgeDescriptor>;
    requires std::convertible_to<typename G::EdgeDescriptor, typename G::ConstEdgeDescriptor>;

    { e.data() } -> std::convertible_to<const typename G::EdgeValueType&>;
    { e.u() } -> std::convertible_to<typename G::ConstVertexDescriptor>;
    { e.v() } -> std::convertible_to<typename G::ConstVertexDescriptor>;

    { g.edges() } -> std::ranges::range;
    { cg.edges() } -> std::ranges::range;

    requires std::convertible_to<
        std::ranges::range_reference_t<decltype(g.edges())>,
        typename G::EdgeDescriptor
        >;
    requires std::convertible_to<
        std::ranges::range_reference_t<decltype(cg.edges())>,
        typename G::ConstEdgeDescriptor
        >;

    typename G::EdgeIterator;
    typename G::ConstEdgeIterator;

    { g.beginEdges() } -> std::forward_iterator;
    { g.endEdges() }   -> std::forward_iterator;
    { cg.beginEdges() } -> std::forward_iterator;
    { cg.endEdges() }   -> std::forward_iterator;

    { g.edgeCount() } -> std::integral;
    { cg.edgeCount() } -> std::integral;
};

template<typename G>
concept DirectedGraphConcept = GraphConcept<G> &&
                               requires(G& g, const G& cg, typename G::ArcDescriptor a)
{
    typename G::ArcValueType;
    typename G::ArcDescriptor;
    typename G::ConstArcDescriptor;

    requires std::copyable<typename G::ArcDescriptor>;
    requires std::equality_comparable<typename G::ArcDescriptor>;
    requires std::convertible_to<typename G::ArcDescriptor, typename G::ConstArcDescriptor>;

    { a.data() } -> std::convertible_to<const typename G::ArcValueType&>;
    { a.from() } -> std::convertible_to<typename G::ConstVertexDescriptor>;
    { a.to() }   -> std::convertible_to<typename G::ConstVertexDescriptor>;

    { g.arcs() } -> std::ranges::range;
    { cg.arcs() } -> std::ranges::range;

    requires std::convertible_to<
        std::ranges::range_reference_t<decltype(g.arcs())>,
        typename G::ArcDescriptor
        >;
    requires std::convertible_to<
        std::ranges::range_reference_t<decltype(cg.arcs())>,
        typename G::ConstArcDescriptor
        >;

    typename G::ArcIterator;
    typename G::ConstArcIterator;

    { g.beginArcs() } -> std::forward_iterator;
    { g.endArcs() }   -> std::forward_iterator;
    { cg.beginArcs() } -> std::forward_iterator;
    { cg.endArcs() }   -> std::forward_iterator;

    { g.arcCount() } -> std::integral;
    { cg.arcCount() } -> std::integral;
};

} // namespace exx::incident

#endif // EXX_GRAPHCONCEPTS_HPP
