#ifndef EXX_UNDIRECTEDABSTRACTGRAPH_HPP
#define EXX_UNDIRECTEDABSTRACTGRAPH_HPP

#include <list>
#include <ranges>
#include <type_traits>
#include <optional>
#include <unordered_map>

namespace exx::incident {

template<typename VertexData, typename EdgeData>
class UndirectedAbstractGraph {
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
    using ConditionalEdgeData = std::conditional_t<std::is_void_v<EdgeData>, EmptyEdgeData, EdgeData>;

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
            requires (!std::is_void_v<EdgeData>)
        Edge(VertexLabel v1, VertexLabel v2, Args&&... args)
            : _v1(v1), _v2(v2), _data(std::forward<Args>(args)...) {}

        Edge(VertexLabel v1, VertexLabel v2)
            requires (std::is_void_v<EdgeData>)
            : _v1(v1), _v2(v2) {}
    };

    struct Vertex {
        std::list<EdgeLabel> _incidentEdges;
        VertexData _data;
        template<typename... Args>
        explicit Vertex(Args&&... args) : _data(std::forward<Args>(args)...) {}
    };

    VertexList _vertices;
    EdgeList   _edges;

    template<bool isConst>
    class VertexDescriptorImpl {
    private:
        using ConditionalVertexLabel = std::conditional_t<isConst, VertexConstLabel, VertexLabel>;

        ConditionalVertexLabel _label;

        friend class UndirectedAbstractGraph;

    public:
        VertexDescriptorImpl() = default;
        VertexDescriptorImpl(const VertexDescriptorImpl&) = default;
        VertexDescriptorImpl& operator=(const VertexDescriptorImpl&) = default;

        explicit VertexDescriptorImpl(ConditionalVertexLabel label) : _label(label) {}

        VertexDescriptorImpl(const VertexDescriptorImpl<false>& other) requires isConst
            : _label(other._label) {}

        auto& data()       requires (!isConst) { return _label->_data; }
        const auto& data() const               { return _label->_data; }

        auto incidentEdges() const {
            return std::views::transform(_label->_incidentEdges,
                                         [](const EdgeLabel& el) { return EdgeDescriptorImpl<isConst>(el); });
        }

        bool operator==(const VertexDescriptorImpl& other) const { return _label == other._label; }
        bool operator!=(const VertexDescriptorImpl& other) const { return !(*this == other); }
    };

    template<bool isConst>
    class EdgeDescriptorImpl {
    private:
        using ConditionalEdgeLabel = std::conditional_t<isConst, EdgeConstLabel, EdgeLabel>;

        ConditionalEdgeLabel _label;

        friend class UndirectedAbstractGraph;

    public:
        EdgeDescriptorImpl() = default;
        EdgeDescriptorImpl(const EdgeDescriptorImpl&) = default;
        EdgeDescriptorImpl& operator=(const EdgeDescriptorImpl&) = default;

        explicit EdgeDescriptorImpl(ConditionalEdgeLabel label) : _label(label) {}

        EdgeDescriptorImpl(const EdgeDescriptorImpl<false>& other) requires isConst
            : _label(other._label) {}

        auto& data()       requires (!std::is_void_v<EdgeData> && !isConst) { return _label->_data; }
        const auto& data() const requires (!std::is_void_v<EdgeData>) { return _label->_data; }

        VertexDescriptorImpl<isConst> v1() const { return VertexDescriptorImpl<isConst>(_label->_v1); }
        VertexDescriptorImpl<isConst> v2() const { return VertexDescriptorImpl<isConst>(_label->_v2); }

        std::optional<VertexDescriptorImpl<isConst>> otherEnd(const VertexDescriptorImpl<isConst>& vertex) const {
            if (vertex._label == _label->_v1)
                return VertexDescriptorImpl<isConst>(_label->_v2);
            else if (vertex._label == _label->_v2)
                return VertexDescriptorImpl<isConst>(_label->_v1);
            else
                return std::nullopt;
        }

        bool operator==(const EdgeDescriptorImpl& other) const { return _label == other._label; }
        bool operator!=(const EdgeDescriptorImpl& other) const { return !(*this == other); }
    };

