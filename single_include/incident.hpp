#ifndef EXX_INCIDENT_HPP
#define EXX_INCIDENT_HPP


/*** Start of inlined file: undirected.hpp ***/
#ifndef EXX_UNDIRECTEDS_HPP
#define EXX_UNDIRECTEDS_HPP


/*** Start of inlined file: UndirectedGraph.hpp ***/
#ifndef EXX_UNDIRECTEDGRAPH_HPP
#define EXX_UNDIRECTEDGRAPH_HPP

#include <optional>
#include <expected>


/*** Start of inlined file: UndirectedMultiGraph.hpp ***/
#ifndef EXX_UNDIRECTEDMULTIGRAPH_HPP
#define EXX_UNDIRECTEDMULTIGRAPH_HPP

#include <optional>

/*** Start of inlined file: UndirectedPseudoGraph.hpp ***/
#ifndef EXX_UNDIRECTEDPSEUDOGRAPH_HPP
#define EXX_UNDIRECTEDPSEUDOGRAPH_HPP

#include <list>
#include <ranges>
#include <type_traits>
#include <optional>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>


/*** Start of inlined file: hash_std_injection.hpp ***/
#ifndef EXX_HASHSTDINJECTION_HPP
#define EXX_HASHSTDINJECTION_HPP

#include <functional>

namespace exx::incident {

template<typename T>
concept HashableByExxIncident =
    requires(const T& v) {
        typename T::CustomHasherProvidedByExxIncident;

        { typename T::CustomHasherProvidedByExxIncident{}(v) }
              -> std::convertible_to<std::size_t>;
    };

} // namespace exx::incident

namespace std {

template<typename T>
    requires exx::incident::HashableByExxIncident<T>
struct hash<T> {
    std::size_t operator()(const T& val) const
    { return typename T::CustomHasherProvidedByExxIncident{}(val); }
};

} // namespace std

#endif // EXX_HASHSTDINJECTION_HPP

/*** End of inlined file: hash_std_injection.hpp ***/

namespace exx::incident {

template<typename VertexData, typename EdgeData>
class UndirectedPseudoGraph {

    static_assert(!std::is_void_v<VertexData>, "VertexData cannot be void");

private:
    struct Vertex;
    struct Edge;

    using VertexList       = std::list<Vertex>;
    using EdgeList         = std::list<Edge>;
    using VertexLabel      = typename VertexList::iterator;
    using VertexConstLabel = typename VertexList::const_iterator;
    using EdgeLabel        = typename EdgeList::iterator;
    using EdgeConstLabel   = typename EdgeList::const_iterator;

    struct EmptyEdgeData final {};
    using ConditionalEdgeData = std::conditional_t<std::is_void_v<EdgeData>,
                                                   EmptyEdgeData,
                                                   EdgeData>;

    struct EraseAccelerationMetaData {
        typename std::list<EdgeLabel>::iterator _posInV1{};
        typename std::list<EdgeLabel>::iterator _posInV2{};
    };

    struct Edge {
        VertexLabel _v1;
        VertexLabel _v2;
        [[no_unique_address]] ConditionalEdgeData _data;
        EraseAccelerationMetaData _meta{};

        template<typename... Args>
        Edge(VertexLabel v1, VertexLabel v2, Args&&... args)
            : _v1(v1), _v2(v2)
        {
            if constexpr (!std::is_void_v<EdgeData>)
                _data = ConditionalEdgeData(std::forward<Args>(args)...);
            else
                static_assert(sizeof...(Args) == 0,
                              "EdgeData is void, cannot pass data to edge");
        }
    };

    struct Vertex {
        std::list<EdgeLabel> _incidentEdges;
        VertexData _data;
        template<typename... Args>
        explicit Vertex(Args&&... args) : _data(std::forward<Args>(args)...) {}
    };

    VertexList _vertices;
    EdgeList   _edges;

    template<bool isConst> class VertexDescriptorImpl;
    template<bool isConst> class EdgeDescriptorImpl;

public:

    struct EdgeDescriptorHash {
        template<bool isConst>
        std::size_t operator()(const EdgeDescriptorImpl<isConst>& desc) const
        { return std::hash<const void*>()( &(*desc._label) ); }
    };

    struct VertexDescriptorHash {
        template<bool isConst>
        std::size_t operator()(const VertexDescriptorImpl<isConst>& desc) const
        { return std::hash<const void*>()( &(*desc._label) ); }
    };

private:

    template<bool isConst>
    class VertexDescriptorImpl {
    private:
        using ConditionalVertexLabel = std::conditional_t<isConst,
                                                          VertexConstLabel,
                                                          VertexLabel>;

        ConditionalVertexLabel _label;

        friend class UndirectedPseudoGraph;

    public:
        VertexDescriptorImpl() = default;
        VertexDescriptorImpl(const VertexDescriptorImpl&) = default;
        VertexDescriptorImpl& operator=(const VertexDescriptorImpl&) = default;

        explicit VertexDescriptorImpl(ConditionalVertexLabel label)
            : _label(label) {}

        VertexDescriptorImpl(const VertexDescriptorImpl<false>& other)
            requires isConst
            : _label(other._label) {}

        auto& data()       requires (!isConst) { return _label->_data; }
        const auto& data() const               { return _label->_data; }

        std::size_t degree() const { return _label->_incidentEdges.size(); }

        auto incidentEdges() const {
            return std::views::transform(_label->_incidentEdges,
                                         [](const EdgeLabel& el) { return EdgeDescriptorImpl<isConst>(el); });
        }

        template<typename Cmp = std::less<VertexData>>
        std::vector<VertexDescriptorImpl> adjacentVertices() const {
            std::unordered_set<VertexDescriptor> unique;
            for (const auto& e : incidentEdges()) {
                auto other = e.otherEnd(*this);
                unique.insert(*other);
            }

            std::vector<VertexDescriptor> res(unique.begin(), unique.end());

            if constexpr(!std::is_void_v<Cmp>)
                std::ranges::sort(res, [](const auto& a, const auto& b)
                                  { return Cmp{}(a.data(), b.data()); });

            return res;
        }

        std::vector<VertexDescriptorImpl> unorderedAdjV() const { return adjacentVertices<void>(); }

        bool operator==(const VertexDescriptorImpl& other) const { return _label == other._label; }
        bool operator!=(const VertexDescriptorImpl& other) const { return !(*this == other); }

        using CustomHasherProvidedByExxIncident = VertexDescriptorHash;
    };

    template<bool isConst>
    class EdgeDescriptorImpl {
    private:
        using ConditionalEdgeLabel = std::conditional_t<isConst,
                                                        EdgeConstLabel,
                                                        EdgeLabel>;

        ConditionalEdgeLabel _label;

        friend class UndirectedPseudoGraph;

    public:
        EdgeDescriptorImpl() = default;
        EdgeDescriptorImpl(const EdgeDescriptorImpl&) = default;
        EdgeDescriptorImpl& operator=(const EdgeDescriptorImpl&) = default;

        explicit EdgeDescriptorImpl(ConditionalEdgeLabel label) : _label(label) {}

        EdgeDescriptorImpl(const EdgeDescriptorImpl<false>& other) requires isConst
            : _label(other._label) {}

        auto& data()       requires (!std::is_void_v<EdgeData> && !isConst) { return _label->_data; }
        const auto& data() const requires (!std::is_void_v<EdgeData>) { return _label->_data; }

        VertexDescriptorImpl<isConst> v() const { return VertexDescriptorImpl<isConst>(_label->_v1); }
        VertexDescriptorImpl<isConst> u() const { return VertexDescriptorImpl<isConst>(_label->_v2); }

        auto otherEnd(const VertexDescriptorImpl<isConst>& vertex) const
            ->std::optional<VertexDescriptorImpl<isConst>>
        {
            if (vertex._label == _label->_v1)
                return VertexDescriptorImpl<isConst>(_label->_v2);
            else if (vertex._label == _label->_v2)
                return VertexDescriptorImpl<isConst>(_label->_v1);
            else
                return std::nullopt;
        }

        bool operator==(const EdgeDescriptorImpl& other) const { return _label == other._label; }
        bool operator!=(const EdgeDescriptorImpl& other) const { return !(*this == other); }

        using CustomHasherProvidedByExxIncident = EdgeDescriptorHash;
    };

    template<bool isConst>
    class VertexIteratorImpl {
    private:
        using ConditionalVertexLabel = std::conditional_t<isConst,
                                                          VertexConstLabel,
                                                          VertexLabel>;

