#ifndef EXX_DIRECTEDABSTRACTGRAPH_HPP
#define EXX_DIRECTEDABSTRACTGRAPH_HPP

#include <list>
#include <ranges>

namespace exx::incident {

template<typename VertexData, typename ArcData>
class DirectedAbstractGraph {
private:
    struct _Vertex;
    struct _Arc;

    using _VertexList       = std::list<_Vertex>;
    using _ArcList          = std::list<_Arc>;
    using _VertexLabel      = typename _VertexList::iterator;
    using _VertexConstLabel = typename _VertexList::const_iterator;
    using _ArcLabel         = typename _ArcList::iterator;
    using _ArcConstLabel    = typename _ArcList::const_iterator;

    struct _EmptyArcData final {};
    using _ArcData = std::conditional_t<std::is_void_v<ArcData>, _EmptyArcData, ArcData>;

    struct _EraseAccelerationMetaData {
        typename std::list<_ArcLabel>::iterator _posInFrom{};
    };

    struct _Arc {
        _VertexLabel _from;
        _VertexLabel _to;

        [[no_unique_address]]
        _ArcData _data;

        _EraseAccelerationMetaData _meta{};

        // Конструктор для случая, когда ArcData != void (принимает данные)
        template<typename... Args>
            requires (!std::is_void_v<ArcData>)
        _Arc(_VertexLabel from, _VertexLabel to, Args&&... args)
            : _from(from), _to(to), _data(std::forward<Args>(args)...) {}

        // Конструктор для случая ArcData == void (без данных)
        _Arc(_VertexLabel from, _VertexLabel to)
            requires (std::is_void_v<ArcData>)
            : _from(from), _to(to) {}
    };

    struct _Vertex {
        std::list<_ArcLabel> _adjacentArcs;
        VertexData _data;

        template<typename... Args>
        explicit _Vertex(Args&&... args) : _data(std::forward<Args>(args)...) {}
    };

    _VertexList _vertices;
    _ArcList    _arcs;

    template<bool isConst> class _VertexIteratorImpl;
    template<bool isConst> class _ArcIteratorImpl;

    template<bool isConst>
    class _ArcProxy {
    private:
        using _ConditionalArcLabel = std::conditional_t<isConst, _ArcConstLabel, _ArcLabel>;
        _ConditionalArcLabel _label;

    public:
        explicit _ArcProxy(_ConditionalArcLabel label) : _label(label) {}

        using _ConditionalVertexIterator = std::conditional_t<isConst,
                                                              _VertexIteratorImpl<true>,
                                                              _VertexIteratorImpl<false>>;

        _ConditionalVertexIterator from() const { return _ConditionalVertexIterator(_label->_from); }
        _ConditionalVertexIterator to()   const { return _ConditionalVertexIterator(_label->_to); }

        auto& data()
            requires (!std::is_void_v<ArcData>)
        { return _label->_data; }

        const auto& data() const
            requires (!std::is_void_v<ArcData>)
        { return _label->_data; }
    };

    template<bool isConst>
    class _VertexProxy {
    private:
        using _ConditionalVertexLabel = std::conditional_t<isConst, _VertexConstLabel, _VertexLabel>;
        _ConditionalVertexLabel _label;

    public:
        explicit _VertexProxy(_ConditionalVertexLabel label) : _label(label) {}

        using _ConditionalDataRef = std::conditional_t<isConst, const VertexData&, VertexData&>;
        _ConditionalDataRef data() const { return _label->_data; }

        auto adjacentArcs() const {
            return std::views::transform(_label->_adjacentArcs, [](const _ArcLabel& al) {
                return _ArcProxy<isConst>(al);
            });
        }
    };

    template<bool isConst>
    class _VertexIteratorImpl {
    private:
        using _ConditionalVertexLabel = std::conditional_t<isConst, _VertexConstLabel, _VertexLabel>;
        _ConditionalVertexLabel _it;
        friend class DirectedAbstractGraph;

    public:
        using value_type = _VertexProxy<isConst>;
        using reference  = value_type;
        using pointer    = void;
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;

        _VertexIteratorImpl() = default;
        explicit _VertexIteratorImpl(_ConditionalVertexLabel it) : _it(it) {}

        value_type operator*() const { return value_type(_it); }
        _VertexIteratorImpl& operator++() { ++_it; return *this; }
        _VertexIteratorImpl operator++(int) { auto tmp = *this; ++(*this); return tmp; }

        bool operator==(const _VertexIteratorImpl& other) const { return _it == other._it; }
        bool operator!=(const _VertexIteratorImpl& other) const { return !(*this == other); }
    };

    template<bool isConst>
    class _ArcIteratorImpl {
    private:
        using _ConditionalArcLabel = std::conditional_t<isConst, _ArcConstLabel, _ArcLabel>;
        _ConditionalArcLabel _it;
        friend class DirectedAbstractGraph;

    public:
        using value_type = _ArcProxy<isConst>;
        using reference  = value_type;
        using pointer    = void;
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;

        _ArcIteratorImpl() = default;
        explicit _ArcIteratorImpl(_ConditionalArcLabel it) : _it(it) {}

