#ifndef EXX_DIRECTEDGRAPH_HPP
#define EXX_DIRECTEDGRAPH_HPP

#include <optional>

#include "DirectedMultiGraph.hpp"

namespace exx::incident {

template<typename VertexData, typename ArcData>
class DirectedGraph {
public:

    using VertexValueType = VertexData;
    using ArcValueType    = ArcData;

    using VertexDescriptor      = typename DirectedMultiGraph<VertexData, ArcData>::VertexDescriptor;
    using ConstVertexDescriptor = typename DirectedMultiGraph<VertexData, ArcData>::ConstVertexDescriptor;
    using VertexDescriptorHash  = typename DirectedMultiGraph<VertexData, ArcData>::VertexDescriptorHash;

    using ArcDescriptor         = typename DirectedMultiGraph<VertexData, ArcData>::ArcDescriptor;
    using ConstArcDescriptor    = typename DirectedMultiGraph<VertexData, ArcData>::ConstArcDescriptor;
    using ArcDescriptorHash     = typename DirectedMultiGraph<VertexData, ArcData>::ArcDescriptorHash;

    using VertexIterator        = typename DirectedMultiGraph<VertexData, ArcData>::VertexIterator;
    using ConstVertexIterator   = typename DirectedMultiGraph<VertexData, ArcData>::ConstVertexIterator;
    using ArcIterator           = typename DirectedMultiGraph<VertexData, ArcData>::ArcIterator;
    using ConstArcIterator      = typename DirectedMultiGraph<VertexData, ArcData>::ConstArcIterator;

    DirectedGraph() = default;

    explicit DirectedGraph(const DirectedPseudoGraph<VertexData, ArcData>& graph)
        : _multiGraph(graph) {}

    DirectedGraph(const DirectedGraph&) = default;
    DirectedGraph(DirectedGraph&&) noexcept = default;

    DirectedGraph& operator=(const DirectedGraph&) = default;
    DirectedGraph& operator=(DirectedGraph&&) noexcept = default;

    void swap(DirectedGraph& other) noexcept { _multiGraph.swap(other._multiGraph); }

    template<typename... Args>
    VertexDescriptor emplaceVertex(Args&&... args)
    { return _multiGraph.emplaceVertex(std::forward<Args>(args)...); }

    template<typename T = VertexData>
        requires (!std::is_void_v<T>)
    VertexDescriptor addVertex(T&& data)
    { return _multiGraph.addVertex(std::forward<T>(data)); }

    VertexDescriptor addVertex() requires (std::is_void_v<VertexData>)
    { return _multiGraph.addVertex(); }

    void removeVertex(VertexDescriptor v) { _multiGraph.removeVertex(v); }

    template<typename... Args>
    std::optional<ArcDescriptor> emplaceArc(VertexDescriptor from,
                                            VertexDescriptor to,
                                            Args&&... args)
    {
        if (from == to) return std::nullopt;
        if (_multiGraph.hasArc(from, to)) return std::nullopt;
        return _multiGraph.emplaceArc(from, to, std::forward<Args>(args)...);
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

    void removeArc(ArcDescriptor a) { _multiGraph.removeArc(a); }

    std::optional<ArcDescriptor> findArc(VertexDescriptor from, VertexDescriptor to)
    { return _multiGraph.findArc(from, to); }
    std::optional<ConstArcDescriptor> findArc(ConstVertexDescriptor from, ConstVertexDescriptor to) const
    { return _multiGraph.findArc(from, to); }

    bool hasArc(ConstVertexDescriptor from, ConstVertexDescriptor to) const
    { return _multiGraph.hasArc(from, to); }

    VertexIterator beginVertices()             { return _multiGraph.beginVertices(); }
    VertexIterator endVertices()               { return _multiGraph.endVertices(); }
    ConstVertexIterator beginVertices()  const { return _multiGraph.beginVertices(); }
    ConstVertexIterator endVertices()    const { return _multiGraph.endVertices(); }
    ConstVertexIterator cbeginVertices() const { return _multiGraph.cbeginVertices(); }
    ConstVertexIterator cendVertices()   const { return _multiGraph.cendVertices(); }

    auto vertices()            { return _multiGraph.vertices(); }
    auto vertices()      const { return _multiGraph.vertices(); }
    auto constVertices() const { return _multiGraph.constVertices(); }

    ArcIterator beginArcs()             { return _multiGraph.beginArcs(); }
    ArcIterator endArcs()               { return _multiGraph.endArcs(); }
    ConstArcIterator beginArcs()  const { return _multiGraph.beginArcs(); }
    ConstArcIterator endArcs()    const { return _multiGraph.endArcs(); }
    ConstArcIterator cbeginArcs() const { return _multiGraph.cbeginArcs(); }
    ConstArcIterator cendArcs()   const { return _multiGraph.cendArcs(); }

    auto arcs()            { return _multiGraph.arcs(); }
    auto arcs()      const { return _multiGraph.arcs(); }
    auto constArcs() const { return _multiGraph.constArcs(); }

    std::size_t vertexCount() const { return _multiGraph.vertexCount(); }
    std::size_t arcCount()   const { return _multiGraph.arcCount(); }

    const DirectedMultiGraph<VertexData, ArcData>& baseMultiGraph() const
    { return _multiGraph; }

    void clear() { _multiGraph.clear(); }

    bool empty() const { return _multiGraph.empty(); }

    [[maybe_unused]]
    bool rotateArc(ArcDescriptor a) { return _multiGraph.rotateArc(a); }

private:

    DirectedMultiGraph<VertexData, ArcData> _multiGraph;

};

} // namespace exx::incident

#endif // EXX_DIRECTEDGRAPH_HPP