        ConditionalVertexLabel _it;

        friend class UndirectedPseudoGraph;

    public:
        using difference_type       = std::ptrdiff_t;
        using value_type            = VertexDescriptorImpl<isConst>;
        using reference             = value_type;
        using pointer               = void;
        using iterator_category     = std::forward_iterator_tag;
        using iterator_concept      = std::forward_iterator_tag;

        VertexIteratorImpl() = default;
        VertexIteratorImpl(const VertexIteratorImpl&) = default;
        VertexIteratorImpl& operator=(const VertexIteratorImpl&) = default;

        explicit VertexIteratorImpl(ConditionalVertexLabel it) : _it(it) {}

        VertexIteratorImpl(const VertexIteratorImpl<false>& other) requires isConst
            : _it(other._it) {}

        reference operator*() const { return value_type(_it); }
        VertexIteratorImpl& operator++() { ++_it; return *this; }
        VertexIteratorImpl operator++(int) { auto tmp = *this; ++*this; return tmp; }

        bool operator==(const VertexIteratorImpl& other) const { return _it == other._it; }
        bool operator!=(const VertexIteratorImpl& other) const { return !(*this == other); }
    };

    template<bool isConst>
    class EdgeIteratorImpl {
    private:
        using ConditionalEdgeLabel = std::conditional_t<isConst,
                                                        EdgeConstLabel,
                                                        EdgeLabel>;

        ConditionalEdgeLabel _it;

        friend class UndirectedPseudoGraph;

    public:
        using difference_type       = std::ptrdiff_t;
        using value_type            = EdgeDescriptorImpl<isConst>;
        using reference             = value_type;
        using pointer               = void;
        using iterator_category     = std::forward_iterator_tag;
        using iterator_concept      = std::forward_iterator_tag;

        EdgeIteratorImpl() = default;
        EdgeIteratorImpl(const EdgeIteratorImpl&) = default;
        EdgeIteratorImpl& operator=(const EdgeIteratorImpl&) = default;

        explicit EdgeIteratorImpl(ConditionalEdgeLabel it) : _it(it) {}

        EdgeIteratorImpl(const EdgeIteratorImpl<false>& other) requires isConst
            : _it(other._it) {}

        reference operator*() const { return value_type(_it); }
        EdgeIteratorImpl& operator++() { ++_it; return *this; }
        EdgeIteratorImpl operator++(int) { auto tmp = *this; ++*this; return tmp; }

        bool operator==(const EdgeIteratorImpl& other) const { return _it == other._it; }
        bool operator!=(const EdgeIteratorImpl& other) const { return !(*this == other); }
    };

public:

    using VertexValueType = VertexData;
    using EdgeValueType   = EdgeData;

    using VertexDescriptor      = VertexDescriptorImpl<false>;
    using ConstVertexDescriptor = VertexDescriptorImpl<true>;
    using EdgeDescriptor        = EdgeDescriptorImpl<false>;
    using ConstEdgeDescriptor   = EdgeDescriptorImpl<true>;

    using VertexIterator      = VertexIteratorImpl<false>;
    using ConstVertexIterator = VertexIteratorImpl<true>;
    using EdgeIterator        = EdgeIteratorImpl<false>;
    using ConstEdgeIterator   = EdgeIteratorImpl<true>;

    UndirectedPseudoGraph() = default;

    UndirectedPseudoGraph(const UndirectedPseudoGraph& other) {
        std::unordered_map<const Vertex*, VertexDescriptor> origToNew;

        for (const auto& origVertex : other._vertices) {
            VertexDescriptor newV = emplaceVertex(origVertex._data);
            origToNew[&origVertex] = newV;
        }

        for (const auto& origEdge : other._edges) {
            VertexDescriptor newV1 = origToNew[&(*origEdge._v1)];
            VertexDescriptor newV2 = origToNew[&(*origEdge._v2)];

            if constexpr (std::is_void_v<EdgeData>) addEdge(newV1, newV2);
            else addEdge(newV1, newV2, origEdge._data);
        }
    }

    UndirectedPseudoGraph(UndirectedPseudoGraph&&) noexcept = default;

    UndirectedPseudoGraph& operator=(const UndirectedPseudoGraph& other) {
        if (this != &other) {
            UndirectedPseudoGraph temp(other);
            *this = std::move(temp);
        }
        return *this;
    }

    UndirectedPseudoGraph& operator=(UndirectedPseudoGraph&&) noexcept = default;

    template<typename... Args>
    VertexDescriptor emplaceVertex(Args&&... args) {
        _vertices.emplace_back(std::forward<Args>(args)...);
        auto it = std::prev(_vertices.end());
        return VertexDescriptor(it);
    }

    VertexDescriptor addVertex(const VertexData& data) { return emplaceVertex(data); }
    VertexDescriptor addVertex(VertexData&& data) { return emplaceVertex(std::move(data)); }

    void removeVertex(VertexDescriptor vertex) {
        auto incidentCopy = vertex._label->_incidentEdges;
        for (auto edgeLabel : incidentCopy)
            removeEdge(EdgeDescriptor(edgeLabel));
        _vertices.erase(vertex._label);
    }

    template<typename... Args>
    EdgeDescriptor emplaceEdge(VertexDescriptor from, VertexDescriptor to, Args&&... args) {
        _edges.emplace_back(from._label, to._label, std::forward<Args>(args)...);
        auto it = std::prev(_edges.end());

        struct Rollback {
            EdgeList& edges;
            EdgeLabel it;
            VertexLabel v1, v2;
            bool v1_done = false;
            bool v2_done = false;
            bool committed = false;

            Rollback(EdgeList& edges, EdgeLabel it, VertexLabel v1, VertexLabel v2)
                : edges(edges), it(it), v1(v1), v2(v2) {}

            ~Rollback() {
                if (committed) return;
                if (v2_done) v2->_incidentEdges.pop_back();
                if (v1_done) v1->_incidentEdges.pop_back();
                edges.erase(it);
            }

            void commit() { committed = true; }

        } rollback(_edges, it, from._label, to._label);

        from._label->_incidentEdges.push_back(it);
        it->_meta._posInV1 = std::prev(from._label->_incidentEdges.end());
        rollback.v1_done = true;

        to._label->_incidentEdges.push_back(it);
        it->_meta._posInV2 = std::prev(to._label->_incidentEdges.end());
        rollback.v2_done = true;

        rollback.commit();

        return EdgeDescriptor(it);
    }

    template<typename T = EdgeData>
        requires (!std::is_void_v<EdgeData>)
    EdgeDescriptor addEdge(VertexDescriptor from, VertexDescriptor to, T&& data)
    { return emplaceEdge(from, to, std::forward<T>(data)); }

    EdgeDescriptor addEdge(VertexDescriptor from, VertexDescriptor to)
        requires (std::is_void_v<EdgeData>)
    { return emplaceEdge(from, to); }

    void removeEdge(EdgeDescriptor edge) {
        edge._label->_v1->_incidentEdges.erase(edge._label->_meta._posInV1);
        edge._label->_v2->_incidentEdges.erase(edge._label->_meta._posInV2);
        _edges.erase(edge._label);
    }

    VertexIterator beginVertices()             { return VertexIterator(_vertices.begin()); }
    VertexIterator endVertices()               { return VertexIterator(_vertices.end()); }
    ConstVertexIterator beginVertices()  const { return ConstVertexIterator(_vertices.begin()); }
    ConstVertexIterator endVertices()    const { return ConstVertexIterator(_vertices.end()); }
    ConstVertexIterator cbeginVertices() const { return ConstVertexIterator(_vertices.begin()); }
    ConstVertexIterator cendVertices()   const { return ConstVertexIterator(_vertices.end()); }

    auto vertices()
    { return std::ranges::subrange<VertexIterator, VertexIterator>(beginVertices(), endVertices()); }
    auto vertices() const
    { return std::ranges::subrange<ConstVertexIterator, ConstVertexIterator>(beginVertices(), endVertices()); }
    auto constVertices() const
    { return std::ranges::subrange<ConstVertexIterator, ConstVertexIterator>(cbeginVertices(), cendVertices()); }

