#ifndef EXX_UNDIRECTEDPSEUDOGRAPH_HPP
#define EXX_UNDIRECTEDPSEUDOGRAPH_HPP

#include <unordered_map>
#include <optional>

#include "UndirectedAbstractGraph.hpp"

namespace exx::incident {

template<typename VertexData,
         typename EdgeData,
         typename VertexHash = std::hash<VertexData>>
class UndirectedPseudoGraph {
public:

    using VertexDescriptor      = typename UndirectedAbstractGraph<VertexData, EdgeData>::VertexDescriptor;
    using ConstVertexDescriptor = typename UndirectedAbstractGraph<VertexData, EdgeData>::ConstVertexDescriptor;
    using VertexDescriptorHash  = typename UndirectedAbstractGraph<VertexData, EdgeData>::VertexDescriptorHash;

    using EdgeDescriptor        = typename UndirectedAbstractGraph<VertexData, EdgeData>::EdgeDescriptor;
    using ConstEdgeDescriptor   = typename UndirectedAbstractGraph<VertexData, EdgeData>::ConstEdgeDescriptor;
    using EdgeDescriptorHash    = typename UndirectedAbstractGraph<VertexData, EdgeData>::EdgeDescriptorHash;

    using VertexIterator        = typename UndirectedAbstractGraph<VertexData, EdgeData>::VertexIterator;
    using ConstVertexIterator   = typename UndirectedAbstractGraph<VertexData, EdgeData>::ConstVertexIterator;
    using EdgeIterator          = typename UndirectedAbstractGraph<VertexData, EdgeData>::EdgeIterator;
    using ConstEdgeIterator     = typename UndirectedAbstractGraph<VertexData, EdgeData>::ConstEdgeIterator;


    UndirectedPseudoGraph() = default;

    explicit UndirectedPseudoGraph(const UndirectedAbstractGraph<VertexData, EdgeData>& graph)
        : _topology(graph)
    { _rebuildIndexes(); }

    UndirectedPseudoGraph(const UndirectedPseudoGraph& other)
        : _topology(other._topology)
    { _rebuildIndexes(); }

    UndirectedPseudoGraph(UndirectedPseudoGraph&& other) noexcept
        : _topology(std::move(other._topology)),
        _vht(std::move(other._vht)) {}

    UndirectedPseudoGraph& operator=(const UndirectedPseudoGraph& other) {
        if (this != &other) {
            UndirectedPseudoGraph tmp(other);
            swap(tmp);
        }
        return *this;
    }

    UndirectedPseudoGraph& operator=(UndirectedPseudoGraph&& other) noexcept {
        if (this != &other) {
            _topology = std::move(other._topology);
            _vht   = std::move(other._vht);
        }
        return *this;
    }

    void swap(UndirectedPseudoGraph& other) noexcept {
        using std::swap;
        swap(_topology, other._topology);
        swap(_vht,   other._vht);
    }

    template<typename... Args>
    std::optional<VertexDescriptor> emplaceVertex(Args&&... args) {
        VertexData data(std::forward<Args>(args)...);
        if constexpr (!std::is_void_v<VertexHash>) {
            if (_vht.contains(data)) return std::nullopt;
            VertexDescriptor v = _topology.addVertex(std::move(data));
            _vht.emplace(v.data(), v);
            return v;
        } else {
            for(auto vert : _topology.vertices())
                if(vert.data() == data) return std::nullopt;
            return _topology.emplaceVertex(std::move(data));
        }
    }

    std::optional<VertexDescriptor> addVertex(const VertexData& data)
    { return emplaceVertex(data); }
    std::optional<VertexDescriptor> addVertex(VertexData&& data)
    { return emplaceVertex(std::move(data)); }

    void removeVertex(VertexDescriptor v) {
        if constexpr (!std::is_void_v<VertexHash>)
            _vht.erase(v.data());

        _topology.removeVertex(v);
    }

    std::optional<VertexDescriptor> findVertex(const VertexData& data) const {
        if constexpr (std::is_void_v<VertexHash>) {
            for(auto vert : _topology.vertices())
                if(vert.data() == data) return vert;
            return std::nullopt;
        } else {
            auto it = _vht.find(data);
            if (it != _vht.end())
                return it->second;
            return std::nullopt;
        }
    }

    bool containsVertex(const VertexData& data) const
        requires (!std::is_void_v<VertexHash>)
    { return _vht.contains(data); }

    template<typename... Args>
        requires (!std::is_void_v<EdgeData>)
    EdgeDescriptor emplaceEdge(VertexDescriptor from,
                               VertexDescriptor to,
                               Args&&... args)
    { return _topology.addEdge(from, to, std::forward<Args>(args)...); }