        value_type operator*() const { return value_type(_it); }
        _ArcIteratorImpl& operator++() { ++_it; return *this; }
        _ArcIteratorImpl operator++(int) { auto tmp = *this; ++(*this); return tmp; }

        bool operator==(const _ArcIteratorImpl& other) const { return _it == other._it; }
        bool operator!=(const _ArcIteratorImpl& other) const { return !(*this == other); }
    };

public:
    using VertexIterator      = _VertexIteratorImpl<false>;
    using ConstVertexIterator = _VertexIteratorImpl<true>;
    using ArcIterator         = _ArcIteratorImpl<false>;
    using ConstArcIterator    = _ArcIteratorImpl<true>;
    using VertexProxy         = _VertexProxy<false>;
    using ConstVertexProxy    = _VertexProxy<true>;
    using ArcProxy            = _ArcProxy<false>;
    using ConstArcProxy       = _ArcProxy<true>;

    // ----- Вершины -----
    template<typename... Args>
    VertexIterator emplaceVertex(Args&&... args) {
        _vertices.emplace_back(std::forward<Args>(args)...);
        auto it = _vertices.end();
        --it;
        return VertexIterator(it);
    }

    VertexIterator addVertex(const VertexData& data) { return emplaceVertex(data); }

    void removeVertex(VertexIterator vertexIt) {
        for (auto arcIt = _arcs.begin(); arcIt != _arcs.end(); ) {
            if (arcIt->_from == vertexIt._it || arcIt->_to == vertexIt._it) {
                arcIt->_from->_adjacentArcs.remove(arcIt);
                arcIt = _arcs.erase(arcIt);
            } else {
                ++arcIt;
            }
        }
        _vertices.erase(vertexIt._it);
    }

    // ----- Дуги -----
    // Вариант для ArcData != void
    template<typename... Args>
        requires (!std::is_void_v<ArcData>)
    ArcIterator emplaceArc(VertexIterator from, VertexIterator to, Args&&... args) {
        _arcs.emplace_back(from._it, to._it, std::forward<Args>(args)...);
        auto it = _arcs.end();
        --it;
        from._it->_adjacentArcs.push_back(it);
        it->_meta._posInFrom = std::prev(from._it->_adjacentArcs.end());
        return ArcIterator(it);
    }

    // addArc с данными (если ArcData не void)
    ArcIterator addArc(VertexIterator from,
                       VertexIterator to,
                       const ArcData& data)
        requires (!std::is_void_v<ArcData>)
    { return emplaceArc(from, to, data); }

    // addArc без данных (только для ArcData == void)
    ArcIterator addArc(VertexIterator from, VertexIterator to)
        requires (std::is_void_v<ArcData>)
    {
        _arcs.emplace_back(from._it, to._it);
        auto it = _arcs.end();
        --it;
        from._it->_adjacentArcs.push_back(it);
        it->_meta._posInFrom = std::prev(from._it->_adjacentArcs.end());
        return ArcIterator(it);
    }

    void removeArc(ArcIterator arcIt) {
        auto& arc = *arcIt._it;
        arc._from->_adjacentArcs.erase(arc._meta._posInFrom);
        _arcs.erase(arcIt._it);
    }

    // ----- Итераторы и обходы -----
    VertexIterator      beginVertices()        { return VertexIterator(_vertices.begin()); }
    VertexIterator      endVertices()          { return VertexIterator(_vertices.end()); }
    ConstVertexIterator beginVertices()  const { return ConstVertexIterator(_vertices.begin()); }
    ConstVertexIterator endVertices()    const { return ConstVertexIterator(_vertices.end()); }
    ConstVertexIterator cbeginVertices() const { return ConstVertexIterator(_vertices.begin()); }
    ConstVertexIterator cendVertices()   const { return ConstVertexIterator(_vertices.end()); }

    auto vertices()            { return std::ranges::subrange(beginVertices(), endVertices()); }
    auto vertices()      const { return std::ranges::subrange(beginVertices(), endVertices()); }
    auto constVertices() const { return std::ranges::subrange(cbeginVertices(), cendVertices()); }

    ArcIterator      beginArcs()        { return ArcIterator(_arcs.begin()); }
    ArcIterator      endArcs()          { return ArcIterator(_arcs.end()); }
    ConstArcIterator beginArcs()  const { return ConstArcIterator(_arcs.begin()); }
    ConstArcIterator endArcs()    const { return ConstArcIterator(_arcs.end()); }
    ConstArcIterator cbeginArcs() const { return ConstArcIterator(_arcs.begin()); }
    ConstArcIterator cendArcs()   const { return ConstArcIterator(_arcs.end()); }

    auto arcs()            { return std::ranges::subrange(beginArcs(), endArcs()); }
    auto arcs()      const { return std::ranges::subrange(beginArcs(), endArcs()); }
    auto constArcs() const { return std::ranges::subrange(cbeginArcs(), cendArcs()); }

    std::size_t vertexCount() const { return _vertices.size(); }
    std::size_t arcCount() const { return _arcs.size(); }
};

} // namespace exx::incident

#endif // EXX_DIRECTEDABSTRACTGRAPH_HPP