    EdgeIterator beginEdges()             { return EdgeIterator(_edges.begin()); }
    EdgeIterator endEdges()               { return EdgeIterator(_edges.end()); }
    ConstEdgeIterator beginEdges()  const { return ConstEdgeIterator(_edges.begin()); }
    ConstEdgeIterator endEdges()    const { return ConstEdgeIterator(_edges.end()); }
    ConstEdgeIterator cbeginEdges() const { return ConstEdgeIterator(_edges.begin()); }
    ConstEdgeIterator cendEdges()   const { return ConstEdgeIterator(_edges.end()); }

    auto edges()
    { return std::ranges::subrange<EdgeIterator, EdgeIterator>(beginEdges(), endEdges()); }
    auto edges() const
    { return std::ranges::subrange<ConstEdgeIterator, ConstEdgeIterator>(beginEdges(), endEdges()); }
    auto constEdges() const
    { return std::ranges::subrange<ConstEdgeIterator, ConstEdgeIterator>(cbeginEdges(), cendEdges()); }

    std::size_t vertexCount() const { return _vertices.size(); }
    std::size_t edgeCount()   const { return _edges.size(); }

    bool hasVertex(const VertexData& data) const {
        auto it = std::ranges::find_if(vertices(), [&](auto vd){ return vd.data() == data; });
        return it != vertices().end();
    }

    std::optional<EdgeDescriptor> findEdge(VertexDescriptor from, VertexDescriptor to) {
        for (auto e : from.incidentEdges())
            if (*e.otherEnd(from) == to) return e;
        return std::nullopt;
    }

    std::optional<ConstEdgeDescriptor> findEdge(ConstVertexDescriptor from, ConstVertexDescriptor to) const {
        for (auto e : from.incidentEdges())
            if (*e.otherEnd(from) == to) return e;
        return std::nullopt;
    }

    bool hasEdge(ConstVertexDescriptor from, ConstVertexDescriptor to) const
    { return findEdge(from, to).has_value(); }

    void clear() {
        _edges.clear();
        _vertices.clear();
    }

    bool empty() const { return _vertices.empty(); }
};

} // namespace exx::incident

#endif // EXX_UNDIRECTEDPSEUDOGRAPH_HPP

/*** End of inlined file: UndirectedPseudoGraph.hpp ***/


namespace exx::incident {

template<typename VertexData, typename EdgeData>
class UndirectedMultiGraph {
public:

    using VertexValueType = VertexData;
    using EdgeValueType   = EdgeData;

    using VertexDescriptor      = typename UndirectedPseudoGraph<VertexData, EdgeData>::VertexDescriptor;
    using ConstVertexDescriptor = typename UndirectedPseudoGraph<VertexData, EdgeData>::ConstVertexDescriptor;
    using VertexDescriptorHash  = typename UndirectedPseudoGraph<VertexData, EdgeData>::VertexDescriptorHash;

    using EdgeDescriptor        = typename UndirectedPseudoGraph<VertexData, EdgeData>::EdgeDescriptor;
    using ConstEdgeDescriptor   = typename UndirectedPseudoGraph<VertexData, EdgeData>::ConstEdgeDescriptor;
    using EdgeDescriptorHash    = typename UndirectedPseudoGraph<VertexData, EdgeData>::EdgeDescriptorHash;

    using VertexIterator        = typename UndirectedPseudoGraph<VertexData, EdgeData>::VertexIterator;
    using ConstVertexIterator   = typename UndirectedPseudoGraph<VertexData, EdgeData>::ConstVertexIterator;
    using EdgeIterator          = typename UndirectedPseudoGraph<VertexData, EdgeData>::EdgeIterator;
    using ConstEdgeIterator     = typename UndirectedPseudoGraph<VertexData, EdgeData>::ConstEdgeIterator;

    UndirectedMultiGraph() = default;

    explicit UndirectedMultiGraph(const UndirectedPseudoGraph<VertexData, EdgeData>& graph)
        : _pseudoGraph(graph) {}

    UndirectedMultiGraph(const UndirectedMultiGraph&) = default;
    UndirectedMultiGraph(UndirectedMultiGraph&&) noexcept = default;

    UndirectedMultiGraph& operator=(const UndirectedMultiGraph&) = default;
    UndirectedMultiGraph& operator=(UndirectedMultiGraph&&) noexcept = default;

    void swap(UndirectedMultiGraph& other) noexcept { _pseudoGraph.swap(other._pseudoGraph); }

    template<typename... Args>
    VertexDescriptor emplaceVertex(Args&&... args)
    { return _pseudoGraph.emplaceVertex(std::forward<Args>(args)...); }

    VertexDescriptor addVertex(const VertexData& data)
    { return _pseudoGraph.addVertex(data); }

    VertexDescriptor addVertex(VertexData&& data)
    { return _pseudoGraph.addVertex(std::move(data)); }

    void removeVertex(VertexDescriptor v)
    { _pseudoGraph.removeVertex(v); }

    template<typename... Args>
    std::optional<EdgeDescriptor> emplaceEdge(VertexDescriptor from,
                                              VertexDescriptor to,
                                              Args&&... args)
    {
        if (from == to) return std::nullopt;
        return _pseudoGraph.emplaceEdge(from, to, std::forward<Args>(args)...);
    }

    template<typename T = EdgeData>
        requires (!std::is_void_v<EdgeData>)
    std::optional<EdgeDescriptor> addEdge(VertexDescriptor from,
                                          VertexDescriptor to,
                                          T&& data)
    { return emplaceEdge(from, to, std::forward<T>(data)); }

    std::optional<EdgeDescriptor> addEdge(VertexDescriptor from, VertexDescriptor to)
        requires (std::is_void_v<EdgeData>)
    { return emplaceEdge(from, to); }

    void removeEdge(EdgeDescriptor e) { _pseudoGraph.removeEdge(e); }

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
    std::optional<ConstEdgeDescriptor> findEdge(ConstVertexDescriptor from, ConstVertexDescriptor to) const
    { return _pseudoGraph.findEdge(from, to); }

    bool hasEdge(ConstVertexDescriptor from, ConstVertexDescriptor to) const
    { return findEdge(from, to).has_value(); }

    const UndirectedPseudoGraph<VertexData, EdgeData>& basePseudoGraph() const
    { return _pseudoGraph; }

    void clear() { _pseudoGraph.clear(); }

    bool empty() const { return _pseudoGraph.empty(); }

private:

    UndirectedPseudoGraph<VertexData, EdgeData> _pseudoGraph;

};

} // namespace exx::incident

#endif // EXX_UNDIRECTEDMULTIGRAPH_HPP

/*** End of inlined file: UndirectedMultiGraph.hpp ***/


/*** Start of inlined file: Matrix.hpp ***/
#ifndef MATRIX_HPP
#define MATRIX_HPP

#include <vector>

namespace exx::incident {

template<typename T>
class Matrix {
public:
    using value_type = T;

    explicit Matrix(std::size_t rows, std::size_t cols)
        : _rows(rows), _cols(cols), _storage(rows * cols) {}

    std::size_t rows() const noexcept { return _rows; }
    std::size_t cols() const noexcept { return _cols; }

    const T* data() const { return _storage.data(); }

    decltype(auto) operator()(std::size_t i, std::size_t j)
    { return _storage[i * _cols + j]; }

    decltype(auto) operator()(std::size_t i, std::size_t j) const
    { return _storage[i * _cols + j]; }

private:
    std::size_t _rows, _cols;
    std::vector<T> _storage;
};

} // namespace exx::incident

#endif // MATRIX_HPP

/*** End of inlined file: Matrix.hpp ***/


/*** Start of inlined file: errors.hpp ***/
#ifndef EXX_ERRORS_HPP
#define EXX_ERRORS_HPP

#include <string>

namespace exx::incident {

enum class GraphBuildingError {
    NullPointer,
    EmptyVector,
    ZeroSize,
    NonSquareMatrix
};

inline std::string to_string(GraphBuildingError error) noexcept {
    using enum GraphBuildingError;
    switch (error) {
    case GraphBuildingError::NullPointer:          return "NullPointer: input matrix pointer is null";
    case GraphBuildingError::EmptyVector:          return "EmptyVector: there are no data in vector";
    case GraphBuildingError::ZeroSize:             return "ZeroSize: matrix size is zero";
    case GraphBuildingError::NonSquareMatrix:      return "NonSquareMatrix: matrix is not square";
    default:                                       return "Unknown error";
    }
}

enum class PrimError {
    DisconnectedGraph
};

inline std::string to_string(PrimError e) {
    switch (e) {
    case PrimError::DisconnectedGraph: return "Graph is disconnected.";
    default:                           return "Unknown error";
    }
}

} // namespace exx::incident