    template<typename T = EdgeData, typename... Args>
        requires (!std::is_void_v<EdgeData>)
    EdgeDescriptor addEdge(VertexDescriptor from,
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
    EdgeDescriptor addEdge(VertexDescriptor from, VertexDescriptor to)
    { return _topology.addEdge(from, to); }

    template<typename... Args>
        requires (std::is_void_v<EdgeData>)
    EdgeDescriptor addEdge(const VertexData& fromData, const VertexData& toData) {
        auto fromOpt = findVertex(fromData);
        auto toOpt = findVertex(toData);
        if(!fromOpt || ! toOpt) return;
        return addEdge(*fromOpt, *toOpt);
    }

    void removeEdge(EdgeDescriptor e) { _topology.removeEdge(e); }

    void removeEdge(const VertexData& fromData, const VertexData& toData) {
        auto fromOpt = findVertex(fromData);
        auto toOpt = findVertex(toData);
        if(!fromOpt || ! toOpt) return;
        auto edgeOpt = findEdge(*fromOpt, *toOpt);
        if(edgeOpt) removeEdge(*edgeOpt);
    }

    VertexIterator beginVertices()       { return _topology.beginVertices(); }
    VertexIterator endVertices()         { return _topology.endVertices(); }
    ConstVertexIterator beginVertices() const { return _topology.beginVertices(); }
    ConstVertexIterator endVertices()   const { return _topology.endVertices(); }
    ConstVertexIterator cbeginVertices() const { return _topology.cbeginVertices(); }
    ConstVertexIterator cendVertices()   const { return _topology.cendVertices(); }

    auto vertices()       { return _topology.vertices(); }
    auto vertices() const { return _topology.vertices(); }
    auto constVertices() const { return _topology.constVertices(); }

    EdgeIterator beginEdges()       { return _topology.beginEdges(); }
    EdgeIterator endEdges()         { return _topology.endEdges(); }
    ConstEdgeIterator beginEdges() const { return _topology.beginEdges(); }
    ConstEdgeIterator endEdges()   const { return _topology.endEdges(); }
    ConstEdgeIterator cbeginEdges() const { return _topology.cbeginEdges(); }
    ConstEdgeIterator cendEdges()   const { return _topology.cendEdges(); }

    auto edges()       { return _topology.edges(); }
    auto edges() const { return _topology.edges(); }
    auto constEdges() const { return _topology.constEdges(); }

    std::size_t vertexCount() const { return _topology.vertexCount(); }
    std::size_t edgeCount()   const { return _topology.edgeCount(); }

    std::optional<EdgeDescriptor> findEdge(VertexDescriptor from, VertexDescriptor to)
    { return _topology.findEdge(from, to); }

    std::optional<ConstEdgeDescriptor> findEdge(VertexDescriptor from, VertexDescriptor to) const
    { return _topology.findEdge(from, to); }

    std::optional<EdgeDescriptor> findEdge(const VertexData& fromData, const VertexData& toData) {
        auto fromOpt = findVertex(fromData);
        auto toOpt = findVertex(toData);
        if(!fromOpt || ! toOpt) return std::nullopt;
        return _topology.findEdge(*fromOpt, *toOpt);
    }

    std::optional<ConstEdgeDescriptor> findEdge(const VertexData& fromData, const VertexData& toData) const {
        auto fromOpt = findVertex(fromData);
        auto toOpt = findVertex(toData);
        if(!fromOpt || ! toOpt) return std::nullopt;
        return _topology.findEdge(*fromOpt, *toOpt);
    }

    bool hasEdge(VertexDescriptor from, VertexDescriptor to) const
    { return findEdge(from, to).has_value(); }

    bool hasEdge(const VertexData& fromData, const VertexData& toData) const {
        auto fromOpt = findVertex(fromData);
        auto toOpt = findVertex(toData);
        if(!fromOpt || ! toOpt) return false;
        return _topology.hasEdge(*fromOpt, *toOpt);
    }

    const UndirectedAbstractGraph<VertexData, EdgeData>& baseAbstractGraph() const
    { return _topology; }

private:
    struct EmptyType {};

    using ConditionalVertexHT = std::conditional_t<std::is_void_v<VertexHash>,
                                                    EmptyType,
                                                    std::unordered_map<VertexData, VertexDescriptor, VertexHash>>;

    void _rebuildIndexes() {
        if constexpr (!std::is_void_v<VertexHash>) {
            _vht.clear();
            for (auto v : _topology.vertices())
                _vht.emplace(v.data(), v);
        }
    }

    UndirectedAbstractGraph<VertexData, EdgeData> _topology;
    [[no_unique_address]] ConditionalVertexHT _vht;
};

} // namespace exx::incident

#endif // EXX_UNDIRECTEDPSEUDOGRAPH_HPP
