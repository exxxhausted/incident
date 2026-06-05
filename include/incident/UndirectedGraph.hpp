#ifndef EXX_UNDIRECTEDGRAPH_HPP
#define EXX_UNDIRECTEDGRAPH_HPP

#include <optional>
#include "UndirectedMultiGraph.hpp"

namespace exx::incident {

template<typename VertexData,
         typename EdgeData,
         typename VertexHash = std::hash<VertexData>>
class UndirectedGraph {
public:

    using VertexDescriptor      = typename UndirectedMultiGraph<VertexData, EdgeData, VertexHash>::VertexDescriptor;
    using ConstVertexDescriptor = typename UndirectedMultiGraph<VertexData, EdgeData, VertexHash>::ConstVertexDescriptor;
    using VertexDescriptorHash  = typename UndirectedMultiGraph<VertexData, EdgeData>::VertexDescriptorHash;

    using EdgeDescriptor        = typename UndirectedMultiGraph<VertexData, EdgeData, VertexHash>::EdgeDescriptor;
    using ConstEdgeDescriptor   = typename UndirectedMultiGraph<VertexData, EdgeData, VertexHash>::ConstEdgeDescriptor;
    using EdgeDescriptorHash    = typename UndirectedMultiGraph<VertexData, EdgeData>::EdgeDescriptorHash;

    using VertexIterator        = typename UndirectedMultiGraph<VertexData, EdgeData, VertexHash>::VertexIterator;
    using ConstVertexIterator   = typename UndirectedMultiGraph<VertexData, EdgeData, VertexHash>::ConstVertexIterator;
    using EdgeIterator          = typename UndirectedMultiGraph<VertexData, EdgeData, VertexHash>::EdgeIterator;
    using ConstEdgeIterator     = typename UndirectedMultiGraph<VertexData, EdgeData, VertexHash>::ConstEdgeIterator;

    UndirectedGraph() = default;

    explicit UndirectedGraph(const UndirectedAbstractGraph<VertexData, EdgeData>& graph)
        : _multiGraph(graph) {}

    UndirectedGraph(const UndirectedGraph&) = default;
    UndirectedGraph(UndirectedGraph&&) noexcept = default;

    UndirectedGraph& operator=(const UndirectedGraph&) = default;
    UndirectedGraph& operator=(UndirectedGraph&&) noexcept = default;

    void swap(UndirectedGraph& other) noexcept { _multiGraph.swap(other._multiGraph); }

    template<typename... Args>
    std::optional<VertexDescriptor> emplaceVertex(Args&&... args)
    { return _multiGraph.emplaceVertex(std::forward<Args>(args)...); }

    std::optional<VertexDescriptor> addVertex(const VertexData& data)
    { return _multiGraph.addVertex(data); }

    std::optional<VertexDescriptor> addVertex(VertexData&& data)
    { return _multiGraph.addVertex(std::move(data)); }

    void removeVertex(VertexDescriptor v) { _multiGraph.removeVertex(v); }

    std::optional<VertexDescriptor> findVertex(const VertexData& data)
    { return _multiGraph.findVertex(data); }

    std::optional<ConstVertexDescriptor> findVertex(const VertexData& data) const
    { return _multiGraph.findVertex(data); }

    bool containsVertex(const VertexData& data) const
        requires (!std::is_void_v<VertexHash>)
    { return _multiGraph.containsVertex(data); }

    template<typename... Args>
        requires (!std::is_void_v<EdgeData>)
    std::optional<EdgeDescriptor> emplaceEdge(VertexDescriptor from,
                                              VertexDescriptor to,
                                              Args&&... args)
    {
        if (_multiGraph.hasEdge(from, to)) return std::nullopt;
        return _multiGraph.emplaceEdge(from, to, std::forward<Args>(args)...);
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
        if (_multiGraph.hasEdge(from, to)) return std::nullopt;
        return _multiGraph.addEdge(from, to);
    }