#endif // EXX_ERRORS_HPP

/*** End of inlined file: errors.hpp ***/

namespace exx::incident {

template<typename VertexData, typename EdgeData>
class UndirectedGraph {
public:

    using VertexValueType = VertexData;
    using EdgeValueType   = EdgeData;

    using VertexDescriptor      = typename UndirectedMultiGraph<VertexData, EdgeData>::VertexDescriptor;
    using ConstVertexDescriptor = typename UndirectedMultiGraph<VertexData, EdgeData>::ConstVertexDescriptor;
    using VertexDescriptorHash  = typename UndirectedMultiGraph<VertexData, EdgeData>::VertexDescriptorHash;

    using EdgeDescriptor        = typename UndirectedMultiGraph<VertexData, EdgeData>::EdgeDescriptor;
    using ConstEdgeDescriptor   = typename UndirectedMultiGraph<VertexData, EdgeData>::ConstEdgeDescriptor;
    using EdgeDescriptorHash    = typename UndirectedMultiGraph<VertexData, EdgeData>::EdgeDescriptorHash;

    using VertexIterator        = typename UndirectedMultiGraph<VertexData, EdgeData>::VertexIterator;
    using ConstVertexIterator   = typename UndirectedMultiGraph<VertexData, EdgeData>::ConstVertexIterator;
    using EdgeIterator          = typename UndirectedMultiGraph<VertexData, EdgeData>::EdgeIterator;
    using ConstEdgeIterator     = typename UndirectedMultiGraph<VertexData, EdgeData>::ConstEdgeIterator;

    UndirectedGraph() = default;

    explicit UndirectedGraph(const UndirectedPseudoGraph<VertexData, EdgeData>& graph)
        : _multiGraph(graph) {}

    UndirectedGraph(const UndirectedGraph&) = default;
    UndirectedGraph(UndirectedGraph&&) noexcept = default;

    UndirectedGraph& operator=(const UndirectedGraph&) = default;
    UndirectedGraph& operator=(UndirectedGraph&&) noexcept = default;

    void swap(UndirectedGraph& other) noexcept { _multiGraph.swap(other._multiGraph); }

    template<typename... Args>
    VertexDescriptor emplaceVertex(Args&&... args)
    { return _multiGraph.emplaceVertex(std::forward<Args>(args)...); }

    VertexDescriptor addVertex(const VertexData& data)
    { return _multiGraph.addVertex(data); }

    VertexDescriptor addVertex(VertexData&& data)
    { return _multiGraph.addVertex(std::move(data)); }

    void removeVertex(VertexDescriptor v) { _multiGraph.removeVertex(v); }

    template<typename... Args>
    std::optional<EdgeDescriptor> emplaceEdge(VertexDescriptor from,
                                              VertexDescriptor to,
                                              Args&&... args)
    {
        if (from == to) return std::nullopt;
        if (_multiGraph.hasEdge(from, to)) return std::nullopt;
        return _multiGraph.emplaceEdge(from, to, std::forward<Args>(args)...);
    }

    template<typename T = EdgeData>
        requires (!std::is_void_v<EdgeData>)
    std::optional<EdgeDescriptor> addEdge(VertexDescriptor from,
                                          VertexDescriptor to,
                                          T&& data)
    { return emplaceEdge(from, to, std::forward<T>(data)); }

    std::optional<EdgeDescriptor> addEdge(VertexDescriptor from, VertexDescriptor to)
        requires (std::is_void_v<EdgeData>)
    { return emplaceEdge(from, to); }

    void removeEdge(EdgeDescriptor e) { _multiGraph.removeEdge(e); }

    std::optional<EdgeDescriptor> findEdge(VertexDescriptor from, VertexDescriptor to)
    { return _multiGraph.findEdge(from, to); }
    std::optional<ConstEdgeDescriptor> findEdge(ConstVertexDescriptor from, ConstVertexDescriptor to) const
    { return _multiGraph.findEdge(from, to); }

    bool hasEdge(ConstVertexDescriptor from, ConstVertexDescriptor to) const
    { return _multiGraph.hasEdge(from, to); }

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

    const UndirectedMultiGraph<VertexData, EdgeData>& baseMultiGraph() const
    { return _multiGraph; }

    void clear() { _multiGraph.clear(); }

    bool empty() const { return _multiGraph.empty(); }

    template <typename T = EdgeData>
        requires (!std::is_void_v<EdgeData>)
    Matrix<T> toAdjacencyMatrix(T default_value = T{}) const {
        const std::size_t n = vertexCount();
        Matrix<T> mat(n, n);

        for (std::size_t i = 0; i < n; ++i)
            for (std::size_t j = 0; j < n; ++j)
                mat(i, j) = default_value;

        for (auto v : vertices()) {
            for (auto e : v.incidentEdges()) {
                auto u = *e.otherEnd(v);
                mat(v.data(), u.data()) = e.data();
                mat(u.data(), v.data()) = e.data();
            }
        }
        return mat;
    }

    Matrix<bool> toAdjacencyMatrix() const
        requires std::is_void_v<EdgeData>
    {
        const std::size_t n = vertexCount();
        Matrix<bool> mat(n, n);

        for (std::size_t i = 0; i < n; ++i)
            for (std::size_t j = 0; j < n; ++j)
                mat(i, j) = false;

        for (auto v : vertices()) {
            for (auto e : v.incidentEdges()) {
                auto u = *e.otherEnd(v);
                mat(v.data(), u.data()) = true;
                mat(u.data(), v.data()) = true;
            }
        }
        return mat;
    }

    template <typename T = EdgeData>
        requires (!std::is_void_v<EdgeData>)
    static auto fromAdjacencyMatrix(const T* matrix, std::size_t n)
        ->std::expected<UndirectedGraph<VertexData, T>, GraphBuildingError>
    {
        if (!matrix) return std::unexpected(GraphBuildingError::NullPointer);
        if (n == 0) return std::unexpected(GraphBuildingError::ZeroSize);

        UndirectedGraph<VertexData, EdgeData> g;
        std::unordered_map<std::size_t,
                           typename UndirectedGraph<VertexData, EdgeData>::VertexDescriptor> ht;

        for (std::size_t i = 0; i < n; ++i) {
            auto desc = g.addVertex(static_cast<VertexData>(i));
            ht.emplace(i, desc);
        }

        for (std::size_t i = 0; i < n; ++i) {
            for (std::size_t j = i + 1; j < n; ++j) {
                auto val = matrix[i * n + j];
                if (val != EdgeData{})
                    g.addEdge(ht[i], ht[j], val);
            }
        }

        return g;
    }

    template <typename T = EdgeData>
        requires (!std::is_void_v<EdgeData>)
    static auto fromAdjacencyMatrix(const std::vector<std::vector<T>>& matrix)
        ->std::expected<UndirectedGraph<VertexData, T>, GraphBuildingError>
    {
        if (matrix.empty())
            return std::unexpected(GraphBuildingError::ZeroSize);

        std::size_t n = matrix.size();
        std::vector<EdgeData> flat;
        flat.reserve(n * n);
        for (const auto& row : matrix) {
            if (row.size() != n) return std::unexpected(GraphBuildingError::NonSquareMatrix);
            flat.insert(flat.end(), row.begin(), row.end());
        }
        return fromAdjacencyMatrix<EdgeData>(flat.data(), n);
    }

    static auto fromAdjacencyMatrix(const std::vector<bool>& matrix, std::size_t n)
        ->std::expected<UndirectedGraph<VertexData, void>, GraphBuildingError>
        requires std::is_void_v<EdgeData>
    {
        if (matrix.empty()) return std::unexpected(GraphBuildingError::EmptyVector);
        if (n == 0) return std::unexpected(GraphBuildingError::ZeroSize);

        UndirectedGraph<VertexData, void> g;
        std::unordered_map<VertexData,
                           typename UndirectedGraph<VertexData, void>::VertexDescriptor> ht;

        for (std::size_t i = 0; i < n; ++i) {
            auto desc = g.addVertex(static_cast<VertexData>(i));
            ht.emplace(i, desc);
        }

        for (std::size_t i = 0; i < n; ++i) {
            for (std::size_t j = i + 1; j < n; ++j) {
                auto val = matrix[i * n + j];
                if (val) g.addEdge(ht[i], ht[j]);
            }
        }

        return g;
    }