    template<bool isConst>
    class VertexIteratorImpl {
    private:
        using ConditionalVertexLabel = std::conditional_t<isConst, VertexConstLabel, VertexLabel>;

        ConditionalVertexLabel _it;

        friend class UndirectedAbstractGraph;

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
        using ConditionalEdgeLabel = std::conditional_t<isConst, EdgeConstLabel, EdgeLabel>;

        ConditionalEdgeLabel _it;

        friend class UndirectedAbstractGraph;

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

    using VertexDescriptor      = VertexDescriptorImpl<false>;
    using ConstVertexDescriptor = VertexDescriptorImpl<true>;
    struct VertexDescriptorHash {
        template<bool isConst>
        std::size_t operator()(const VertexDescriptorImpl<isConst>& desc) const
        { return std::hash<const void*>()( &(*desc._label) ); }
    };

    using EdgeDescriptor        = EdgeDescriptorImpl<false>;
    using ConstEdgeDescriptor   = EdgeDescriptorImpl<true>;
    struct EdgeDescriptorHash {
        template<bool isConst>
        std::size_t operator()(const EdgeDescriptorImpl<isConst>& desc) const
        { return std::hash<const void*>()( &(*desc._label) ); }
    };

    using VertexIterator      = VertexIteratorImpl<false>;
    using ConstVertexIterator = VertexIteratorImpl<true>;
    using EdgeIterator        = EdgeIteratorImpl<false>;
    using ConstEdgeIterator   = EdgeIteratorImpl<true>;

    UndirectedAbstractGraph() = default;

    UndirectedAbstractGraph(const UndirectedAbstractGraph& other) {
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

    UndirectedAbstractGraph(UndirectedAbstractGraph&&) noexcept = default;

    UndirectedAbstractGraph& operator=(const UndirectedAbstractGraph& other) {
        if (this != &other) {
            UndirectedAbstractGraph temp(other);
            *this = std::move(temp);
        }
        return *this;
    }

    UndirectedAbstractGraph& operator=(UndirectedAbstractGraph&&) noexcept = default;

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
        requires (!std::is_void_v<EdgeData>)
    EdgeDescriptor emplaceEdge(VertexDescriptor from,
                               VertexDescriptor to,
                               Args&&... args)
    {
        _edges.emplace_back(from._label, to._label, std::forward<Args>(args)...);
        auto it = std::prev(_edges.end());
        from._label->_incidentEdges.push_back(it);
        to._label->_incidentEdges.push_back(it);
        it->_meta._posInV1 = std::prev(from._label->_incidentEdges.end());
        it->_meta._posInV2 = std::prev(to._label->_incidentEdges.end());
        return EdgeDescriptor(it);
    }

    template<typename T = EdgeData>
        requires (!std::is_void_v<EdgeData>)
    EdgeDescriptor addEdge(VertexDescriptor from, VertexDescriptor to, T&& data)
    { return emplaceEdge(from, to, std::forward<T>(data)); }

    EdgeDescriptor addEdge(VertexDescriptor from, VertexDescriptor to)
        requires (std::is_void_v<EdgeData>)
    {
        _edges.emplace_back(from._label, to._label);
        auto it = std::prev(_edges.end());
        from._label->_incidentEdges.push_back(it);
        to._label->_incidentEdges.push_back(it);
        it->_meta._posInV1 = std::prev(from._label->_incidentEdges.end());
        it->_meta._posInV2 = std::prev(to._label->_incidentEdges.end());
        return EdgeDescriptor(it);
    }

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

    std::optional<EdgeDescriptor> findEdge(VertexDescriptor from, VertexDescriptor to) {
        for (auto e : from.incidentEdges())
            if (*e.otherEnd(from) == to) return e;
        return std::nullopt;
    }

    std::optional<ConstEdgeDescriptor> findEdge(VertexDescriptor from, VertexDescriptor to) const {
        for (auto e : from.incidentEdges())
            if (*e.otherEnd(from) == to) return e;
        return std::nullopt;
    }

    bool hasEdge(VertexDescriptor from, VertexDescriptor to) const { return findEdge(from, to).has_value(); }
};

} // namespace exx::incident

#endif // EXX_UNDIRECTEDABSTRACTGRAPH_HPP
