#ifndef EXX_UNDIRECTEDMULTIGRAPH_HPP
#define EXX_UNDIRECTEDMULTIGRAPH_HPP

#include <optional>
#include "UndirectedPseudoGraph.hpp"

namespace exx::incident {

template<typename VertexData,
         typename EdgeData,
         typename VertexHash = std::hash<VertexData>,
         typename EdgeHash = std::hash<EdgeData>>
class UndirectedMultiGraph {
public:
    // --- Типы, совпадающие с типами псевдографа ---
    using VertexDescriptor      = typename UndirectedPseudoGraph<VertexData, EdgeData, VertexHash, EdgeHash>::VertexDescriptor;
    using ConstVertexDescriptor = typename UndirectedPseudoGraph<VertexData, EdgeData, VertexHash, EdgeHash>::ConstVertexDescriptor;
    using EdgeDescriptor        = typename UndirectedPseudoGraph<VertexData, EdgeData, VertexHash, EdgeHash>::EdgeDescriptor;
    using ConstEdgeDescriptor   = typename UndirectedPseudoGraph<VertexData, EdgeData, VertexHash, EdgeHash>::ConstEdgeDescriptor;

    using VertexIterator        = typename UndirectedPseudoGraph<VertexData, EdgeData, VertexHash, EdgeHash>::VertexIterator;
    using ConstVertexIterator   = typename UndirectedPseudoGraph<VertexData, EdgeData, VertexHash, EdgeHash>::ConstVertexIterator;
    using EdgeIterator          = typename UndirectedPseudoGraph<VertexData, EdgeData, VertexHash, EdgeHash>::EdgeIterator;
    using ConstEdgeIterator     = typename UndirectedPseudoGraph<VertexData, EdgeData, VertexHash, EdgeHash>::ConstEdgeIterator;

    // --- Конструкторы ---
    UndirectedMultiGraph() = default;

    // Явное преобразование из произвольного абстрактного графа
    explicit UndirectedMultiGraph(const UndirectedAbstractGraph<VertexData, EdgeData>& graph)
        : _pseudo(graph) {}

    // Копирование и перемещение
    UndirectedMultiGraph(const UndirectedMultiGraph&) = default;
    UndirectedMultiGraph(UndirectedMultiGraph&&) noexcept = default;

    // Присваивание
    UndirectedMultiGraph& operator=(const UndirectedMultiGraph&) = default;
    UndirectedMultiGraph& operator=(UndirectedMultiGraph&&) noexcept = default;

    // Обмен содержимым
    void swap(UndirectedMultiGraph& other) noexcept {
        _pseudo.swap(other._pseudo);
    }

    // --- Вершины (полностью делегируются) ---
    template<typename... Args>
    std::optional<VertexDescriptor> emplaceVertex(Args&&... args) {
        return _pseudo.emplaceVertex(std::forward<Args>(args)...);
    }

    std::optional<VertexDescriptor> addVertex(const VertexData& data) {
        return _pseudo.addVertex(data);
    }

    std::optional<VertexDescriptor> addVertex(VertexData&& data) {
        return _pseudo.addVertex(std::move(data));
    }

    void removeVertex(VertexDescriptor v) {
        _pseudo.removeVertex(v);
    }

    std::optional<VertexDescriptor> findVertex(const VertexData& data) const {
        return _pseudo.findVertex(data);
    }

    bool containsVertex(const VertexData& data) const
        requires (!std::is_void_v<VertexHash>)
    {
        return _pseudo.containsVertex(data);
    }

    // --- Рёбра с запретом петель ---
    template<typename... Args>
        requires (!std::is_void_v<EdgeData>)
    std::optional<EdgeDescriptor> emplaceEdge(VertexDescriptor from, VertexDescriptor to, Args&&... args) {
        if (from == to) return std::nullopt;
        return _pseudo.emplaceEdge(from, to, std::forward<Args>(args)...);
    }

    template<typename T = EdgeData, typename... Args>
        requires (!std::is_void_v<EdgeData>)
    std::optional<EdgeDescriptor> addEdge(VertexDescriptor from, VertexDescriptor to, T&& data) {
        return emplaceEdge(from, to, std::forward<T>(data));
    }

    template<typename... Args>
        requires (std::is_void_v<EdgeData>)
    EdgeDescriptor addEdge(VertexDescriptor from, VertexDescriptor to) {
        if (from == to) return std::nullopt;
        return _pseudo.addEdge(from, to);
    }

    void removeEdge(EdgeDescriptor e) {
        _pseudo.removeEdge(e);
    }

    // Поиск рёбер по данным (делегируется)
    template <typename T = EdgeData>
        requires (!std::is_void_v<EdgeData>)
    auto findEdge(const T& data) const {
        return _pseudo.findEdge(data);
    }

    template <typename T = EdgeData>
        requires (!std::is_void_v<EdgeData> && !std::is_void_v<EdgeHash>)
    bool containsEdge(const T& data) const {
        return _pseudo.containsEdge(data);
    }

    template <typename T = EdgeData>
        requires (!std::is_void_v<EdgeData> && std::is_void_v<EdgeHash>)
    bool containsEdge(const T& data) const {
        return _pseudo.containsEdge(data);
    }

    // --- Итераторы и обход (делегируются) ---
    VertexIterator beginVertices()             { return _pseudo.beginVertices(); }
    VertexIterator endVertices()               { return _pseudo.endVertices(); }
    ConstVertexIterator beginVertices()  const { return _pseudo.beginVertices(); }
    ConstVertexIterator endVertices()    const { return _pseudo.endVertices(); }
    ConstVertexIterator cbeginVertices() const { return _pseudo.cbeginVertices(); }
    ConstVertexIterator cendVertices()   const { return _pseudo.cendVertices(); }

    auto vertices()            { return _pseudo.vertices(); }
    auto vertices()      const { return _pseudo.vertices(); }
    auto constVertices() const { return _pseudo.constVertices(); }

    EdgeIterator beginEdges()             { return _pseudo.beginEdges(); }
    EdgeIterator endEdges()               { return _pseudo.endEdges(); }
    ConstEdgeIterator beginEdges()  const { return _pseudo.beginEdges(); }
    ConstEdgeIterator endEdges()    const { return _pseudo.endEdges(); }
    ConstEdgeIterator cbeginEdges() const { return _pseudo.cbeginEdges(); }
    ConstEdgeIterator cendEdges()   const { return _pseudo.cendEdges(); }

    auto edges()            { return _pseudo.edges(); }
    auto edges()      const { return _pseudo.edges(); }
    auto constEdges() const { return _pseudo.constEdges(); }

    // --- Количество элементов ---
    std::size_t vertexCount() const { return _pseudo.vertexCount(); }
    std::size_t edgeCount()   const { return _pseudo.edgeCount(); }


    std::optional<EdgeDescriptor> findEdge(VertexDescriptor from, VertexDescriptor to) const
    { return _pseudo.findEdge(from, to); }

    bool hasEdge(VertexDescriptor from, VertexDescriptor to) const { return findEdge(from, to).has_value(); }

    // --- Доступ к внутреннему псевдографу ---
    const UndirectedPseudoGraph<VertexData, EdgeData, VertexHash, EdgeHash>& basePseudoGraph() const {
        return _pseudo;
    }

private:
    UndirectedPseudoGraph<VertexData, EdgeData, VertexHash, EdgeHash> _pseudo;
};

} // namespace exx::incident

#endif // EXX_UNDIRECTEDMULTIGRAPH_HPP