    static auto fromAdjacencyMatrix(const std::vector<std::vector<bool>>& matrix)
        ->std::expected<UndirectedGraph<VertexData, void>, GraphBuildingError>
        requires std::is_void_v<EdgeData>
    {
        if (matrix.empty())
            return std::unexpected(GraphBuildingError::ZeroSize);

        std::size_t n = matrix.size();
        std::vector<bool> flat;
        flat.reserve(n * n);
        for (const auto& row : matrix) {
            if (row.size() != n) return std::unexpected(GraphBuildingError::NonSquareMatrix);
            flat.insert(flat.end(), row.begin(), row.end());
        }
        return fromAdjacencyMatrix(flat, n);
    }

private:

    UndirectedMultiGraph<VertexData, EdgeData> _multiGraph;

};

} // namespace exx::incident

#endif // EXX_UNDIRECTEDGRAPH_HPP

/*** End of inlined file: UndirectedGraph.hpp ***/

#endif // EXX_UNDIRECTEDS_HPP

/*** End of inlined file: undirected.hpp ***/


/*** Start of inlined file: directed.hpp ***/
#ifndef EXX_DIRECTED_HPP
#define EXX_DIRECTED_HPP


/*** Start of inlined file: DirectedGraph.hpp ***/
#ifndef EXX_DIRECTEDGRAPH_HPP
#define EXX_DIRECTEDGRAPH_HPP

#include <optional>
#include <expected>


/*** Start of inlined file: DirectedMultiGraph.hpp ***/
#ifndef EXX_DIRECTEDMULTIGRAPH_HPP
#define EXX_DIRECTEDMULTIGRAPH_HPP

#include <optional>


/*** Start of inlined file: DirectedPseudoGraph.hpp ***/
#ifndef EXX_DIRECTEDPSEUDOGRAPH_HPP
#define EXX_DIRECTEDPSEUDOGRAPH_HPP

#include <list>
#include <ranges>
#include <type_traits>
#include <optional>
#include <unordered_set>
#include <algorithm>

namespace exx::incident {

template<typename VertexData, typename ArcData>
class DirectedPseudoGraph {

    static_assert(!std::is_void_v<VertexData>, "VertexData cannot be void");

private:
    struct Vertex;
    struct Arc;

    using VertexList       = std::list<Vertex>;
    using ArcList          = std::list<Arc>;
    using VertexLabel      = typename VertexList::iterator;
    using VertexConstLabel = typename VertexList::const_iterator;
    using ArcLabel         = typename ArcList::iterator;
    using ArcConstLabel    = typename ArcList::const_iterator;

    struct EmptyArcData final {};
    using ConditionalArcData = std::conditional_t<std::is_void_v<ArcData>,
                                                  EmptyArcData,
                                                  ArcData>;

    struct EraseAccelerationMetaData {
        typename std::list<ArcLabel>::iterator _posInFrom{};
        typename std::list<ArcLabel>::iterator _posInTo{};
    };

    struct Arc {
        VertexLabel _from;
        VertexLabel _to;
        [[no_unique_address]] ConditionalArcData _data;
        EraseAccelerationMetaData _meta{};

        template<typename... Args>
        Arc(VertexLabel from, VertexLabel to, Args&&... args)
            : _from(from), _to(to)
        {
            if constexpr (!std::is_void_v<ArcData>)
                _data = ConditionalArcData(std::forward<Args>(args)...);
            else
                static_assert(sizeof...(Args) == 0,
                              "ArcData is void, cannot pass data to edge");
        }
    };

    struct Vertex {
        std::list<ArcLabel> _outgoingArcs;
        std::list<ArcLabel> _incomingArcs;
        VertexData _data;
        template<typename... Args>
        explicit Vertex(Args&&... args) : _data(std::forward<Args>(args)...) {}
    };

    VertexList _vertices;
    ArcList    _arcs;

    template<bool isConst> class VertexDescriptorImpl;
    template<bool isConst> class ArcDescriptorImpl;

public:

    struct ArcDescriptorHash {
        template<bool isConst>
        std::size_t operator()(const ArcDescriptorImpl<isConst>& desc) const
        { return std::hash<const void*>()( &(*desc._label) ); }
    };

    struct VertexDescriptorHash {
        template<bool isConst>
        std::size_t operator()(const VertexDescriptorImpl<isConst>& desc) const
        { return std::hash<const void*>()( &(*desc._label) ); }
    };

private:

    template<bool isConst>
    class VertexDescriptorImpl {
    private:
        using ConditionalVertexLabel = std::conditional_t<isConst,
                                                          VertexConstLabel,
                                                          VertexLabel>;

        ConditionalVertexLabel _label;

        friend class DirectedPseudoGraph;

    public:
        VertexDescriptorImpl() = default;
        VertexDescriptorImpl(const VertexDescriptorImpl&) = default;
        VertexDescriptorImpl& operator=(const VertexDescriptorImpl&) = default;

        explicit VertexDescriptorImpl(ConditionalVertexLabel label)
            : _label(label) {}

        VertexDescriptorImpl(const VertexDescriptorImpl<false>& other)
            requires isConst
            : _label(other._label) {}

        auto& data()       requires (!isConst) { return _label->_data; }
        const auto& data() const               { return _label->_data; }

        std::size_t outDegree() const { return _label->_outgoingArcs.size(); }
        std::size_t inDegree() const { return _label->_incomingArcs.size(); }

        auto outgoingArcs() const {
            return std::views::transform(_label->_outgoingArcs,
                                         [](const ArcLabel& al) { return ArcDescriptorImpl<isConst>(al); });
        }

        auto incomingArcs() const {
            return std::views::transform(_label->_incomingArcs,
                                         [](const ArcLabel& al) { return ArcDescriptorImpl<isConst>(al); });
        }

        template<typename Cmp = std::less<VertexData>>
        std::vector<VertexDescriptorImpl> adjacentVertices() const {
            std::unordered_set<VertexDescriptorImpl> unique;
            for (auto e : outgoingArcs()) {
                auto other = e.followArcDirection(*this);
                unique.insert(*other);
            }

            std::vector<VertexDescriptorImpl> res(unique.begin(), unique.end());

            if constexpr(!std::is_void_v<Cmp>)
                std::ranges::sort(res, [](const auto& a, const auto& b)
                                  { return Cmp{}(a.data(), b.data()); });

            return res;
        }

        std::vector<VertexDescriptorImpl> unorderedOutV() const { return adjacentVertices<void>(); }

        template<typename Cmp = std::less<VertexData>>
        std::vector<VertexDescriptorImpl> incomingVertices() const {
            std::unordered_set<VertexDescriptorImpl> unique;
            for (auto a : incomingArcs())
                unique.insert(a.from());

            std::vector<VertexDescriptorImpl> res(unique.begin(), unique.end());

            if constexpr(!std::is_void_v<Cmp>)
                std::ranges::sort(res, [](const auto& a, const auto& b)
                                  { return Cmp{}(a.data(), b.data()); });

            return res;
        }

        std::vector<VertexDescriptorImpl> unorderedInV() const { return incomingVertices<void>(); }

        bool operator==(const VertexDescriptorImpl& other) const { return _label == other._label; }
        bool operator!=(const VertexDescriptorImpl& other) const { return !(*this == other); }

        using CustomHasherProvidedByExxIncident = VertexDescriptorHash;
    };

    template<bool isConst>
    class ArcDescriptorImpl {
    private:
        using ConditionalArcLabel = std::conditional_t<isConst,
                                                       ArcConstLabel,
                                                       ArcLabel>;

        ConditionalArcLabel _label;

        friend class DirectedPseudoGraph;

    public:
        ArcDescriptorImpl() = default;
        ArcDescriptorImpl(const ArcDescriptorImpl&) = default;
        ArcDescriptorImpl& operator = (const ArcDescriptorImpl&) = default;

        explicit ArcDescriptorImpl(ConditionalArcLabel label) : _label(label) {}

        ArcDescriptorImpl(const ArcDescriptorImpl<false>& other) requires isConst
            : _label(other._label) {}

        auto& data()       requires (!std::is_void_v<ArcData> && !isConst) { return _label->_data; }
        const auto& data() const requires (!std::is_void_v<ArcData>) { return _label->_data; }

        VertexDescriptorImpl<isConst> from() const { return VertexDescriptorImpl<isConst>(_label->_from); }
        VertexDescriptorImpl<isConst> to()   const { return VertexDescriptorImpl<isConst>(_label->_to); }

