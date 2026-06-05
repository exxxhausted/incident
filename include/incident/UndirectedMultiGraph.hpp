#ifndef EXX_UNDIRECTEDMULTIGRAPH_HPP
#define EXX_UNDIRECTEDMULTIGRAPH_HPP

#include <optional>
#include "UndirectedPseudoGraph.hpp"

namespace exx::incident {

template<typename VertexData,
         typename EdgeData,
         typename VertexHash = std::hash<VertexData>>
class UndirectedMultiGraph {
public:

    using VertexDescriptor      = typename UndirectedPseudoGraph<VertexData, EdgeData, VertexHash>::VertexDescriptor;
    using ConstVertexDescriptor = typename UndirectedPseudoGraph<VertexData, EdgeData, VertexHash>::ConstVertexDescriptor;
    using VertexDescriptorHash  = typename UndirectedPseudoGraph<VertexData, EdgeData>::VertexDescriptorHash;

    using EdgeDescriptor        = typename UndirectedPseudoGraph<VertexData, EdgeData, VertexHash>::EdgeDescriptor;
    using ConstEdgeDescriptor   = typename UndirectedPseudoGraph<VertexData, EdgeData, VertexHash>::ConstEdgeDescriptor;
    using EdgeDescriptorHash    = typename UndirectedPseudoGraph<VertexData, EdgeData>::EdgeDescriptorHash;

    using VertexIterator        = typename UndirectedPseudoGraph<VertexData, EdgeData, VertexHash>::VertexIterator;
    using ConstVertexIterator   = typename UndirectedPseudoGraph<VertexData, EdgeData, VertexHash>::ConstVertexIterator;
    using EdgeIterator          = typename UndirectedPseudoGraph<VertexData, EdgeData, VertexHash>::EdgeIterator;
    using ConstEdgeIterator     = typename UndirectedPseudoGraph<VertexData, EdgeData, VertexHash>::ConstEdgeIterator;

    UndirectedMultiGraph() = default;

    explicit UndirectedMultiGraph(const UndirectedAbstractGraph<VertexData, EdgeData>& graph)
        : _pseudoGraph(graph) {}

    UndirectedMultiGraph(const UndirectedMultiGraph&) = default;
    UndirectedMultiGraph(UndirectedMultiGraph&&) noexcept = default;

    UndirectedMultiGraph& operator=(const UndirectedMultiGraph&) = default;
    UndirectedMultiGraph& operator=(UndirectedMultiGraph&&) noexcept = default;

    void swap(UndirectedMultiGraph& other) noexcept { _pseudoGraph.swap(other._pseudoGraph); }

    template<typename... Args>
    std::optional<VertexDescriptor> emplaceVertex(Args&&... args)
    { return _pseudoGraph.emplaceVertex(std::forward<Args>(args)...); }

    std::optional<VertexDescriptor> addVertex(const VertexData& data)
    { return _pseudoGraph.addVertex(data); }

    std::optional<VertexDescriptor> addVertex(VertexData&& data)
    { return _pseudoGraph.addVertex(std::move(data)); }

    void removeVertex(VertexDescriptor v)
    { _pseudoGraph.removeVertex(v); }

    std::optional<VertexDescriptor> findVertex(const VertexData& data)
    { return _pseudoGraph.findVertex(data); }

    std::optional<ConstVertexDescriptor> findVertex(const VertexData& data) const
    { return _pseudoGraph.findVertex(data); }

    bool containsVertex(const VertexData& data) const
        requires (!std::is_void_v<VertexHash>)
    { return _pseudoGraph.containsVertex(data); }

    template<typename... Args>
        requires (!std::is_void_v<EdgeData>)
    std::optional<EdgeDescriptor> emplaceEdge(VertexDescriptor from,
                                              VertexDescriptor to,
                                              Args&&... args)
    {
        if (from == to) return std::nullopt;
        return _pseudoGraph.emplaceEdge(from, to, std::forward<Args>(args)...);
    }

    template<typename T = EdgeData, typename... Args>
        requires (!std::is_void_v<EdgeData>)
    std::optional<EdgeDescriptor> addEdge(VertexDescriptor from,
                                          VertexDescriptor to,
                                          T&& data)
    { return emplaceEdge(from, to, std::forward<T>(data)); }

    template<typename T, typename... Args>
        requires (!std::is_void_v<EdgeData>)
    std::optional<EdgeDescriptor> addEdge(const VertexData& fromData,
                                          const VertexData& toData,
                                          T&& data)
    {
        auto fromOpt = findVertex(fromData);
        auto toOpt = findVertex(toData);
        if(!fromOpt || ! toOpt) return std::nullopt;
        return addEdge(*fromOpt, *toOpt, std::forward<T>(data));
    }

