#ifndef EXX_UNDIRECTEDABSTRACTGRAPH_HPP
#define EXX_UNDIRECTEDABSTRACTGRAPH_HPP

#include <list>
#include <ranges>

namespace exx::incident {

template<typename VertexData, typename EdgeData>
class UndirectedAbstractGraph {
private:
    struct _Vertex;
    struct _Edge;

    using _VertexList       = std::list<_Vertex>;
    using _EdgeList         = std::list<_Edge>;
    using _VertexLabel      = typename _VertexList::iterator;
    using _VertexConstLabel = typename _VertexList::const_iterator;
    using _EdgeLabel        = typename _EdgeList::iterator;
    using _EdgeConstLabel   = typename _EdgeList::const_iterator;

    struct _EmptyEdgeData final {};
    using _EdgeData = std::conditional_t<std::is_void_v<EdgeData>, _EmptyEdgeData, EdgeData>;

    struct _EraseAccelerationMetaData {
        typename std::list<_EdgeLabel>::iterator _posInV1{};
        typename std::list<_EdgeLabel>::iterator _posInV2{};
    };

    struct _Edge {
        _VertexLabel _v1;
        _VertexLabel _v2;

        [[no_unique_address]]
        _EdgeData _data;

        _EraseAccelerationMetaData _meta{};   // метаданные заполняются ПОСЛЕ вставки

        template<typename... Args>
            requires (!std::is_void_v<EdgeData>)
        _Edge(_VertexLabel v1, _VertexLabel v2, Args&&... args) :
            _v1(v1),
            _v2(v2),
            _data(std::forward<Args>(args)...) {}

        _Edge(_VertexLabel v1, _VertexLabel v2)
            requires (std::is_void_v<EdgeData>)
            : _v1(v1), _v2(v2) {}
    };

    struct _Vertex {
        std::list<_EdgeLabel> _incidentEdges;
        VertexData _data;

        template<typename... Args>
        explicit _Vertex(Args&&... args) : _data(std::forward<Args>(args)...) {}
    };

    _VertexList _vertices;
    _EdgeList   _edges;

    template<bool isConst> class _VertexIteratorImpl;
    template<bool isConst> class _EdgeIteratorImpl;

    template<bool isConst>
    class _EdgeProxy {
    private:
        using _ConditionalEdgeLabel = std::conditional_t<isConst,
                                                         _EdgeConstLabel,
                                                         _EdgeLabel>;

        _ConditionalEdgeLabel _label;

    public:
        explicit _EdgeProxy(_ConditionalEdgeLabel label) : _label(label) {}

        using _ConditionalVertexIterator = std::conditional_t<isConst,
                                                              _VertexIteratorImpl<true>,
                                                              _VertexIteratorImpl<false>>;

        _ConditionalVertexIterator v1() const { return _ConditionalVertexIterator(_label->_v1); }
        _ConditionalVertexIterator v2() const { return _ConditionalVertexIterator(_label->_v2); }

        _ConditionalVertexIterator otherEnd(_ConditionalVertexIterator vertex) const {
            if (vertex._it == _label->_v1) return _ConditionalVertexIterator(_label->_v2);
            else if (vertex._it == _label->_v2) return _ConditionalVertexIterator(_label->_v1);
            else throw std::invalid_argument("Vertex is not incident to this edge");
        }

        auto& data()
            requires (!std::is_void_v<EdgeData>)
        { return _label->_data; }

        const auto& data() const
            requires (!std::is_void_v<EdgeData>)
        { return _label->_data; }
    };

    template<bool isConst>
    class _VertexProxy {
    private:
        using _ConditionalVertexLabel = std::conditional_t<isConst,
                                                           _VertexConstLabel,
                                                           _VertexLabel>;

        _ConditionalVertexLabel _label;

    public:
        explicit _VertexProxy(_ConditionalVertexLabel label) : _label(label) {}