        auto followArcDirection(const VertexDescriptorImpl<isConst> vertex) const
            ->std::optional<VertexDescriptorImpl<isConst>>
        {
            if (vertex._label == _label->_from)
                return VertexDescriptorImpl<isConst>(_label->_to);
            else
                return std::nullopt;
        }

        auto otherEnd(const VertexDescriptorImpl<isConst> vertex) const
            ->std::optional<VertexDescriptorImpl<isConst>>
        {
            if (vertex._label == _label->_from)
                return VertexDescriptorImpl<isConst>(_label->_to);
            else if (vertex._label == _label->_to)
                return VertexDescriptorImpl<isConst>(_label->_from);
            else
                return std::nullopt;
        }

        bool operator==(const ArcDescriptorImpl& other) const { return _label == other._label; }
        bool operator!=(const ArcDescriptorImpl& other) const { return !(*this == other); }

        using CustomHasherProvidedByExxIncident = ArcDescriptorHash;
    };

    template<bool isConst>
    class VertexIteratorImpl {
    private:
        using ConditionalVertexLabel = std::conditional_t<isConst,
                                                          VertexConstLabel,
                                                          VertexLabel>;

        ConditionalVertexLabel _it;

        friend class DirectedPseudoGraph;

    public:
        using difference_type       = std::ptrdiff_t;
        using value_type            = VertexDescriptorImpl<isConst>;
        using reference             = value_type;
        using pointer               = void;
        using iterator_category     = std::forward_iterator_tag;
        using iterator_concept      = std::forward_iterator_tag;

        VertexIteratorImpl() = default;
        VertexIteratorImpl(const VertexIteratorImpl&) = default;
        VertexIteratorImpl& operator = (const VertexIteratorImpl&) = default;

        explicit VertexIteratorImpl(ConditionalVertexLabel it) : _it(it) {}

        VertexIteratorImpl(const VertexIteratorImpl<false>& other) requires isConst
            : _it(other._it) {}

        reference operator*() const { return value_type(_it); }
        VertexIteratorImpl& operator++() { ++_it; return *this; }
        VertexIteratorImpl operator++(int) { auto tmp = *this; ++*this; return tmp; }

        bool operator==(const VertexIteratorImpl& other) const { return _it == other._it; }
        bool operator!=(const VertexIteratorImpl& other) const { return !(*this == other); }
    };

    template<bool isConst>
    class ArcIteratorImpl {
    private:
        using ConditionalArcLabel = std::conditional_t<isConst,
                                                       ArcConstLabel,
                                                       ArcLabel>;

        ConditionalArcLabel _it;

        friend class DirectedPseudoGraph;

    public:
        using difference_type       = std::ptrdiff_t;
        using value_type            = ArcDescriptorImpl<isConst>;
        using reference             = value_type;
        using pointer               = void;
        using iterator_category     = std::forward_iterator_tag;
        using iterator_concept      = std::forward_iterator_tag;

        ArcIteratorImpl() = default;
        ArcIteratorImpl(const ArcIteratorImpl&) = default;
        ArcIteratorImpl& operator = (const ArcIteratorImpl&) = default;

        explicit ArcIteratorImpl(ConditionalArcLabel it) : _it(it) {}

        ArcIteratorImpl(const ArcIteratorImpl<false>& other) requires isConst
            : _it(other._it) {}

        reference operator*() const { return value_type(_it); }
        ArcIteratorImpl& operator++() { ++_it; return *this; }
        ArcIteratorImpl operator++(int) { auto tmp = *this; ++*this; return tmp; }

        bool operator==(const ArcIteratorImpl& other) const { return _it == other._it; }
        bool operator!=(const ArcIteratorImpl& other) const { return !(*this == other); }
    };

public:

    using VertexValueType = VertexData;
    using ArcValueType   = ArcData;

    using VertexDescriptor      = VertexDescriptorImpl<false>;
    using ConstVertexDescriptor = VertexDescriptorImpl<true>;
    using ArcDescriptor         = ArcDescriptorImpl<false>;
    using ConstArcDescriptor    = ArcDescriptorImpl<true>;

    using VertexIterator      = VertexIteratorImpl<false>;
    using ConstVertexIterator = VertexIteratorImpl<true>;
    using ArcIterator         = ArcIteratorImpl<false>;
    using ConstArcIterator    = ArcIteratorImpl<true>;

    DirectedPseudoGraph() = default;

    DirectedPseudoGraph(const DirectedPseudoGraph& other) {
        std::unordered_map<const Vertex*, VertexDescriptor> origToNew;

        for (const auto& origVertex : other._vertices) {
            VertexDescriptor newV = emplaceVertex(origVertex._data);
            origToNew[&origVertex] = newV;
        }

        for (const auto& origArc : other._arcs) {
            VertexDescriptor newFrom = origToNew[&(*origArc._from)];
            VertexDescriptor newTo = origToNew[&(*origArc._to)];

            if constexpr (std::is_void_v<ArcData>) addArc(newFrom, newTo);
            else addArc(newFrom, newTo, origArc._data);
        }
    }

    DirectedPseudoGraph(DirectedPseudoGraph&&) noexcept = default;

    DirectedPseudoGraph& operator=(const DirectedPseudoGraph& other) {
        if (this != &other) {
            DirectedPseudoGraph temp(other);
            *this = std::move(temp);
        }
        return *this;
    }

    DirectedPseudoGraph& operator=(DirectedPseudoGraph&&) noexcept = default;

    template<typename... Args>
    VertexDescriptor emplaceVertex(Args&&... args) {
        _vertices.emplace_back(std::forward<Args>(args)...);
        auto it = std::prev(_vertices.end());
        return VertexDescriptor(it);
    }

    VertexDescriptor addVertex(const VertexData& data) { return emplaceVertex(data); }
    VertexDescriptor addVertex(VertexData&& data) { return emplaceVertex(std::move(data)); }

    void removeVertex(VertexDescriptor vertex) {
        auto outgoingArcsCopy = vertex._label->_outgoingArcs;
        for (auto arcLabel : outgoingArcsCopy)
            removeArc(ArcDescriptor(arcLabel));

        auto incomingArcsCopy = vertex._label->_incomingArcs;
        for (auto arcLabel : incomingArcsCopy)
            removeArc(ArcDescriptor(arcLabel));

        _vertices.erase(vertex._label);
    }

    template<typename... Args>
    ArcDescriptor emplaceArc(VertexDescriptor from, VertexDescriptor to, Args&&... args) {
        _arcs.emplace_back(from._label, to._label, std::forward<Args>(args)...);
        auto it = std::prev(_arcs.end());

        struct Rollback {
            ArcList& arcs;
            ArcLabel it;
            VertexLabel from, to;
            bool from_done = false;
            bool to_done = false;
            bool committed = false;

            Rollback(ArcList& arcs, ArcLabel it, VertexLabel v1, VertexLabel v2)
                : arcs(arcs), it(it), from(v1), to(v2) {}

            ~Rollback() {
                if (committed) return;
                if (to_done) to->_incomingArcs.pop_back();
                if (from_done) from->_outgoingArcs.pop_back();
                arcs.erase(it);
            }

            void commit() { committed = true; }

        } rollback(_arcs, it, from._label, to._label);

        from._label->_outgoingArcs.push_back(it);
        it->_meta._posInFrom = std::prev(from._label->_outgoingArcs.end());
        rollback.from_done = true;

        to._label->_incomingArcs.push_back(it);
        it->_meta._posInTo = std::prev(to._label->_incomingArcs.end());
        rollback.to_done = true;

        rollback.commit();

        return ArcDescriptor(it);
    }

    template<typename T = ArcData>
        requires (!std::is_void_v<ArcData>)
    ArcDescriptor addArc(VertexDescriptor from, VertexDescriptor to, T&& data)
    { return emplaceArc(from, to, std::forward<T>(data)); }

    ArcDescriptor addArc(VertexDescriptor from, VertexDescriptor to)
        requires (std::is_void_v<ArcData>)
    { return emplaceArc(from, to); }

    void removeArc(ArcDescriptor arc) {
        arc._label->_from->_outgoingArcs.erase(arc._label->_meta._posInFrom);
        arc._label->_to->_incomingArcs.erase(arc._label->_meta._posInTo);
        _arcs.erase(arc._label);
    }

