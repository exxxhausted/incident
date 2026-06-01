#ifndef EXX_UNDIRECTEDPSEUDOGRAPH_HPP
#define EXX_UNDIRECTEDPSEUDOGRAPH_HPP

#include <unordered_map>
#include <optional>
#include <algorithm>

#include "UndirectedAbstractGraph.hpp"

namespace exx::incident {

template<typename VertexData,
         typename EdgeData,
         typename VertexHash = std::hash<VertexData>,
         typename EdgeHash = std::hash<EdgeData>>
class UndirectedPseudoGraph {
public:
    // --- Юзинги ---
    using VertexDescriptor      = typename UndirectedAbstractGraph<VertexData, EdgeData>::VertexDescriptor;
    using ConstVertexDescriptor = typename UndirectedAbstractGraph<VertexData, EdgeData>::ConstVertexDescriptor;
    using EdgeDescriptor        = typename UndirectedAbstractGraph<VertexData, EdgeData>::EdgeDescriptor;
    using ConstEdgeDescriptor   = typename UndirectedAbstractGraph<VertexData, EdgeData>::ConstEdgeDescriptor;

    using VertexIterator        = typename UndirectedAbstractGraph<VertexData, EdgeData>::VertexIterator;
    using ConstVertexIterator   = typename UndirectedAbstractGraph<VertexData, EdgeData>::ConstVertexIterator;
    using EdgeIterator          = typename UndirectedAbstractGraph<VertexData, EdgeData>::EdgeIterator;
    using ConstEdgeIterator     = typename UndirectedAbstractGraph<VertexData, EdgeData>::ConstEdgeIterator;

    // --- Конструкторы ---
    UndirectedPseudoGraph() = default;

    explicit UndirectedPseudoGraph(const UndirectedAbstractGraph<VertexData, EdgeData>& graph)
        : _graph(graph)
    { _rebuildIndexes(); }

    // Копирование (глубокое)
    UndirectedPseudoGraph(const UndirectedPseudoGraph& other)
        : _graph(other._graph)
    { _rebuildIndexes(); }

    // Перемещение (простое)
    UndirectedPseudoGraph(UndirectedPseudoGraph&& other) noexcept
        : _graph(std::move(other._graph)),
        _vht(std::move(other._vht)),
        _eht(std::move(other._eht)) {}

    // Копирующее присваивание (через copy-and-swap)
    UndirectedPseudoGraph& operator=(const UndirectedPseudoGraph& other) {
        if (this != &other) {
            UndirectedPseudoGraph tmp(other);
            swap(tmp);
        }
        return *this;
    }

    // Перемещающее присваивание
    UndirectedPseudoGraph& operator=(UndirectedPseudoGraph&& other) noexcept {
        if (this != &other) {
            _graph = std::move(other._graph);
            _vht   = std::move(other._vht);
            _eht   = std::move(other._eht);
        }
        return *this;
    }

    // Обмен
    void swap(UndirectedPseudoGraph& other) noexcept {
        using std::swap;
        swap(_graph, other._graph);
        swap(_vht,   other._vht);
        swap(_eht,   other._eht);
    }

    // --- Вершины (уникальность) ---
    template<typename... Args>
    std::optional<VertexDescriptor> emplaceVertex(Args&&... args) {
        VertexData data(std::forward<Args>(args)...);
        if constexpr (!std::is_void_v<VertexHash>) {
            if (_vht.contains(data)) return std::nullopt;
            VertexDescriptor v = _graph.addVertex(std::move(data));
            _vht.emplace(v.data(), v);
            return v;
        } else {
            for(auto vert : _graph.vertices())
                if(vert.data() == data) return std::nullopt;
            return _graph.emplaceVertex(std::move(data));
        }
    }

    std::optional<VertexDescriptor> addVertex(const VertexData& data)
    { return emplaceVertex(data); }
    std::optional<VertexDescriptor> addVertex(VertexData&& data)
    { return emplaceVertex(std::move(data)); }

    void removeVertex(VertexDescriptor v) {
        if constexpr (!std::is_void_v<VertexHash>)
            _vht.erase(v.data());

        _graph.removeVertex(v);
    }

    std::optional<VertexDescriptor> findVertex(const VertexData& data) const {
        if constexpr (std::is_void_v<VertexHash>) {
            for(auto vert : _graph.vertices())
                if(vert.data() == data) return vert;
            return std::nullopt;
        }else {
            auto it = _vht.find(data);
            if (it != _vht.end())
                return it->second;
            return std::nullopt;
        }
    }

    template <typename T = EdgeData>
        requires (!std::is_void_v<EdgeData>)
    auto findEdge(const T& data) const
        requires (!std::is_void_v<EdgeData>)
    {
        if constexpr (!std::is_void_v<EdgeHash>) {
            auto [first, last] = _eht.equal_range(data);
            return std::ranges::subrange(first, last)
                   | std::views::transform([](const auto& pair) { return pair.second; });
        } else {
            return _graph.edges()
                   | std::views::filter([data](const auto& edge) { return edge.data() == data; });
        }
    }