        using _ConditionalDataRef = std::conditional_t<isConst,
                                                       const VertexData&,
                                                       VertexData&>;

        _ConditionalDataRef data() const { return _label->_data; }

        auto incidentEdges() const {
            return std::views::transform(_label->_incidentEdges, [](const _EdgeLabel& el) {
                return _EdgeProxy<isConst>(el);
            });
        }
    };

    template<bool isConst>
    class _VertexIteratorImpl {
    private:
        using _ConditionalVertexLabel = std::conditional_t<isConst,
                                                           _VertexConstLabel,
                                                           _VertexLabel>;

        _ConditionalVertexLabel _it;

        friend class UndirectedAbstractGraph;

    public:
        using value_type = _VertexProxy<isConst>;
        using reference  = value_type;
        using pointer    = void;
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;

        _VertexIteratorImpl() = default;
        _VertexIteratorImpl(const _VertexIteratorImpl&) = default;
        _VertexIteratorImpl& operator=(const _VertexIteratorImpl&) = default;

        _VertexIteratorImpl(const _VertexIteratorImpl<false>& other) requires isConst
           : _it(other._it) {}

        explicit _VertexIteratorImpl(_ConditionalVertexLabel it) : _it(it) {}

        value_type operator*() const { return value_type(_it); }
        _VertexIteratorImpl& operator++() { ++_it; return *this; }
        _VertexIteratorImpl operator++(int) { auto tmp = *this; ++(*this); return tmp; }

        bool operator==(const _VertexIteratorImpl& other) const { return _it == other._it; }
        bool operator!=(const _VertexIteratorImpl& other) const { return !(*this == other); }
    };

    template<bool isConst>
    class _EdgeIteratorImpl {
    private:
        using _ConditionalEdgeLabel = std::conditional_t<isConst,
                                                         _EdgeConstLabel,
                                                         _EdgeLabel>;

        _ConditionalEdgeLabel _it;

        friend class UndirectedAbstractGraph;

    public:
        using value_type = _EdgeProxy<isConst>;
        using reference  = value_type;
        using pointer    = void;
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;

        _EdgeIteratorImpl() = default;
        _EdgeIteratorImpl(const _EdgeIteratorImpl&) = default;
        _EdgeIteratorImpl& operator=(const _EdgeIteratorImpl&) = default;

        _EdgeIteratorImpl(const _EdgeIteratorImpl<false>& other) requires isConst
            : _it(other._it) {}

        explicit _EdgeIteratorImpl(_ConditionalEdgeLabel it) : _it(it) {}

        value_type operator*() const { return value_type(_it); }
        _EdgeIteratorImpl& operator++() { ++_it; return *this; }
        _EdgeIteratorImpl operator++(int) { auto tmp = *this; ++(*this); return tmp; }

        bool operator==(const _EdgeIteratorImpl& other) const { return _it == other._it; }
        bool operator!=(const _EdgeIteratorImpl& other) const { return !(*this == other); }
    };

public:
    using VertexIterator      = _VertexIteratorImpl<false>;
    using ConstVertexIterator = _VertexIteratorImpl<true>;
    using EdgeIterator        = _EdgeIteratorImpl<false>;
    using ConstEdgeIterator   = _EdgeIteratorImpl<true>;

    using VertexProxy = _VertexProxy<false>;
    using ConstVertexProxy = _VertexProxy<true>;
    using EdgeProxy = _EdgeProxy<false>;
    using ConstEdgeProxy = _EdgeProxy<true>;

    template<typename... Args>
    VertexIterator emplaceVertex(Args&&... args) {
        _vertices.emplace_back(std::forward<Args>(args)...);
        auto it = _vertices.end();
        --it;
        return VertexIterator(it);
    }

    VertexIterator addVertex(const VertexData& data) { return emplaceVertex(data); }