    VertexIterator beginVertices() { return VertexIterator(_vertices.begin()); }
    VertexIterator endVertices()   { return VertexIterator(_vertices.end()); }
    ConstVertexIterator beginVertices() const { return ConstVertexIterator(_vertices.begin()); }
    ConstVertexIterator endVertices()   const { return ConstVertexIterator(_vertices.end()); }
    ConstVertexIterator cbeginVertices() const { return ConstVertexIterator(_vertices.begin()); }
    ConstVertexIterator cendVertices()   const { return ConstVertexIterator(_vertices.end()); }

    auto vertices()
    { return std::ranges::subrange<VertexIterator, VertexIterator>(beginVertices(), endVertices()); }
    auto vertices() const
    { return std::ranges::subrange<ConstVertexIterator, ConstVertexIterator>(beginVertices(), endVertices()); }
    auto constVertices() const
    { return std::ranges::subrange<ConstVertexIterator, ConstVertexIterator>(cbeginVertices(), cendVertices()); }

    ArcIterator beginArcs() { return ArcIterator(_arcs.begin()); }
    ArcIterator endArcs()   { return ArcIterator(_arcs.end()); }
    ConstArcIterator beginArcs() const { return ConstArcIterator(_arcs.begin()); }
    ConstArcIterator endArcs()   const { return ConstArcIterator(_arcs.end()); }
    ConstArcIterator cbeginArcs() const { return ConstArcIterator(_arcs.begin()); }
    ConstArcIterator cendArcs()   const { return ConstArcIterator(_arcs.end()); }

    auto arcs()
    { return std::ranges::subrange<ArcIterator, ArcIterator>(beginArcs(), endArcs()); }
    auto arcs() const
    { return std::ranges::subrange<ConstArcIterator, ConstArcIterator>(beginArcs(), endArcs()); }
    auto constArcs() const
    { return std::ranges::subrange<ConstArcIterator, ConstArcIterator>(cbeginArcs(), cendArcs()); }

    std::size_t vertexCount() const { return _vertices.size(); }
    std::size_t arcCount() const { return _arcs.size(); }

    bool hasVertex(const VertexData& data) const {
        auto it = std::ranges::find_if(vertices(), [&](auto vd){ return vd.data() == data; });
        return it != vertices().end();
    }

    std::optional<ArcDescriptor> findArc(VertexDescriptor from, VertexDescriptor to) {
        for (auto e : from.outgoingArcs())
            if (*e.followArcDirection(from) == to) return e;
        return std::nullopt;
    }

    std::optional<ConstArcDescriptor> findArc(ConstVertexDescriptor from, ConstVertexDescriptor to) const {
        for (auto e : from.outgoingArcs())
            if (*e.followArcDirection(from) == to) return e;
        return std::nullopt;
    }

    bool hasArc(ConstVertexDescriptor from, ConstVertexDescriptor to) const
    { return findArc(from, to).has_value(); }

    void clear() {
        _arcs.clear();
        _vertices.clear();
    }

    bool empty() const { return _vertices.empty(); }
};

} // namespace exx::incident

#endif // EXX_DIRECTEDPSEUDOGRAPH_HPP

/*** End of inlined file: DirectedPseudoGraph.hpp ***/

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

    VertexDescriptor addVertex(const VertexData& data)
    { return _pseudoGraph.addVertex(data); }

    VertexDescriptor addVertex(VertexData&& data)
    { return _pseudoGraph.addVertex(std::move(data)); }

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

private:

    DirectedPseudoGraph<VertexData, ArcData> _pseudoGraph;

};

} // namespace exx::incident

#endif // EXX_DIRECTEDMULTIGRAPH_HPP

/*** End of inlined file: DirectedMultiGraph.hpp ***/

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

    VertexDescriptor addVertex(const VertexData& data)
    { return _multiGraph.addVertex(data); }

    VertexDescriptor addVertex(VertexData&& data)
    { return _multiGraph.addVertex(std::move(data)); }

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

    template <typename T = ArcData>
        requires (!std::is_void_v<ArcData>)
    Matrix<T> toAdjacencyMatrix(T default_value = T{}) const {
        const std::size_t n = vertexCount();
        Matrix<T> mat(n, n);

        for (std::size_t i = 0; i < n; ++i)
            for (std::size_t j = 0; j < n; ++j)
                mat(i, j) = default_value;

        for (auto v : vertices()) {
            for (auto a : v.outgoingArcs()) {
                auto u = a.to();
                mat(v.data(), u.data()) = a.data();
            }
        }
        return mat;
    }

    Matrix<bool> toAdjacencyMatrix() const
        requires std::is_void_v<ArcData>
    {
        const std::size_t n = vertexCount();
        Matrix<bool> mat(n, n);

        for (std::size_t i = 0; i < n; ++i)
            for (std::size_t j = 0; j < n; ++j)
                mat(i, j) = false;

        for (auto v : vertices()) {
            for (auto a : v.outgoingArcs()) {
                auto u = a.to();
                mat(v.data(), u.data()) = true;
            }
        }
        return mat;
    }

    template <typename T = ArcData>
        requires (!std::is_void_v<ArcData>)
    static auto fromAdjacencyMatrix(const T* matrix, std::size_t n)
        ->std::expected<DirectedGraph<VertexData, T>, GraphBuildingError>
    {
        if (!matrix) return std::unexpected(GraphBuildingError::NullPointer);
        if (n == 0) return std::unexpected(GraphBuildingError::ZeroSize);

        DirectedGraph<VertexData, ArcData> g;
        std::unordered_map<std::size_t,
                           typename DirectedGraph<VertexData, ArcData>::VertexDescriptor> ht;

        for (std::size_t i = 0; i < n; ++i) {
            auto desc = g.addVertex(static_cast<VertexData>(i));
            ht.emplace(i, desc);
        }

        for (std::size_t i = 0; i < n; ++i) {
            for (std::size_t j = 0; j < n; ++j) {
                auto val = matrix[i * n + j];
                if (val != T{})
                    g.addArc(ht[i], ht[j], val);
            }
        }

        return g;
    }

    template <typename T = ArcData>
        requires (!std::is_void_v<ArcData>)
    static auto fromAdjacencyMatrix(const std::vector<std::vector<T>>& matrix)
        ->std::expected<DirectedGraph<VertexData, T>, GraphBuildingError>
    {
        if (matrix.empty())
            return std::unexpected(GraphBuildingError::ZeroSize);

        std::size_t n = matrix.size();
        std::vector<T> flat;
        flat.reserve(n * n);
        for (const auto& row : matrix) {
            if (row.size() != n) return std::unexpected(GraphBuildingError::NonSquareMatrix);
            flat.insert(flat.end(), row.begin(), row.end());
        }
        return fromAdjacencyMatrix<T>(flat.data(), n);
    }

    static auto fromAdjacencyMatrix(const std::vector<bool>& matrix, std::size_t n)
        ->std::expected<DirectedGraph<VertexData, void>, GraphBuildingError>
        requires std::is_void_v<ArcData>
    {
        if (matrix.empty()) return std::unexpected(GraphBuildingError::EmptyVector);
        if (n == 0) return std::unexpected(GraphBuildingError::ZeroSize);

        DirectedGraph<VertexData, void> g;
        std::unordered_map<std::size_t,
                           typename DirectedGraph<VertexData, void>::VertexDescriptor> ht;

        for (std::size_t i = 0; i < n; ++i) {
            auto desc = g.addVertex(static_cast<VertexData>(i));
            ht.emplace(i, desc);
        }

        for (std::size_t i = 0; i < n; ++i) {
            for (std::size_t j = 0; j < n; ++j) {
                auto val = matrix[i * n + j];
                if (val) g.addArc(ht[i], ht[j]);
            }
        }

        return g;
    }

    static auto fromAdjacencyMatrix(const std::vector<std::vector<bool>>& matrix)
        ->std::expected<DirectedGraph<VertexData, void>, GraphBuildingError>
        requires std::is_void_v<ArcData>
    {
        if (matrix.empty())
            return std::unexpected(GraphBuildingError::ZeroSize);

        std::size_t n = matrix.size();
        std::vector<bool> flat;
        flat.reserve(n * n);
        for (const auto& row : matrix) {
            if (row.size() != n) return std::unexpected(GraphBuildingError::NonSquareMatrix);
            flat.insert(flat.end(), row.begin(), row.end());
        }
        return fromAdjacencyMatrix(flat, n);
    }

