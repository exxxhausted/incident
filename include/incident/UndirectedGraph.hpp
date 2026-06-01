#ifndef EXX_UNDIRECTEDGRAPH_HPP
#define EXX_UNDIRECTEDGRAPH_HPP

#include <optional>
#include "UndirectedMultiGraph.hpp"

namespace exx::incident {

template<typename VertexData,
         typename EdgeData,
         typename VertexHash = std::hash<VertexData>,
         typename EdgeHash = std::hash<EdgeData>>
class UndirectedGraph {
public:
    // --- Типы, совпадающие с типами мультиграфа ---
    using VertexDescriptor      = typename UndirectedMultiGraph<VertexData, EdgeData, VertexHash, EdgeHash>::VertexDescriptor;
    using ConstVertexDescriptor = typename UndirectedMultiGraph<VertexData, EdgeData, VertexHash, EdgeHash>::ConstVertexDescriptor;
    using EdgeDescriptor        = typename UndirectedMultiGraph<VertexData, EdgeData, VertexHash, EdgeHash>::EdgeDescriptor;
    using ConstEdgeDescriptor   = typename UndirectedMultiGraph<VertexData, EdgeData, VertexHash, EdgeHash>::ConstEdgeDescriptor;

    using VertexIterator        = typename UndirectedMultiGraph<VertexData, EdgeData, VertexHash, EdgeHash>::VertexIterator;
    using ConstVertexIterator   = typename UndirectedMultiGraph<VertexData, EdgeData, VertexHash, EdgeHash>::ConstVertexIterator;
    using EdgeIterator          = typename UndirectedMultiGraph<VertexData, EdgeData, VertexHash, EdgeHash>::EdgeIterator;
    using ConstEdgeIterator     = typename UndirectedMultiGraph<VertexData, EdgeData, VertexHash, EdgeHash>::ConstEdgeIterator;

    // --- Конструкторы ---
    UndirectedGraph() = default;

    explicit UndirectedGraph(const UndirectedAbstractGraph<VertexData, EdgeData>& graph)
        : _graph(graph) {}

    UndirectedGraph(const UndirectedGraph&) = default;
    UndirectedGraph(UndirectedGraph&&) noexcept = default;
    UndirectedGraph& operator=(const UndirectedGraph&) = default;
    UndirectedGraph& operator=(UndirectedGraph&&) noexcept = default;

    void swap(UndirectedGraph& other) noexcept {
        _graph.swap(other._graph);
    }

    // --- Вершины (полностью делегируются) ---
    template<typename... Args>
    std::optional<VertexDescriptor> emplaceVertex(Args&&... args) {
        return _graph.emplaceVertex(std::forward<Args>(args)...);
    }

    std::optional<VertexDescriptor> addVertex(const VertexData& data) {
        return _graph.addVertex(data);
    }

    std::optional<VertexDescriptor> addVertex(VertexData&& data) {
        return _graph.addVertex(std::move(data));
    }

    void removeVertex(VertexDescriptor v) {
        _graph.removeVertex(v);
    }

    std::optional<VertexDescriptor> findVertex(const VertexData& data) const {
        return _graph.findVertex(data);
    }

    bool containsVertex(const VertexData& data) const
        requires (!std::is_void_v<VertexHash>)
    {
        return _graph.containsVertex(data);
    }

    // --- Рёбра с запретом кратных ---
    template<typename... Args>
        requires (!std::is_void_v<EdgeData>)
    std::optional<EdgeDescriptor> emplaceEdge(VertexDescriptor from, VertexDescriptor to, Args&&... args) {
        if (_graph.hasEdge(from, to)) return std::nullopt;
        return _graph.emplaceEdge(from, to, std::forward<Args>(args)...);
    }

    template<typename T = EdgeData, typename... Args>
        requires (!std::is_void_v<EdgeData>)
    std::optional<EdgeDescriptor> addEdge(VertexDescriptor from, VertexDescriptor to, T&& data) {
        return emplaceEdge(from, to, std::forward<T>(data));
    }

    template<typename... Args>
        requires (std::is_void_v<EdgeData>)
    std::optional<EdgeDescriptor> addEdge(VertexDescriptor from, VertexDescriptor to) {
        if (from == to) return std::nullopt;
        if (_graph.hasEdge(from, to)) return std::nullopt;
        return _graph.addEdge(from, to);
    }

    void removeEdge(EdgeDescriptor e) {
        _graph.removeEdge(e);
    }

    // --- Проверка существования ребра ---
    bool hasEdge(VertexDescriptor from, VertexDescriptor to) const {
        return _graph.hasEdge(from, to);
    }

    // Поиск рёбер по данным (делегируется)
    template <typename T = EdgeData>
        requires (!std::is_void_v<EdgeData>)
    auto findEdge(const T& data) const {
        return _graph.findEdge(data);
    }

    template <typename T = EdgeData>
        requires (!std::is_void_v<EdgeData> && !std::is_void_v<EdgeHash>)
    bool containsEdge(const T& data) const {
        return _graph.containsEdge(data);
    }

    template <typename T = EdgeData>
        requires (!std::is_void_v<EdgeData> && std::is_void_v<EdgeHash>)
    bool containsEdge(const T& data) const {
        return _graph.containsEdge(data);
    }

    // --- Итераторы и обход (делегируются) ---
    VertexIterator beginVertices()             { return _graph.beginVertices(); }
    VertexIterator endVertices()               { return _graph.endVertices(); }
    ConstVertexIterator beginVertices()  const { return _graph.beginVertices(); }
    ConstVertexIterator endVertices()    const { return _graph.endVertices(); }
    ConstVertexIterator cbeginVertices() const { return _graph.cbeginVertices(); }
    ConstVertexIterator cendVertices()   const { return _graph.cendVertices(); }

    auto vertices()            { return _graph.vertices(); }
    auto vertices()      const { return _graph.vertices(); }
    auto constVertices() const { return _graph.constVertices(); }

    EdgeIterator beginEdges()             { return _graph.beginEdges(); }
    EdgeIterator endEdges()               { return _graph.endEdges(); }
    ConstEdgeIterator beginEdges()  const { return _graph.beginEdges(); }
    ConstEdgeIterator endEdges()    const { return _graph.endEdges(); }
    ConstEdgeIterator cbeginEdges() const { return _graph.cbeginEdges(); }
    ConstEdgeIterator cendEdges()   const { return _graph.cendEdges(); }

    auto edges()            { return _graph.edges(); }
    auto edges()      const { return _graph.edges(); }
    auto constEdges() const { return _graph.constEdges(); }

    // --- Количество элементов ---
    std::size_t vertexCount() const { return _graph.vertexCount(); }
    std::size_t edgeCount()   const { return _graph.edgeCount(); }

    // --- Доступ к внутреннему мультиграфу (только чтение) ---
    const UndirectedMultiGraph<VertexData, EdgeData, VertexHash, EdgeHash>& baseMultiGraph() const {
        return _graph;
    }

private:
    UndirectedMultiGraph<VertexData, EdgeData, VertexHash, EdgeHash> _graph;
};

} // namespace exx::incident

#endif // EXX_UNDIRECTEDGRAPH_HPP