    void removeVertex(ConstVertexIterator vertexIt) {
        auto incidentCopy = vertexIt._it->_incidentEdges;
        for (auto edgeIt : incidentCopy) {
            removeEdge(EdgeIterator(edgeIt));
        }
        _vertices.erase(vertexIt._it);
    }

    template<typename... Args>
        requires (!std::is_void_v<EdgeData>)
    EdgeIterator emplaceEdge(VertexIterator from, VertexIterator to, Args&&... args) {
        _edges.emplace_back(from._it, to._it, std::forward<Args>(args)...);
        auto edgeIt = std::prev(_edges.end());

        from._it->_incidentEdges.push_back(edgeIt);
        to._it->_incidentEdges.push_back(edgeIt);

        edgeIt->_meta._posInV1 = std::prev(from._it->_incidentEdges.end());
        edgeIt->_meta._posInV2 = std::prev(to._it->_incidentEdges.end());

        return EdgeIterator(edgeIt);
    }

    EdgeIterator addEdge(VertexIterator from,
                         VertexIterator to,
                         const EdgeData& data)
        requires (!std::is_void_v<EdgeData>)
    { return emplaceEdge(from, to, data); }

    EdgeIterator addEdge(VertexIterator from, VertexIterator to)
        requires (std::is_void_v<EdgeData>)
    {
        _edges.emplace_back(from._it, to._it);
        auto edgeIt = std::prev(_edges.end());

        from._it->_incidentEdges.push_back(edgeIt);
        to._it->_incidentEdges.push_back(edgeIt);

        edgeIt->_meta._posInV1 = std::prev(from._it->_incidentEdges.end());
        edgeIt->_meta._posInV2 = std::prev(to._it->_incidentEdges.end());

        return EdgeIterator(edgeIt);
    }

    void removeEdge(EdgeIterator edgeIt) {
        auto& e = *edgeIt._it;
        e._v1->_incidentEdges.erase(e._meta._posInV1);
        e._v2->_incidentEdges.erase(e._meta._posInV2);
        _edges.erase(edgeIt._it);
    }

    VertexIterator      beginVertices()        { return VertexIterator(_vertices.begin()); }
    VertexIterator      endVertices()          { return VertexIterator(_vertices.end()); }
    ConstVertexIterator beginVertices()  const { return ConstVertexIterator(_vertices.begin()); }
    ConstVertexIterator endVertices()    const { return ConstVertexIterator(_vertices.end()); }
    ConstVertexIterator cbeginVertices() const { return ConstVertexIterator(_vertices.begin()); }
    ConstVertexIterator cendVertices()   const { return ConstVertexIterator(_vertices.end()); }

    auto vertices()            { return std::ranges::subrange(beginVertices(), endVertices()); }
    auto vertices()      const { return std::ranges::subrange(beginVertices(), endVertices()); }
    auto constVertices() const { return std::ranges::subrange(cbeginVertices(), cendVertices()); }

    EdgeIterator      beginEdges()        { return EdgeIterator(_edges.begin()); }
    EdgeIterator      endEdges()          { return EdgeIterator(_edges.end()); }
    ConstEdgeIterator beginEdges()  const { return ConstEdgeIterator(_edges.begin()); }
    ConstEdgeIterator endEdges()    const { return ConstEdgeIterator(_edges.end()); }
    ConstEdgeIterator cbeginEdges() const { return ConstEdgeIterator(_edges.begin()); }
    ConstEdgeIterator cendEdges()   const { return ConstEdgeIterator(_edges.end()); }

    auto edges()            { return std::ranges::subrange(beginEdges(), endEdges()); }
    auto edges()      const { return std::ranges::subrange(beginEdges(), endEdges()); }
    auto constEdges() const { return std::ranges::subrange(cbeginEdges(), cendEdges()); }

    std::size_t vertexCount() const { return _vertices.size(); }
    std::size_t edgeCount() const { return _edges.size(); }
};

} // namespace exx::incident

#endif // EXX_UNDIRECTEDABSTRACTGRAPH_HPP