private:

    DirectedMultiGraph<VertexData, ArcData> _multiGraph;

};

} // namespace exx::incident

#endif // EXX_DIRECTEDGRAPH_HPP

/*** End of inlined file: DirectedGraph.hpp ***/

#endif // EXX_DIRECTED_HPP

/*** End of inlined file: directed.hpp ***/


/*** Start of inlined file: algorithms.hpp ***/
#ifndef EXX_ALGORITHMS_HPP
#define EXX_ALGORITHMS_HPP


/*** Start of inlined file: mstPrim.hpp ***/
#ifndef EXX_MSTPRIM_HPP
#define EXX_MSTPRIM_HPP

#include <queue>
#include <unordered_set>
#include <vector>
#include <expected>


/*** Start of inlined file: graph_concepts.hpp ***/
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

/*** End of inlined file: graph_concepts.hpp ***/

namespace exx::incident {

template<UndirectedGraphConcept G>
    requires std::is_copy_constructible_v<typename G::EdgeValueType>
auto mstPrim(const G& graph)
    -> std::expected<UndirectedGraph<typename G::VertexValueType,
                                     typename G::EdgeValueType>, PrimError>
{
    using GraphType = UndirectedGraph<typename G::VertexValueType,
                                      typename G::EdgeValueType>;
    using ConstVertexDesc = typename GraphType::ConstVertexDescriptor;
    using VertexDesc = typename GraphType::VertexDescriptor;

    if (graph.vertexCount() == 0) return GraphType{};

    GraphType mst;
    std::unordered_map<ConstVertexDesc, VertexDesc> newDescOf;

    for (auto v : graph.constVertices()) {
        VertexDesc newDesc = mst.addVertex(v.data());
        newDescOf[v] = newDesc;
    }

    struct QueueElement {
        typename G::EdgeValueType weight;
        ConstVertexDesc vertex;
        ConstVertexDesc parent;
    };
    auto cmp = [](const QueueElement& a, const QueueElement& b) {
        return a.weight > b.weight;
    };
    std::priority_queue<QueueElement,
                        std::vector<QueueElement>,
                        decltype(cmp)> pq(cmp);

    std::unordered_set<ConstVertexDesc> visited;

    ConstVertexDesc startDesc = *graph.constVertices().begin();
    visited.insert(startDesc);

    for (auto edge : startDesc.incidentEdges()) {
        auto otherDesc = edge.otherEnd(startDesc); // always valid
        if (!visited.contains(*otherDesc))
            pq.push({ edge.data(), *otherDesc, startDesc });
    }

    while (!pq.empty()) {
        auto [weight, vDesc, parentDesc] = pq.top();
        pq.pop();

        if (visited.contains(vDesc)) continue;
        visited.insert(vDesc);

        mst.addEdge(newDescOf[parentDesc], newDescOf[vDesc], weight);

        for (auto edge : vDesc.incidentEdges()) {
            auto otherDesc = edge.otherEnd(vDesc); // always valid
            if (!visited.contains(*otherDesc))
                pq.push({ edge.data(), *otherDesc, vDesc });
        }
    }

    if (visited.size() != graph.vertexCount())
        return std::unexpected(PrimError::DisconnectedGraph);

    return mst;
}

} // namespace exx::incident

#endif // EXX_MSTPRIM_HPP

/*** End of inlined file: mstPrim.hpp ***/


/*** Start of inlined file: bfs.hpp ***/
#ifndef EXX_BFS_HPP
#define EXX_BFS_HPP

#include <queue>
#include <unordered_set>

namespace exx::incident {

template<GraphConcept Graph,
         typename Cmp = std::less<typename Graph::VertexValueType>>
auto bfs(Graph& G, typename Graph::VertexDescriptor start)
    ->std::vector<typename Graph::VertexDescriptor>
{
    using Descriptor = typename Graph::VertexDescriptor;

    std::queue<Descriptor> queue;
    std::unordered_set<Descriptor> visited;
    std::vector<Descriptor> res;

    queue.push(start);
    visited.insert(start);

    while(!queue.empty()) {
        auto current = queue.front();
        queue.pop();

        res.push_back(current);

        for(auto adjV : current.template adjacentVertices<Cmp>()) {
            if(!visited.contains(adjV)) {
                queue.push(adjV);
                visited.insert(adjV);
            }
        }
    }

    return res;
}

} // namespace exx::incident

#endif // EXX_BFS_HPP

/*** End of inlined file: bfs.hpp ***/


/*** Start of inlined file: dfs.hpp ***/
#ifndef EXX_DFS_HPP
#define EXX_DFS_HPP

#include <vector>
#include <stack>
#include <unordered_set>

namespace exx::incident {

template<GraphConcept Graph,
         typename Cmp = std::less<typename Graph::VertexValueType>>
auto dfs(Graph& G, typename Graph::VertexDescriptor start)
    ->std::vector<typename Graph::VertexDescriptor>
{
    using Descriptor = typename Graph::VertexDescriptor;

    std::stack<Descriptor> stack;
    std::unordered_set<Descriptor> visited;
    std::vector<Descriptor> res;

    stack.push(start);

    while(!stack.empty()) {
        auto current = stack.top();
        stack.pop();

        if(!visited.contains(current)) {
            visited.insert(current);

            res.push_back(current);

            for(auto adjV : current.template adjacentVertices<Cmp>())
                if(!visited.contains(adjV)) stack.push(adjV);
        }
    }

    return res;
}

} // namespace exx::incident

#endif // EXX_DFS_HPP

/*** End of inlined file: dfs.hpp ***/

#endif // EXX_ALGORITHMS_HPP

/*** End of inlined file: algorithms.hpp ***/


/*** Start of inlined file: utility.hpp ***/
#ifndef EXX_UTILITY_HPP
#define EXX_UTILITY_HPP


/*** Start of inlined file: UniqueVertexIndexedView.hpp ***/
#ifndef EXX_UNIQUEVERTEXINDEXEDVIEW_HPP
#define EXX_UNIQUEVERTEXINDEXEDVIEW_HPP

#include <unordered_map>
#include <optional>

namespace exx::incident {

template<GraphConcept Graph,
         typename Hash = std::hash<typename Graph::VertexValueType>,
         typename KeyEqual = std::equal_to<typename Graph::VertexValueType>>
class UniqueVertexIndexedView {
public:
    using VertexValueType = typename Graph::VertexValueType;
    using VertexDescriptor = typename Graph::VertexDescriptor;

    explicit UniqueVertexIndexedView(Graph& graph) : graph_(graph)
    { rebuild(); }

    void rebuild() {
        index_.clear();
        for (auto v : graph_.vertices())
            index_.emplace(v.data(), v);
    }

    template<typename... Args>
    VertexDescriptor emlaceVertex(Args&&... args) {
        auto v = graph_.emplaceVertex(std::forward<Args>(args)...);
        index_.emplace(v.data(), v);
        return v;
    }

    VertexDescriptor addVertex(const Graph::VertexValueType& data)
    { return emplaceVertex(data); }

    void removeVertex(VertexDescriptor v) {
        index_.erase(v.data());
        graph_.removeVertex(v);
    }

    void setVertexData(VertexDescriptor v, const VertexValueType& newData) {
        index_.erase(v.data());
        v.data() = newData;
        index_.emplace(newData, v);
    }

    void setVertexData(VertexDescriptor v, VertexValueType&& newData) {
        index_.erase(v.data());
        v.data() = std::move(newData);
        index_.emplace(v.data(), v);
    }

    std::optional<VertexDescriptor> findVertex(const VertexValueType& key) const {
        auto it = index_.find(key);
        if (it != index_.end()) return it->second;
        return std::nullopt;
    }

    bool containsVertex(const VertexValueType& key) const
    { return findVertex(key).has_value(); }

    auto begin() const { return index_.begin(); }
    auto end() const   { return index_.end(); }

    size_t size() const { return index_.size(); }
    bool empty() const  { return index_.empty(); }

    Graph& graph() { return graph_; }

private:
    Graph& graph_;
    std::unordered_map<VertexValueType, VertexDescriptor, Hash, KeyEqual> index_;
};

} // namespace exx::incident

#endif // EXX_UNIQUEVERTEXINDEXEDVIEW_HPP

/*** End of inlined file: UniqueVertexIndexedView.hpp ***/

#endif // EXX_UTILITY_HPP

/*** End of inlined file: utility.hpp ***/

#endif // EXX_INCIDENT_HPP