    template<typename... Args>
        requires (std::is_void_v<EdgeData>)
    std::optional<EdgeDescriptor> addEdge(VertexDescriptor from, VertexDescriptor to) {
        if (from == to) return std::nullopt;
        return _pseudoGraph.addEdge(from, to);
    }

    template<typename... Args>
        requires (std::is_void_v<EdgeData>)
    EdgeDescriptor addEdge(const VertexData& fromData, const VertexData& toData) {
        auto fromOpt = findVertex(fromData);
        auto toOpt = findVertex(toData);
        if(!fromOpt || ! toOpt) return;
        return addEdge(*fromOpt, *toOpt);
    }

    void removeEdge(EdgeDescriptor e) { _pseudoGraph.removeEdge(e); }

    void removeEdge(const VertexData& fromData, const VertexData& toData)
    { _pseudoGraph.removeEdge(fromData, toData); }

    VertexIterator beginVertices()             { return _pseudoGraph.beginVertices(); }
    VertexIterator endVertices()               { return _pseudoGraph.endVertices(); }
    ConstVertexIterator beginVertices()  const { return _pseudoGraph.beginVertices(); }
    ConstVertexIterator endVertices()    const { return _pseudoGraph.endVertices(); }
    ConstVertexIterator cbeginVertices() const { return _pseudoGraph.cbeginVertices(); }
    ConstVertexIterator cendVertices()   const { return _pseudoGraph.cendVertices(); }

    auto vertices()            { return _pseudoGraph.vertices(); }
    auto vertices()      const { return _pseudoGraph.vertices(); }
    auto constVertices() const { return _pseudoGraph.constVertices(); }

    EdgeIterator beginEdges()             { return _pseudoGraph.beginEdges(); }
    EdgeIterator endEdges()               { return _pseudoGraph.endEdges(); }
    ConstEdgeIterator beginEdges()  const { return _pseudoGraph.beginEdges(); }
    ConstEdgeIterator endEdges()    const { return _pseudoGraph.endEdges(); }
    ConstEdgeIterator cbeginEdges() const { return _pseudoGraph.cbeginEdges(); }
    ConstEdgeIterator cendEdges()   const { return _pseudoGraph.cendEdges(); }

    auto edges()            { return _pseudoGraph.edges(); }
    auto edges()      const { return _pseudoGraph.edges(); }
    auto constEdges() const { return _pseudoGraph.constEdges(); }

    std::size_t vertexCount() const { return _pseudoGraph.vertexCount(); }
    std::size_t edgeCount()   const { return _pseudoGraph.edgeCount(); }

    std::optional<EdgeDescriptor> findEdge(VertexDescriptor from, VertexDescriptor to)
    { return _pseudoGraph.findEdge(from, to); }
    std::optional<ConstEdgeDescriptor> findEdge(VertexDescriptor from, VertexDescriptor to) const
    { return _pseudoGraph.findEdge(from, to); }

    std::optional<EdgeDescriptor> findEdge(const VertexData& fromData, const VertexData& toData)
    { return _pseudoGraph.findEdge(fromData, toData); }
    std::optional<ConstEdgeDescriptor> findEdge(const VertexData& fromData, const VertexData& toData) const
    { return _pseudoGraph.findEdge(fromData, toData); }

    bool hasEdge(VertexDescriptor from, VertexDescriptor to) const
    { return findEdge(from, to).has_value(); }

    bool hasEdge(const VertexData& fromData, const VertexData& toData) const
    { return _pseudoGraph.hasEdge(fromData, toData); }

    UndirectedPseudoGraph<VertexData, EdgeData, VertexHash>& basePseudoGraph()
    { return _pseudoGraph; }
    const UndirectedPseudoGraph<VertexData, EdgeData, VertexHash>& basePseudoGraph() const
    { return _pseudoGraph; }

    UndirectedAbstractGraph<VertexData, EdgeData>& topology()
    { return _pseudoGraph.topology(); }
    const UndirectedAbstractGraph<VertexData, EdgeData>& topology() const
    { return _pseudoGraph.topology(); }

private:

    UndirectedPseudoGraph<VertexData, EdgeData, VertexHash> _pseudoGraph;

};

} // namespace exx::incident

#endif // EXX_UNDIRECTEDMULTIGRAPH_HPP