    bool containsVertex(const VertexData& data) const
        requires (!std::is_void_v<VertexHash>)
    { return _vht.contains(data); }

    template <typename T = EdgeData>
        requires (!std::is_void_v<EdgeHash> && !std::is_void_v<EdgeData>)
    bool containsEdge(const T& data) const
    { return _eht.contains(data); }
    template <typename T = EdgeData>
        requires (!std::is_void_v<EdgeData> && std::is_void_v<EdgeHash>)
    bool containsEdge(const T& data) const {
        for (auto edge : _graph.edges())
            if (edge.data() == data) return true;
        return false;
    }

    // --- Рёбра ---
    template<typename... Args>
        requires (!std::is_void_v<EdgeData>)
    EdgeDescriptor emplaceEdge(VertexDescriptor from, VertexDescriptor to, Args&&... args)
    {
        auto d = _graph.addEdge(from, to, std::forward<Args>(args)...);
        if constexpr (!std::is_void_v<EdgeHash>) _eht.emplace(d.data(), d);
        return d;
    }

    template<typename T = EdgeData, typename... Args>
        requires (!std::is_void_v<EdgeData>)
    EdgeDescriptor addEdge(VertexDescriptor from, VertexDescriptor to, T&& data)
    { return emplaceEdge(from, to, std::forward<T>(data)); }

    template<typename... Args>
        requires (std::is_void_v<EdgeData>)
    EdgeDescriptor addEdge(VertexDescriptor from, VertexDescriptor to)
    { return _graph.addEdge(from, to); }

    void removeEdge(EdgeDescriptor e) {
        if constexpr(!std::is_void_v<EdgeHash> && !std::is_void_v<EdgeData>) {
            auto [first, last] = _eht.equal_range(e.data());
            auto it = std::find_if(first, last, [&](const auto& pair) { return pair.second == e; });
            if (it != last) _eht.erase(it);
        }

        _graph.removeEdge(e);
    }

    // --- Итераторы вершин ---
    VertexIterator beginVertices()       { return _graph.beginVertices(); }
    VertexIterator endVertices()         { return _graph.endVertices(); }
    ConstVertexIterator beginVertices() const { return _graph.beginVertices(); }
    ConstVertexIterator endVertices()   const { return _graph.endVertices(); }
    ConstVertexIterator cbeginVertices() const { return _graph.cbeginVertices(); }
    ConstVertexIterator cendVertices()   const { return _graph.cendVertices(); }

    auto vertices()       { return _graph.vertices(); }
    auto vertices() const { return _graph.vertices(); }
    auto constVertices() const { return _graph.constVertices(); }

    // --- Итераторы рёбер ---
    EdgeIterator beginEdges()       { return _graph.beginEdges(); }
    EdgeIterator endEdges()         { return _graph.endEdges(); }
    ConstEdgeIterator beginEdges() const { return _graph.beginEdges(); }
    ConstEdgeIterator endEdges()   const { return _graph.endEdges(); }
    ConstEdgeIterator cbeginEdges() const { return _graph.cbeginEdges(); }
    ConstEdgeIterator cendEdges()   const { return _graph.cendEdges(); }

    auto edges()       { return _graph.edges(); }
    auto edges() const { return _graph.edges(); }
    auto constEdges() const { return _graph.constEdges(); }

    // --- Capacity ---
    std::size_t vertexCount() const { return _graph.vertexCount(); }
    std::size_t edgeCount()   const { return _graph.edgeCount(); }

    std::optional<EdgeDescriptor> findEdge(VertexDescriptor from, VertexDescriptor to) const
    { return _graph.findEdge(from, to); }

    bool hasEdge(VertexDescriptor from, VertexDescriptor to) const { return findEdge(from, to).has_value(); }

    // --- Доступ к базовому графу ---
    const UndirectedAbstractGraph<VertexData, EdgeData>& baseAbstractGraph() const { return _graph; }

private:
    struct _EmptyType {};

    using _ConditionalVertexHT = std::conditional_t<std::is_void_v<VertexHash>,
                                                    _EmptyType,
                                                    std::unordered_map<VertexData, VertexDescriptor, VertexHash>>;

    using _ConditionalEdgeMHT = std::conditional_t<(std::is_void_v<EdgeHash> || std::is_void_v<EdgeData>),
                                                  _EmptyType,
                                                  std::unordered_multimap<EdgeData, EdgeDescriptor, EdgeHash>>;

    void _rebuildIndexes() {
        if constexpr (!std::is_void_v<VertexHash>) {
            _vht.clear();
            for (auto v : _graph.vertices())
                _vht.emplace(v.data(), v);
        }

        if constexpr (!std::is_void_v<EdgeHash> && !std::is_void_v<EdgeData>) {
            _eht.clear();
            for (auto e : _graph.edges())
                _eht.emplace(e.data(), e);
        }
    }

    UndirectedAbstractGraph<VertexData, EdgeData> _graph;
    [[no_unique_address]] _ConditionalVertexHT _vht;
    [[no_unique_address]] _ConditionalEdgeMHT _eht;
};

} // namespace exx::incident

#endif // EXX_UNDIRECTEDPSEUDOGRAPH_HPP
