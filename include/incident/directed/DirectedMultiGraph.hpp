#ifndef EXX_DIRECTEDMULTIGRAPH_HPP
#define EXX_DIRECTEDMULTIGRAPH_HPP

#include <optional>

#include "DirectedPseudoGraph.hpp"

namespace exx::incident {

template<typename VertexData, typename ArcData>
class DirectedMultiGraph {
public:

    using VertexValueType = VertexData;
    using ArcValueType    = ArcData;

    using VertexDescriptor      = typename DirectedPseudoGraph<VertexData, ArcData>::VertexDescriptor;
    using ConstVertexDescriptor = typename DirectedPseudoGraph<VertexData, ArcData>::ConstVertexDescriptor;
    using VertexDescriptorHash  = typename DirectedPseudoGraph<VertexData, ArcData>::VertexDescriptorHash;

    using ArcDescriptor         = typename DirectedPseudoGraph<VertexData, ArcData>::ArcDescriptor;
    using ConstArcDescriptor    = typename DirectedPseudoGraph<VertexData, ArcData>::ConstArcDescriptor;
    using ArcDescriptorHash     = typename DirectedPseudoGraph<VertexData, ArcData>::ArcDescriptorHash;

    using VertexIterator        = typename DirectedPseudoGraph<VertexData, ArcData>::VertexIterator;
    using ConstVertexIterator   = typename DirectedPseudoGraph<VertexData, ArcData>::ConstVertexIterator;
    using ArcIterator           = typename DirectedPseudoGraph<VertexData, ArcData>::ArcIterator;
    using ConstArcIterator      = typename DirectedPseudoGraph<VertexData, ArcData>::ConstArcIterator;

    DirectedMultiGraph() = default;

    explicit DirectedMultiGraph(const DirectedPseudoGraph<VertexData, ArcData>& graph)
        : _pseudoGraph(graph) {}

    DirectedMultiGraph(const DirectedMultiGraph&) = default;
    DirectedMultiGraph(DirectedMultiGraph&&) noexcept = default;

    DirectedMultiGraph& operator=(const DirectedMultiGraph&) = default;
    DirectedMultiGraph& operator=(DirectedMultiGraph&&) noexcept = default;

    void swap(DirectedMultiGraph& other) noexcept { _pseudoGraph.swap(other._pseudoGraph); }

    template<typename... Args>
    VertexDescriptor emplaceVertex(Args&&... args)
    { return _pseudoGraph.emplaceVertex(std::forward<Args>(args)...); }

    template<typename T = VertexData>
        requires (!std::is_void_v<T>)
    VertexDescriptor addVertex(T&& data)
    { return _pseudoGraph.addVertex(std::forward<T>(data)); }

    VertexDescriptor addVertex()
        requires std::is_void_v<VertexData>
    { return _pseudoGraph.addVertex(); }

    void removeVertex(VertexDescriptor v)
    { _pseudoGraph.removeVertex(v); }

    template<typename... Args>
    std::optional<ArcDescriptor> emplaceArc(VertexDescriptor from,
                                            VertexDescriptor to,
                                            Args&&... args)
    {
        if (from == to) return std::nullopt;
        return _pseudoGraph.emplaceArc(from, to, std::forward<Args>(args)...);
    }

    template<typename T = ArcData>
        requires (!std::is_void_v<ArcData>)
    std::optional<ArcDescriptor> addArc(VertexDescriptor from,
                                        VertexDescriptor to,
                                        T&& data)
    { return emplaceArc(from, to, std::forward<T>(data)); }

    std::optional<ArcDescriptor> addArc(VertexDescriptor from, VertexDescriptor to)
        requires (std::is_void_v<ArcData>)
    { return emplaceArc(from, to); }

    void removeArc(ArcDescriptor a) { _pseudoGraph.removeArc(a); }

    VertexIterator beginVertices()             { return _pseudoGraph.beginVertices(); }
    VertexIterator endVertices()               { return _pseudoGraph.endVertices(); }
    ConstVertexIterator beginVertices()  const { return _pseudoGraph.beginVertices(); }
    ConstVertexIterator endVertices()    const { return _pseudoGraph.endVertices(); }
    ConstVertexIterator cbeginVertices() const { return _pseudoGraph.cbeginVertices(); }
    ConstVertexIterator cendVertices()   const { return _pseudoGraph.cendVertices(); }

    auto vertices()            { return _pseudoGraph.vertices(); }
    auto vertices()      const { return _pseudoGraph.vertices(); }
    auto constVertices() const { return _pseudoGraph.constVertices(); }

    ArcIterator beginArcs()             { return _pseudoGraph.beginArcs(); }
    ArcIterator endArcs()               { return _pseudoGraph.endArcs(); }
    ConstArcIterator beginArcs()  const { return _pseudoGraph.beginArcs(); }
    ConstArcIterator endArcs()    const { return _pseudoGraph.endArcs(); }
    ConstArcIterator cbeginArcs() const { return _pseudoGraph.cbeginArcs(); }
    ConstArcIterator cendArcs()   const { return _pseudoGraph.cendArcs(); }

    auto arcs()            { return _pseudoGraph.arcs(); }
    auto arcs()      const { return _pseudoGraph.arcs(); }
    auto constArcs() const { return _pseudoGraph.constArcs(); }

    std::size_t vertexCount() const { return _pseudoGraph.vertexCount(); }
    std::size_t arcCount()   const { return _pseudoGraph.arcCount(); }

    std::optional<ArcDescriptor> findArc(VertexDescriptor from, VertexDescriptor to)
    { return _pseudoGraph.findArc(from, to); }
    std::optional<ConstArcDescriptor> findArc(ConstVertexDescriptor from, ConstVertexDescriptor to) const
    { return _pseudoGraph.findArc(from, to); }

    bool hasArc(ConstVertexDescriptor from, ConstVertexDescriptor to) const
    { return findArc(from, to).has_value(); }

    const DirectedPseudoGraph<VertexData, ArcData>& basePseudoGraph() const
    { return _pseudoGraph; }

    void clear() { _pseudoGraph.clear(); }

    bool empty() const { return _pseudoGraph.empty(); }

    bool rotateArc(ArcDescriptor a) { return _pseudoGraph.rotateArc(a); }

private:

    DirectedPseudoGraph<VertexData, ArcData> _pseudoGraph;

};

} // namespace exx::incident

#endif // EXX_DIRECTEDMULTIGRAPH_HPP