    template<typename... Args>
        requires (std::is_void_v<EdgeData>)
    std::optional<EdgeDescriptor> addEdge(const VertexData& fromData, const VertexData& toData) {
        auto fromOpt = findVertex(fromData);
        auto toOpt = findVertex(toData);
        if(!fromOpt || ! toOpt) return std::nullopt;
        return addEdge(*fromOpt, *toOpt);
    }

    void removeEdge(EdgeDescriptor e) { _multiGraph.removeEdge(e); }

    void removeEdge(const VertexData& fromData, const VertexData& toData)
    { _multiGraph.removeEdge(fromData, toData); }

    std::optional<EdgeDescriptor> findEdge(VertexDescriptor from, VertexDescriptor to)
    { return _multiGraph.findEdge(from, to); }
    std::optional<ConstEdgeDescriptor> findEdge(VertexDescriptor from, VertexDescriptor to) const
    { return _multiGraph.findEdge(from, to); }

    std::optional<EdgeDescriptor> findEdge(const VertexData& fromData, const VertexData& toData)
    { return _multiGraph.findEdge(fromData, toData); }
    std::optional<ConstEdgeDescriptor> findEdge(const VertexData& fromData, const VertexData& toData) const
    { return _multiGraph.findEdge(fromData, toData); }

    bool hasEdge(VertexDescriptor from, VertexDescriptor to) const
    { return _multiGraph.hasEdge(from, to); }

    bool hasEdge(const VertexData& fromData, const VertexData& toData) const
    { return _multiGraph.hasEdge(fromData, toData); }

    VertexIterator beginVertices()             { return _multiGraph.beginVertices(); }
    VertexIterator endVertices()               { return _multiGraph.endVertices(); }
    ConstVertexIterator beginVertices()  const { return _multiGraph.beginVertices(); }
    ConstVertexIterator endVertices()    const { return _multiGraph.endVertices(); }
    ConstVertexIterator cbeginVertices() const { return _multiGraph.cbeginVertices(); }
    ConstVertexIterator cendVertices()   const { return _multiGraph.cendVertices(); }

    auto vertices()            { return _multiGraph.vertices(); }
    auto vertices()      const { return _multiGraph.vertices(); }
    auto constVertices() const { return _multiGraph.constVertices(); }

    EdgeIterator beginEdges()             { return _multiGraph.beginEdges(); }
    EdgeIterator endEdges()               { return _multiGraph.endEdges(); }
    ConstEdgeIterator beginEdges()  const { return _multiGraph.beginEdges(); }
    ConstEdgeIterator endEdges()    const { return _multiGraph.endEdges(); }
    ConstEdgeIterator cbeginEdges() const { return _multiGraph.cbeginEdges(); }
    ConstEdgeIterator cendEdges()   const { return _multiGraph.cendEdges(); }

    auto edges()            { return _multiGraph.edges(); }
    auto edges()      const { return _multiGraph.edges(); }
    auto constEdges() const { return _multiGraph.constEdges(); }

    std::size_t vertexCount() const { return _multiGraph.vertexCount(); }
    std::size_t edgeCount()   const { return _multiGraph.edgeCount(); }

    UndirectedMultiGraph<VertexData, EdgeData, VertexHash>& baseMultiGraph()
    { return _multiGraph; }
    const UndirectedMultiGraph<VertexData, EdgeData, VertexHash>& baseMultiGraph() const
    { return _multiGraph; }

    UndirectedAbstractGraph<VertexData, EdgeData>& topology()
    { return _multiGraph.topology(); }
    const UndirectedAbstractGraph<VertexData, EdgeData>& topology() const
    { return _multiGraph.topology(); }

private:

    UndirectedMultiGraph<VertexData, EdgeData, VertexHash> _multiGraph;

};

} // namespace exx::incident

#endif // EXX_UNDIRECTEDGRAPH_HPP
