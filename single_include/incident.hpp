#ifndef EXX_INCIDENT_HPP
#define EXX_INCIDENT_HPP

#ifndef EXX_DIRECTEDABSTRACTGRAPH_HPP
#define EXX_DIRECTEDABSTRACTGRAPH_HPP

#include <list>
#include <ranges>
#include <type_traits>

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
        [[no_unique_address]] _ArcData _data;
        _EraseAccelerationMetaData _meta{};

        template<typename... Args>
            requires (!std::is_void_v<ArcData>)
        _Arc(_VertexLabel from, _VertexLabel to, Args&&... args)
            : _from(from), _to(to), _data(std::forward<Args>(args)...) {}

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

    // ---------- Внутренние дескрипторы (без итераторных операций) ----------
    template<bool isConst>
    class _VertexDescriptor {
    private:
        using _ConditionalVertexLabel = std::conditional_t<isConst, _VertexConstLabel, _VertexLabel>;
        _ConditionalVertexLabel _label;

        friend class DirectedAbstractGraph;

    public:
        _VertexDescriptor() = default;
        _VertexDescriptor(const _VertexDescriptor&) = default;
        _VertexDescriptor& operator=(const _VertexDescriptor&) = default;

        explicit _VertexDescriptor(_ConditionalVertexLabel label) : _label(label) {}

        // Конвертация из неконстантного в константный
        _VertexDescriptor(const _VertexDescriptor<false>& other) requires isConst
            : _label(other._label) {}

        auto& data()       requires (!isConst) { return _label->_data; }
        const auto& data() const               { return _label->_data; }

        auto adjacentArcs() const {
            return std::views::transform(_label->_adjacentArcs,
                                         [](const _ArcLabel& al) { return _ArcDescriptor<isConst>(al); });
        }

        bool operator==(const _VertexDescriptor& other) const { return _label == other._label; }
        bool operator!=(const _VertexDescriptor& other) const { return !(*this == other); }
    };

    template<bool isConst>
    class _ArcDescriptor {
    private:
        using _ConditionalArcLabel = std::conditional_t<isConst, _ArcConstLabel, _ArcLabel>;
        _ConditionalArcLabel _label;

        friend class DirectedAbstractGraph;

    public:
        _ArcDescriptor() = default;
        _ArcDescriptor(const _ArcDescriptor&) = default;
        _ArcDescriptor& operator = (const _ArcDescriptor&) = default;

        explicit _ArcDescriptor(_ConditionalArcLabel label) : _label(label) {}

        _ArcDescriptor(const _ArcDescriptor<false>& other) requires isConst
            : _label(other._label) {}

        auto& data()       requires (!std::is_void_v<ArcData> && !isConst) { return _label->_data; }
        const auto& data() const requires (!std::is_void_v<ArcData>) { return _label->_data; }

        _VertexDescriptor<isConst> from() const { return _VertexDescriptor<isConst>(_label->_from); }
        _VertexDescriptor<isConst> to()   const { return _VertexDescriptor<isConst>(_label->_to); }

        bool operator==(const _ArcDescriptor& other) const { return _label == other._label; }
        bool operator!=(const _ArcDescriptor& other) const { return !(*this == other); }
    };

    // ---------- Итераторы (только для обхода) ----------
    template<bool isConst>
    class _VertexIterator {
    private:
        using _ConditionalVertexLabel = std::conditional_t<isConst, _VertexConstLabel, _VertexLabel>;
        _ConditionalVertexLabel _it;

        friend class DirectedAbstractGraph;

    public:
        using difference_type       = std::ptrdiff_t;
        using value_type            = _VertexDescriptor<isConst>;
        using reference             = value_type;
        using pointer               = void;
        using iterator_category     = std::forward_iterator_tag;
        using iterator_concept      = std::forward_iterator_tag;

        _VertexIterator() = default;
        _VertexIterator(const _VertexIterator&) = default;
        _VertexIterator& operator = (const _VertexIterator&) = default;

        explicit _VertexIterator(_ConditionalVertexLabel it) : _it(it) {}

        // Конвертация из неконстантного в константный
        _VertexIterator(const _VertexIterator<false>& other) requires isConst
            : _it(other._it) {}

        reference operator*() const { return value_type(_it); }
        _VertexIterator& operator++() { ++_it; return *this; }
        _VertexIterator operator++(int) { auto tmp = *this; ++*this; return tmp; }

        bool operator==(const _VertexIterator& other) const { return _it == other._it; }
        bool operator!=(const _VertexIterator& other) const { return !(*this == other); }
    };

    template<bool isConst>
    class _ArcIterator {
    private:
        using _ConditionalArcLabel = std::conditional_t<isConst, _ArcConstLabel, _ArcLabel>;
        _ConditionalArcLabel _it;

        friend class DirectedAbstractGraph;

    public:
        using difference_type       = std::ptrdiff_t;
        using value_type            = _ArcDescriptor<isConst>;
        using reference             = value_type;
        using pointer               = void;
        using iterator_category     = std::forward_iterator_tag;
        using iterator_concept      = std::forward_iterator_tag;

        _ArcIterator() = default;
        _ArcIterator(const _ArcIterator&) = default;
        _ArcIterator& operator = (const _ArcIterator&) = default;

        explicit _ArcIterator(_ConditionalArcLabel it) : _it(it) {}

        _ArcIterator(const _ArcIterator<false>& other) requires isConst
            : _it(other._it) {}

        reference operator*() const { return value_type(_it); }
        _ArcIterator& operator++() { ++_it; return *this; }
        _ArcIterator operator++(int) { auto tmp = *this; ++*this; return tmp; }

        bool operator==(const _ArcIterator& other) const { return _it == other._it; }
        bool operator!=(const _ArcIterator& other) const { return !(*this == other); }
    };

public:
    // Публичные типы
    using VertexDescriptor      = _VertexDescriptor<false>;
    using ConstVertexDescriptor = _VertexDescriptor<true>;
    using ArcDescriptor         = _ArcDescriptor<false>;
    using ConstArcDescriptor    = _ArcDescriptor<true>;

    using VertexIterator      = _VertexIterator<false>;
    using ConstVertexIterator = _VertexIterator<true>;
    using ArcIterator         = _ArcIterator<false>;
    using ConstArcIterator    = _ArcIterator<true>;

    // ---------- Методы графа ----------
    template<typename... Args>
    VertexDescriptor emplaceVertex(Args&&... args) {
        _vertices.emplace_back(std::forward<Args>(args)...);
        auto it = std::prev(_vertices.end());
        return VertexDescriptor(it);
    }

    VertexDescriptor addVertex(const VertexData& data) { return emplaceVertex(data); }

    void removeVertex(VertexDescriptor vertex) {
        for (auto it = _arcs.begin(); it != _arcs.end(); ) {
            if (it->_from == vertex._label || it->_to == vertex._label) {
                // Удаляем дугу из списка смежности источника
                it->_from->_adjacentArcs.erase(it->_meta._posInFrom);
                it = _arcs.erase(it);
            } else {
                ++it;
            }
        }
        _vertices.erase(vertex._label);
    }

    template<typename... Args>
        requires (!std::is_void_v<ArcData>)
    ArcDescriptor emplaceArc(VertexDescriptor from, VertexDescriptor to, Args&&... args) {
        _arcs.emplace_back(from._label, to._label, std::forward<Args>(args)...);
        auto it = std::prev(_arcs.end());
        from._label->_adjacentArcs.push_back(it);
        it->_meta._posInFrom = std::prev(from._label->_adjacentArcs.end());
        return ArcDescriptor(it);
    }

    ArcDescriptor addArc(VertexDescriptor from, VertexDescriptor to, const ArcData& data)
        requires (!std::is_void_v<ArcData>)
    { return emplaceArc(from, to, data); }

    ArcDescriptor addArc(VertexDescriptor from, VertexDescriptor to)
        requires (std::is_void_v<ArcData>)
    {
        _arcs.emplace_back(from._label, to._label);
        auto it = std::prev(_arcs.end());
        from._label->_adjacentArcs.push_back(it);
        it->_meta._posInFrom = std::prev(from._label->_adjacentArcs.end());
        return ArcDescriptor(it);
    }

    void removeArc(ArcDescriptor arc) {
        arc._label->_from->_adjacentArcs.erase(arc._label->_meta._posInFrom);
        _arcs.erase(arc._label);
    }

    // Итераторы для обхода
    VertexIterator beginVertices() { return VertexIterator(_vertices.begin()); }
    VertexIterator endVertices()   { return VertexIterator(_vertices.end()); }
    ConstVertexIterator beginVertices() const { return ConstVertexIterator(_vertices.begin()); }
    ConstVertexIterator endVertices()   const { return ConstVertexIterator(_vertices.end()); }
    ConstVertexIterator cbeginVertices() const { return ConstVertexIterator(_vertices.begin()); }
    ConstVertexIterator cendVertices()   const { return ConstVertexIterator(_vertices.end()); }

    auto vertices() {
        return std::ranges::subrange<VertexIterator, VertexIterator>(beginVertices(), endVertices());
    }
    auto vertices() const {
        return std::ranges::subrange<ConstVertexIterator, ConstVertexIterator>(beginVertices(), endVertices());
    }
    auto constVertices() const {
        return std::ranges::subrange<ConstVertexIterator, ConstVertexIterator>(cbeginVertices(), cendVertices());
    }

    ArcIterator beginArcs() { return ArcIterator(_arcs.begin()); }
    ArcIterator endArcs()   { return ArcIterator(_arcs.end()); }
    ConstArcIterator beginArcs() const { return ConstArcIterator(_arcs.begin()); }
    ConstArcIterator endArcs()   const { return ConstArcIterator(_arcs.end()); }
    ConstArcIterator cbeginArcs() const { return ConstArcIterator(_arcs.begin()); }
    ConstArcIterator cendArcs()   const { return ConstArcIterator(_arcs.end()); }

    auto arcs() {
        return std::ranges::subrange<ArcIterator, ArcIterator>(beginArcs(), endArcs());
    }
    auto arcs() const {
        return std::ranges::subrange<ConstArcIterator, ConstArcIterator>(beginArcs(), endArcs());
    }
    auto constArcs() const {
        return std::ranges::subrange<ConstArcIterator, ConstArcIterator>(cbeginArcs(), cendArcs());
    }

    std::size_t vertexCount() const { return _vertices.size(); }
    std::size_t arcCount() const { return _arcs.size(); }
};

} // namespace exx::incident

#endif // EXX_DIRECTEDABSTRACTGRAPH_HPP

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
        [[no_unique_address]] _EdgeData _data;
        _EraseAccelerationMetaData _meta{};

        template<typename... Args>
            requires (!std::is_void_v<EdgeData>)
        _Edge(_VertexLabel v1, _VertexLabel v2, Args&&... args)
            : _v1(v1), _v2(v2), _data(std::forward<Args>(args)...) {}

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

    // ---------- Descriptors ----------
    template<bool isConst>
    class _VertexDescriptor {
    private:
        using _ConditionalVertexLabel = std::conditional_t<isConst, _VertexConstLabel, _VertexLabel>;
        _ConditionalVertexLabel _label;

        friend class UndirectedAbstractGraph;

    public:
        _VertexDescriptor() = default;
        _VertexDescriptor(const _VertexDescriptor&) = default;
        _VertexDescriptor& operator=(const _VertexDescriptor&) = default;

        explicit _VertexDescriptor(_ConditionalVertexLabel label) : _label(label) {}

        // Conversion from non-const to const
        _VertexDescriptor(const _VertexDescriptor<false>& other) requires isConst
            : _label(other._label) {}

        auto& data()       requires (!isConst) { return _label->_data; }
        const auto& data() const               { return _label->_data; }

        auto incidentEdges() const {
            return std::views::transform(_label->_incidentEdges,
                                         [](const _EdgeLabel& el) { return _EdgeDescriptor<isConst>(el); });
        }

        bool operator==(const _VertexDescriptor& other) const { return _label == other._label; }
        bool operator!=(const _VertexDescriptor& other) const { return !(*this == other); }
    };

    template<bool isConst>
    class _EdgeDescriptor {
    private:
        using _ConditionalEdgeLabel = std::conditional_t<isConst, _EdgeConstLabel, _EdgeLabel>;
        _ConditionalEdgeLabel _label;

        friend class UndirectedAbstractGraph;

    public:
        _EdgeDescriptor() = default;
        _EdgeDescriptor(const _EdgeDescriptor&) = default;
        _EdgeDescriptor& operator=(const _EdgeDescriptor&) = default;

        explicit _EdgeDescriptor(_ConditionalEdgeLabel label) : _label(label) {}

        _EdgeDescriptor(const _EdgeDescriptor<false>& other) requires isConst
            : _label(other._label) {}

        auto& data()       requires (!std::is_void_v<EdgeData> && !isConst) { return _label->_data; }
        const auto& data() const requires (!std::is_void_v<EdgeData>) { return _label->_data; }

        _VertexDescriptor<isConst> v1() const { return _VertexDescriptor<isConst>(_label->_v1); }
        _VertexDescriptor<isConst> v2() const { return _VertexDescriptor<isConst>(_label->_v2); }

        std::optional<_VertexDescriptor<isConst>> otherEnd(const _VertexDescriptor<isConst>& vertex) const {
            if (vertex._label == _label->_v1)
                return _VertexDescriptor<isConst>(_label->_v2);
            else if (vertex._label == _label->_v2)
                return _VertexDescriptor<isConst>(_label->_v1);
            else
                return std::nullopt;
        }

        bool operator==(const _EdgeDescriptor& other) const { return _label == other._label; }
        bool operator!=(const _EdgeDescriptor& other) const { return !(*this == other); }
    };

    // ---------- Iterators ----------
    template<bool isConst>
    class _VertexIteratorImpl {
    private:
        using _ConditionalVertexLabel = std::conditional_t<isConst, _VertexConstLabel, _VertexLabel>;
        _ConditionalVertexLabel _it;

        friend class UndirectedAbstractGraph;

    public:
        using difference_type       = std::ptrdiff_t;
        using value_type            = _VertexDescriptor<isConst>;
        using reference             = value_type;
        using pointer               = void;
        using iterator_category     = std::forward_iterator_tag;
        using iterator_concept      = std::forward_iterator_tag;

        _VertexIteratorImpl() = default;
        _VertexIteratorImpl(const _VertexIteratorImpl&) = default;
        _VertexIteratorImpl& operator=(const _VertexIteratorImpl&) = default;

        explicit _VertexIteratorImpl(_ConditionalVertexLabel it) : _it(it) {}

        _VertexIteratorImpl(const _VertexIteratorImpl<false>& other) requires isConst
            : _it(other._it) {}

        reference operator*() const { return value_type(_it); }
        _VertexIteratorImpl& operator++() { ++_it; return *this; }
        _VertexIteratorImpl operator++(int) { auto tmp = *this; ++*this; return tmp; }

        bool operator==(const _VertexIteratorImpl& other) const { return _it == other._it; }
        bool operator!=(const _VertexIteratorImpl& other) const { return !(*this == other); }
    };

    template<bool isConst>
    class _EdgeIteratorImpl {
    private:
        using _ConditionalEdgeLabel = std::conditional_t<isConst, _EdgeConstLabel, _EdgeLabel>;
        _ConditionalEdgeLabel _it;

        friend class UndirectedAbstractGraph;

    public:
        using difference_type       = std::ptrdiff_t;
        using value_type            = _EdgeDescriptor<isConst>;
        using reference             = value_type;
        using pointer               = void;
        using iterator_category     = std::forward_iterator_tag;
        using iterator_concept      = std::forward_iterator_tag;

        _EdgeIteratorImpl() = default;
        _EdgeIteratorImpl(const _EdgeIteratorImpl&) = default;
        _EdgeIteratorImpl& operator=(const _EdgeIteratorImpl&) = default;

        explicit _EdgeIteratorImpl(_ConditionalEdgeLabel it) : _it(it) {}

        _EdgeIteratorImpl(const _EdgeIteratorImpl<false>& other) requires isConst
            : _it(other._it) {}

        reference operator*() const { return value_type(_it); }
        _EdgeIteratorImpl& operator++() { ++_it; return *this; }
        _EdgeIteratorImpl operator++(int) { auto tmp = *this; ++*this; return tmp; }

        bool operator==(const _EdgeIteratorImpl& other) const { return _it == other._it; }
        bool operator!=(const _EdgeIteratorImpl& other) const { return !(*this == other); }
    };

public:
    // Public types
    using VertexDescriptor      = _VertexDescriptor<false>;
    using ConstVertexDescriptor = _VertexDescriptor<true>;
    using EdgeDescriptor        = _EdgeDescriptor<false>;
    using ConstEdgeDescriptor   = _EdgeDescriptor<true>;

    using VertexIterator      = _VertexIteratorImpl<false>;
    using ConstVertexIterator = _VertexIteratorImpl<true>;
    using EdgeIterator        = _EdgeIteratorImpl<false>;
    using ConstEdgeIterator   = _EdgeIteratorImpl<true>;

    UndirectedAbstractGraph() = default;

    // Конструктор копирования (глубокое копирование)
    UndirectedAbstractGraph(const UndirectedAbstractGraph& other) {
        std::unordered_map<const _Vertex*, VertexDescriptor> origToNew;

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

    // Конструктор перемещения
    UndirectedAbstractGraph(UndirectedAbstractGraph&&) noexcept = default;

    // Копирующее присваивание (через copy-and-swap)
    UndirectedAbstractGraph& operator=(const UndirectedAbstractGraph& other) {
        if (this != &other) {
            UndirectedAbstractGraph temp(other);
            *this = std::move(temp);
        }
        return *this;
    }

    // Перемещающее присваивание
    UndirectedAbstractGraph& operator=(UndirectedAbstractGraph&&) noexcept = default;

    // ---------- Vertex operations ----------
    template<typename... Args>
    VertexDescriptor emplaceVertex(Args&&... args) {
        _vertices.emplace_back(std::forward<Args>(args)...);
        auto it = std::prev(_vertices.end());
        return VertexDescriptor(it);
    }

    VertexDescriptor addVertex(const VertexData& data) { return emplaceVertex(data); }
    VertexDescriptor addVertex(VertexData&& data) { return emplaceVertex(std::move(data)); }

    void removeVertex(VertexDescriptor vertex) {
        // Make a copy because the incident edge list will be modified during iteration
        auto incidentCopy = vertex._label->_incidentEdges;
        for (auto edgeLabel : incidentCopy) {
            removeEdge(EdgeDescriptor(edgeLabel));
        }
        _vertices.erase(vertex._label);
    }

    // ---------- Edge operations ----------
    template<typename... Args>
        requires (!std::is_void_v<EdgeData>)
    EdgeDescriptor emplaceEdge(VertexDescriptor from, VertexDescriptor to, Args&&... args) {
        _edges.emplace_back(from._label, to._label, std::forward<Args>(args)...);
        auto it = std::prev(_edges.end());
        from._label->_incidentEdges.push_back(it);
        to._label->_incidentEdges.push_back(it);
        it->_meta._posInV1 = std::prev(from._label->_incidentEdges.end());
        it->_meta._posInV2 = std::prev(to._label->_incidentEdges.end());
        return EdgeDescriptor(it);
    }

    // Шаблонная версия addEdge
    template<typename T = EdgeData>
        requires (!std::is_void_v<EdgeData>)
    EdgeDescriptor addEdge(VertexDescriptor from, VertexDescriptor to, T&& data)
    { return emplaceEdge(from, to, std::forward<T>(data)); }

    // Специализация для void EdgeData
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

    // ---------- Vertex iteration ----------
    VertexIterator beginVertices() { return VertexIterator(_vertices.begin()); }
    VertexIterator endVertices()   { return VertexIterator(_vertices.end()); }
    ConstVertexIterator beginVertices() const { return ConstVertexIterator(_vertices.begin()); }
    ConstVertexIterator endVertices()   const { return ConstVertexIterator(_vertices.end()); }
    ConstVertexIterator cbeginVertices() const { return ConstVertexIterator(_vertices.begin()); }
    ConstVertexIterator cendVertices()   const { return ConstVertexIterator(_vertices.end()); }

    auto vertices() {
        return std::ranges::subrange<VertexIterator, VertexIterator>(beginVertices(), endVertices());
    }
    auto vertices() const {
        return std::ranges::subrange<ConstVertexIterator, ConstVertexIterator>(beginVertices(), endVertices());
    }
    auto constVertices() const {
        return std::ranges::subrange<ConstVertexIterator, ConstVertexIterator>(cbeginVertices(), cendVertices());
    }

    // ---------- Edge iteration ----------
    EdgeIterator beginEdges() { return EdgeIterator(_edges.begin()); }
    EdgeIterator endEdges()   { return EdgeIterator(_edges.end()); }
    ConstEdgeIterator beginEdges() const { return ConstEdgeIterator(_edges.begin()); }
    ConstEdgeIterator endEdges()   const { return ConstEdgeIterator(_edges.end()); }
    ConstEdgeIterator cbeginEdges() const { return ConstEdgeIterator(_edges.begin()); }
    ConstEdgeIterator cendEdges()   const { return ConstEdgeIterator(_edges.end()); }

    auto edges() {
        return std::ranges::subrange<EdgeIterator, EdgeIterator>(beginEdges(), endEdges());
    }
    auto edges() const {
        return std::ranges::subrange<ConstEdgeIterator, ConstEdgeIterator>(beginEdges(), endEdges());
    }
    auto constEdges() const {
        return std::ranges::subrange<ConstEdgeIterator, ConstEdgeIterator>(cbeginEdges(), cendEdges());
    }

    // ---------- Capacity ----------
    std::size_t vertexCount() const { return _vertices.size(); }
    std::size_t edgeCount() const { return _edges.size(); }

    std::optional<EdgeDescriptor> findEdge(VertexDescriptor from, VertexDescriptor to) const {
        for(auto e : from.incidentEdges()) if(*e.otherEnd(from) == to) return e;
        return std::nullopt;
    }

    bool hasEdge(VertexDescriptor from, VertexDescriptor to) const { return findEdge(from, to).has_value(); }
};

} // namespace exx::incident

#endif // EXX_UNDIRECTEDABSTRACTGRAPH_HPP

#ifndef EXX_INFRASTRUCTURE_HPP
#define EXX_INFRASTRUCTURE_HPP

#ifndef EXX_MATRIXOWNINGVIEW_HPP
#define EXX_MATRIXOWNINGVIEW_HPP

#include <vector>
#include <ranges>
#include <algorithm>

#ifndef EXX_MATRIX_CONCEPTS_HPP
#define EXX_MATRIX_CONCEPTS_HPP

#include <concepts>

namespace exx::incident {

template<typename M>
concept MatrixLike = requires(M m, std::size_t i, std::size_t j) {
    typename M::value_type;
    { m.rows() } -> std::convertible_to<std::size_t>;
    { m.cols() } -> std::convertible_to<std::size_t>;
    { m(i, j) }  -> std::convertible_to<typename M::value_type>;
};

template<typename M>
concept MatrixProvadingDataView = requires(M m, std::size_t i, std::size_t j) {
    typename M::value_type;
    { m.rows() } -> std::convertible_to<std::size_t>;
    { m.cols() } -> std::convertible_to<std::size_t>;
    { m.data() } -> std::convertible_to<const typename M::value_type*>;
};

} // namespace exx::incident

#endif // EXX_MATRIX_CONCEPTS_HPP

namespace exx::incident {

template<typename T>
class MatrixOwningView {
public:
    using value_type = T;

    explicit MatrixOwningView(const T* data, std::size_t rows, std::size_t cols)
        : _rows(rows), _cols(cols), _storage(data, data + rows * cols) {}

    MatrixOwningView(std::initializer_list<std::initializer_list<T>> list) {
        _rows = list.size();
        _cols = list.begin()->size();
        _storage.reserve(_rows * _cols);
        std::ranges::copy(list | std::views::join, _storage.begin());
    }

    MatrixOwningView(const std::vector<std::vector<T>>& vec) {
        _rows = vec.size();
        _cols = vec.begin()->size();
        _storage.reserve(_rows * _cols);
        std::ranges::copy(vec | std::views::join, _storage.begin());
    }

    template<MatrixLike Mat>
    MatrixOwningView(const Mat& mat)
        : _rows(mat.rows()), _cols(mat.cols()), _storage(_rows * _cols)
    {
        for (std::size_t i = 0; i < _rows; ++i)
            for (std::size_t j = 0; j < _cols; ++j)
                _storage[i * _cols + j] = mat(i, j);
    }

    std::size_t rows() const noexcept { return _rows; }
    std::size_t cols() const noexcept { return _cols; }

    const T* data() const { return _storage.data(); }

    const T& operator()(std::size_t i, std::size_t j) const { return _storage[i * _cols + j]; }

private:
    std::size_t _rows, _cols;
    std::vector<T> _storage;
};

} // namespace exx::incident

#endif // EXX_MATRIXOWNINGVIEW_HPP

#ifndef EXX_MATRIXVIEW_HPP
#define EXX_MATRIXVIEW_HPP

#include <cstddef>

#ifndef EXX_MATRIX_CONCEPTS_HPP
#define EXX_MATRIX_CONCEPTS_HPP

#include <concepts>

namespace exx::incident {

template<typename M>
concept MatrixLike = requires(M m, std::size_t i, std::size_t j) {
    typename M::value_type;
    { m.rows() } -> std::convertible_to<std::size_t>;
    { m.cols() } -> std::convertible_to<std::size_t>;
    { m(i, j) }  -> std::convertible_to<typename M::value_type>;
};

template<typename M>
concept MatrixProvadingDataView = requires(M m, std::size_t i, std::size_t j) {
    typename M::value_type;
    { m.rows() } -> std::convertible_to<std::size_t>;
    { m.cols() } -> std::convertible_to<std::size_t>;
    { m.data() } -> std::convertible_to<const typename M::value_type*>;
};

} // namespace exx::incident

#endif // EXX_MATRIX_CONCEPTS_HPP

namespace exx::incident {

template<typename T>
class MatrixView {
public:
    using value_type = T;

    MatrixView(const T* data, std::size_t rows, std::size_t cols)
        : _data(data), _rows(rows), _cols(cols) {}

    template<MatrixProvadingDataView M>
    MatrixView(M&& m) : MatrixView(m.data(), m.rows(), m.cols()) {}

    std::size_t rows() const noexcept { return _rows; }
    std::size_t cols() const noexcept { return _cols; }

    const T& operator()(std::size_t i, std::size_t j) const { return _data[i * _cols + j]; }

private:
    const T* _data;
    std::size_t _rows, _cols;
};

} // namespace exx::incident

#endif // EXX_MATRIXVIEW_HPP

#ifndef EXX_MATRIX_CONCEPTS_HPP
#define EXX_MATRIX_CONCEPTS_HPP

#include <concepts>

namespace exx::incident {

template<typename M>
concept MatrixLike = requires(M m, std::size_t i, std::size_t j) {
    typename M::value_type;
    { m.rows() } -> std::convertible_to<std::size_t>;
    { m.cols() } -> std::convertible_to<std::size_t>;
    { m(i, j) }  -> std::convertible_to<typename M::value_type>;
};

template<typename M>
concept MatrixProvadingDataView = requires(M m, std::size_t i, std::size_t j) {
    typename M::value_type;
    { m.rows() } -> std::convertible_to<std::size_t>;
    { m.cols() } -> std::convertible_to<std::size_t>;
    { m.data() } -> std::convertible_to<const typename M::value_type*>;
};

} // namespace exx::incident

#endif // EXX_MATRIX_CONCEPTS_HPP

#endif // EXX_INFRASTRUCTURE_HPP

#ifndef EXX_ALGORITHMS_HPP
#define EXX_ALGORITHMS_HPP

#ifndef EXX_CREATION_OF_UNDIRECTEDABSTRACTGRAPH_HPP
#define EXX_CREATION_OF_UNDIRECTEDABSTRACTGRAPH_HPP

#include <vector>

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
        [[no_unique_address]] _EdgeData _data;
        _EraseAccelerationMetaData _meta{};

        template<typename... Args>
            requires (!std::is_void_v<EdgeData>)
        _Edge(_VertexLabel v1, _VertexLabel v2, Args&&... args)
            : _v1(v1), _v2(v2), _data(std::forward<Args>(args)...) {}

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

    // ---------- Descriptors ----------
    template<bool isConst>
    class _VertexDescriptor {
    private:
        using _ConditionalVertexLabel = std::conditional_t<isConst, _VertexConstLabel, _VertexLabel>;
        _ConditionalVertexLabel _label;

        friend class UndirectedAbstractGraph;

    public:
        _VertexDescriptor() = default;
        _VertexDescriptor(const _VertexDescriptor&) = default;
        _VertexDescriptor& operator=(const _VertexDescriptor&) = default;

        explicit _VertexDescriptor(_ConditionalVertexLabel label) : _label(label) {}

        // Conversion from non-const to const
        _VertexDescriptor(const _VertexDescriptor<false>& other) requires isConst
            : _label(other._label) {}

        auto& data()       requires (!isConst) { return _label->_data; }
        const auto& data() const               { return _label->_data; }

        auto incidentEdges() const {
            return std::views::transform(_label->_incidentEdges,
                                         [](const _EdgeLabel& el) { return _EdgeDescriptor<isConst>(el); });
        }

        bool operator==(const _VertexDescriptor& other) const { return _label == other._label; }
        bool operator!=(const _VertexDescriptor& other) const { return !(*this == other); }
    };

    template<bool isConst>
    class _EdgeDescriptor {
    private:
        using _ConditionalEdgeLabel = std::conditional_t<isConst, _EdgeConstLabel, _EdgeLabel>;
        _ConditionalEdgeLabel _label;

        friend class UndirectedAbstractGraph;

    public:
        _EdgeDescriptor() = default;
        _EdgeDescriptor(const _EdgeDescriptor&) = default;
        _EdgeDescriptor& operator=(const _EdgeDescriptor&) = default;

        explicit _EdgeDescriptor(_ConditionalEdgeLabel label) : _label(label) {}

        _EdgeDescriptor(const _EdgeDescriptor<false>& other) requires isConst
            : _label(other._label) {}

        auto& data()       requires (!std::is_void_v<EdgeData> && !isConst) { return _label->_data; }
        const auto& data() const requires (!std::is_void_v<EdgeData>) { return _label->_data; }

        _VertexDescriptor<isConst> v1() const { return _VertexDescriptor<isConst>(_label->_v1); }
        _VertexDescriptor<isConst> v2() const { return _VertexDescriptor<isConst>(_label->_v2); }

        std::optional<_VertexDescriptor<isConst>> otherEnd(const _VertexDescriptor<isConst>& vertex) const {
            if (vertex._label == _label->_v1)
                return _VertexDescriptor<isConst>(_label->_v2);
            else if (vertex._label == _label->_v2)
                return _VertexDescriptor<isConst>(_label->_v1);
            else
                return std::nullopt;
        }

        bool operator==(const _EdgeDescriptor& other) const { return _label == other._label; }
        bool operator!=(const _EdgeDescriptor& other) const { return !(*this == other); }
    };

    // ---------- Iterators ----------
    template<bool isConst>
    class _VertexIteratorImpl {
    private:
        using _ConditionalVertexLabel = std::conditional_t<isConst, _VertexConstLabel, _VertexLabel>;
        _ConditionalVertexLabel _it;

        friend class UndirectedAbstractGraph;

    public:
        using difference_type       = std::ptrdiff_t;
        using value_type            = _VertexDescriptor<isConst>;
        using reference             = value_type;
        using pointer               = void;
        using iterator_category     = std::forward_iterator_tag;
        using iterator_concept      = std::forward_iterator_tag;

        _VertexIteratorImpl() = default;
        _VertexIteratorImpl(const _VertexIteratorImpl&) = default;
        _VertexIteratorImpl& operator=(const _VertexIteratorImpl&) = default;

        explicit _VertexIteratorImpl(_ConditionalVertexLabel it) : _it(it) {}

        _VertexIteratorImpl(const _VertexIteratorImpl<false>& other) requires isConst
            : _it(other._it) {}

        reference operator*() const { return value_type(_it); }
        _VertexIteratorImpl& operator++() { ++_it; return *this; }
        _VertexIteratorImpl operator++(int) { auto tmp = *this; ++*this; return tmp; }

        bool operator==(const _VertexIteratorImpl& other) const { return _it == other._it; }
        bool operator!=(const _VertexIteratorImpl& other) const { return !(*this == other); }
    };

    template<bool isConst>
    class _EdgeIteratorImpl {
    private:
        using _ConditionalEdgeLabel = std::conditional_t<isConst, _EdgeConstLabel, _EdgeLabel>;
        _ConditionalEdgeLabel _it;

        friend class UndirectedAbstractGraph;

    public:
        using difference_type       = std::ptrdiff_t;
        using value_type            = _EdgeDescriptor<isConst>;
        using reference             = value_type;
        using pointer               = void;
        using iterator_category     = std::forward_iterator_tag;
        using iterator_concept      = std::forward_iterator_tag;

        _EdgeIteratorImpl() = default;
        _EdgeIteratorImpl(const _EdgeIteratorImpl&) = default;
        _EdgeIteratorImpl& operator=(const _EdgeIteratorImpl&) = default;

        explicit _EdgeIteratorImpl(_ConditionalEdgeLabel it) : _it(it) {}

        _EdgeIteratorImpl(const _EdgeIteratorImpl<false>& other) requires isConst
            : _it(other._it) {}

        reference operator*() const { return value_type(_it); }
        _EdgeIteratorImpl& operator++() { ++_it; return *this; }
        _EdgeIteratorImpl operator++(int) { auto tmp = *this; ++*this; return tmp; }

        bool operator==(const _EdgeIteratorImpl& other) const { return _it == other._it; }
        bool operator!=(const _EdgeIteratorImpl& other) const { return !(*this == other); }
    };

public:
    // Public types
    using VertexDescriptor      = _VertexDescriptor<false>;
    using ConstVertexDescriptor = _VertexDescriptor<true>;
    using EdgeDescriptor        = _EdgeDescriptor<false>;
    using ConstEdgeDescriptor   = _EdgeDescriptor<true>;

    using VertexIterator      = _VertexIteratorImpl<false>;
    using ConstVertexIterator = _VertexIteratorImpl<true>;
    using EdgeIterator        = _EdgeIteratorImpl<false>;
    using ConstEdgeIterator   = _EdgeIteratorImpl<true>;

    UndirectedAbstractGraph() = default;

    // Конструктор копирования (глубокое копирование)
    UndirectedAbstractGraph(const UndirectedAbstractGraph& other) {
        std::unordered_map<const _Vertex*, VertexDescriptor> origToNew;

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

    // Конструктор перемещения
    UndirectedAbstractGraph(UndirectedAbstractGraph&&) noexcept = default;

    // Копирующее присваивание (через copy-and-swap)
    UndirectedAbstractGraph& operator=(const UndirectedAbstractGraph& other) {
        if (this != &other) {
            UndirectedAbstractGraph temp(other);
            *this = std::move(temp);
        }
        return *this;
    }

    // Перемещающее присваивание
    UndirectedAbstractGraph& operator=(UndirectedAbstractGraph&&) noexcept = default;

    // ---------- Vertex operations ----------
    template<typename... Args>
    VertexDescriptor emplaceVertex(Args&&... args) {
        _vertices.emplace_back(std::forward<Args>(args)...);
        auto it = std::prev(_vertices.end());
        return VertexDescriptor(it);
    }

    VertexDescriptor addVertex(const VertexData& data) { return emplaceVertex(data); }
    VertexDescriptor addVertex(VertexData&& data) { return emplaceVertex(std::move(data)); }

    void removeVertex(VertexDescriptor vertex) {
        // Make a copy because the incident edge list will be modified during iteration
        auto incidentCopy = vertex._label->_incidentEdges;
        for (auto edgeLabel : incidentCopy) {
            removeEdge(EdgeDescriptor(edgeLabel));
        }
        _vertices.erase(vertex._label);
    }

    // ---------- Edge operations ----------
    template<typename... Args>
        requires (!std::is_void_v<EdgeData>)
    EdgeDescriptor emplaceEdge(VertexDescriptor from, VertexDescriptor to, Args&&... args) {
        _edges.emplace_back(from._label, to._label, std::forward<Args>(args)...);
        auto it = std::prev(_edges.end());
        from._label->_incidentEdges.push_back(it);
        to._label->_incidentEdges.push_back(it);
        it->_meta._posInV1 = std::prev(from._label->_incidentEdges.end());
        it->_meta._posInV2 = std::prev(to._label->_incidentEdges.end());
        return EdgeDescriptor(it);
    }

    // Шаблонная версия addEdge
    template<typename T = EdgeData>
        requires (!std::is_void_v<EdgeData>)
    EdgeDescriptor addEdge(VertexDescriptor from, VertexDescriptor to, T&& data)
    { return emplaceEdge(from, to, std::forward<T>(data)); }

    // Специализация для void EdgeData
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

    // ---------- Vertex iteration ----------
    VertexIterator beginVertices() { return VertexIterator(_vertices.begin()); }
    VertexIterator endVertices()   { return VertexIterator(_vertices.end()); }
    ConstVertexIterator beginVertices() const { return ConstVertexIterator(_vertices.begin()); }
    ConstVertexIterator endVertices()   const { return ConstVertexIterator(_vertices.end()); }
    ConstVertexIterator cbeginVertices() const { return ConstVertexIterator(_vertices.begin()); }
    ConstVertexIterator cendVertices()   const { return ConstVertexIterator(_vertices.end()); }

    auto vertices() {
        return std::ranges::subrange<VertexIterator, VertexIterator>(beginVertices(), endVertices());
    }
    auto vertices() const {
        return std::ranges::subrange<ConstVertexIterator, ConstVertexIterator>(beginVertices(), endVertices());
    }
    auto constVertices() const {
        return std::ranges::subrange<ConstVertexIterator, ConstVertexIterator>(cbeginVertices(), cendVertices());
    }

    // ---------- Edge iteration ----------
    EdgeIterator beginEdges() { return EdgeIterator(_edges.begin()); }
    EdgeIterator endEdges()   { return EdgeIterator(_edges.end()); }
    ConstEdgeIterator beginEdges() const { return ConstEdgeIterator(_edges.begin()); }
    ConstEdgeIterator endEdges()   const { return ConstEdgeIterator(_edges.end()); }
    ConstEdgeIterator cbeginEdges() const { return ConstEdgeIterator(_edges.begin()); }
    ConstEdgeIterator cendEdges()   const { return ConstEdgeIterator(_edges.end()); }

    auto edges() {
        return std::ranges::subrange<EdgeIterator, EdgeIterator>(beginEdges(), endEdges());
    }
    auto edges() const {
        return std::ranges::subrange<ConstEdgeIterator, ConstEdgeIterator>(beginEdges(), endEdges());
    }
    auto constEdges() const {
        return std::ranges::subrange<ConstEdgeIterator, ConstEdgeIterator>(cbeginEdges(), cendEdges());
    }

    // ---------- Capacity ----------
    std::size_t vertexCount() const { return _vertices.size(); }
    std::size_t edgeCount() const { return _edges.size(); }

    std::optional<EdgeDescriptor> findEdge(VertexDescriptor from, VertexDescriptor to) const {
        for(auto e : from.incidentEdges()) if(*e.otherEnd(from) == to) return e;
        return std::nullopt;
    }

    bool hasEdge(VertexDescriptor from, VertexDescriptor to) const { return findEdge(from, to).has_value(); }
};

} // namespace exx::incident

#endif // EXX_UNDIRECTEDABSTRACTGRAPH_HPP

#ifndef EXX_MATRIXOWNINGVIEW_HPP
#define EXX_MATRIXOWNINGVIEW_HPP

#include <vector>
#include <ranges>
#include <algorithm>

#ifndef EXX_MATRIX_CONCEPTS_HPP
#define EXX_MATRIX_CONCEPTS_HPP

#include <concepts>

namespace exx::incident {

template<typename M>
concept MatrixLike = requires(M m, std::size_t i, std::size_t j) {
    typename M::value_type;
    { m.rows() } -> std::convertible_to<std::size_t>;
    { m.cols() } -> std::convertible_to<std::size_t>;
    { m(i, j) }  -> std::convertible_to<typename M::value_type>;
};

template<typename M>
concept MatrixProvadingDataView = requires(M m, std::size_t i, std::size_t j) {
    typename M::value_type;
    { m.rows() } -> std::convertible_to<std::size_t>;
    { m.cols() } -> std::convertible_to<std::size_t>;
    { m.data() } -> std::convertible_to<const typename M::value_type*>;
};

} // namespace exx::incident

#endif // EXX_MATRIX_CONCEPTS_HPP

namespace exx::incident {

template<typename T>
class MatrixOwningView {
public:
    using value_type = T;

    explicit MatrixOwningView(const T* data, std::size_t rows, std::size_t cols)
        : _rows(rows), _cols(cols), _storage(data, data + rows * cols) {}

    MatrixOwningView(std::initializer_list<std::initializer_list<T>> list) {
        _rows = list.size();
        _cols = list.begin()->size();
        _storage.reserve(_rows * _cols);
        std::ranges::copy(list | std::views::join, _storage.begin());
    }

    MatrixOwningView(const std::vector<std::vector<T>>& vec) {
        _rows = vec.size();
        _cols = vec.begin()->size();
        _storage.reserve(_rows * _cols);
        std::ranges::copy(vec | std::views::join, _storage.begin());
    }

    template<MatrixLike Mat>
    MatrixOwningView(const Mat& mat)
        : _rows(mat.rows()), _cols(mat.cols()), _storage(_rows * _cols)
    {
        for (std::size_t i = 0; i < _rows; ++i)
            for (std::size_t j = 0; j < _cols; ++j)
                _storage[i * _cols + j] = mat(i, j);
    }

    std::size_t rows() const noexcept { return _rows; }
    std::size_t cols() const noexcept { return _cols; }

    const T* data() const { return _storage.data(); }

    const T& operator()(std::size_t i, std::size_t j) const { return _storage[i * _cols + j]; }

private:
    std::size_t _rows, _cols;
    std::vector<T> _storage;
};

} // namespace exx::incident

#endif // EXX_MATRIXOWNINGVIEW_HPP

#ifndef EXX_MATRIXVIEW_HPP
#define EXX_MATRIXVIEW_HPP

#include <cstddef>

#ifndef EXX_MATRIX_CONCEPTS_HPP
#define EXX_MATRIX_CONCEPTS_HPP

#include <concepts>

namespace exx::incident {

template<typename M>
concept MatrixLike = requires(M m, std::size_t i, std::size_t j) {
    typename M::value_type;
    { m.rows() } -> std::convertible_to<std::size_t>;
    { m.cols() } -> std::convertible_to<std::size_t>;
    { m(i, j) }  -> std::convertible_to<typename M::value_type>;
};

template<typename M>
concept MatrixProvadingDataView = requires(M m, std::size_t i, std::size_t j) {
    typename M::value_type;
    { m.rows() } -> std::convertible_to<std::size_t>;
    { m.cols() } -> std::convertible_to<std::size_t>;
    { m.data() } -> std::convertible_to<const typename M::value_type*>;
};

} // namespace exx::incident

#endif // EXX_MATRIX_CONCEPTS_HPP

namespace exx::incident {

template<typename T>
class MatrixView {
public:
    using value_type = T;

    MatrixView(const T* data, std::size_t rows, std::size_t cols)
        : _data(data), _rows(rows), _cols(cols) {}

    template<MatrixProvadingDataView M>
    MatrixView(M&& m) : MatrixView(m.data(), m.rows(), m.cols()) {}

    std::size_t rows() const noexcept { return _rows; }
    std::size_t cols() const noexcept { return _cols; }

    const T& operator()(std::size_t i, std::size_t j) const { return _data[i * _cols + j]; }

private:
    const T* _data;
    std::size_t _rows, _cols;
};

} // namespace exx::incident

#endif // EXX_MATRIXVIEW_HPP

namespace exx::incident {

namespace impl {

template<typename VertexData, typename EdgeData>
static UndirectedAbstractGraph<VertexData, EdgeData>
make_graph_from_matrix(const MatrixView<EdgeData>& mat, EdgeData noEdgeValue) {
    using std::size_t;
    size_t n = mat.rows();
    if (n != mat.cols()) {
        throw std::invalid_argument("Matrix must be square for graph adjacency");
    }

    UndirectedAbstractGraph<VertexData, EdgeData> g;
    std::vector<typename UndirectedAbstractGraph<VertexData, EdgeData>::VertexDescriptor> vertDescrs;
    vertDescrs.reserve(n);

    for (size_t i = 0; i < n; ++i)
        vertDescrs.push_back(g.addVertex(VertexData{static_cast<VertexData>(i)}));

    for (size_t i = 0; i < n; ++i) {
        for (size_t j = i + 1; j < n; ++j) {
            EdgeData w = mat(i, j);
            if (w != noEdgeValue) {
                g.addEdge(vertDescrs[i], vertDescrs[j], w);
            }
        }
    }
    return g;
}

} // namespace impl

template<typename VertexData, typename EdgeData>
UndirectedAbstractGraph<VertexData, EdgeData>
make_graph_from_matrix(std::initializer_list<std::initializer_list<EdgeData>> list, EdgeData noEdgeValue = EdgeData{}) {
    MatrixOwningView<EdgeData> mat(list);
    MatrixView<EdgeData> view(mat.data(), mat.rows(), mat.cols());
    return impl::make_graph_from_matrix<VertexData, EdgeData>(view, noEdgeValue);
}

template<typename VertexData, typename EdgeData>
UndirectedAbstractGraph<VertexData, EdgeData>
make_graph_from_matrix(std::vector<std::vector<EdgeData>> vec, EdgeData noEdgeValue = EdgeData{}) {
    MatrixOwningView<EdgeData> mat(vec);
    MatrixView<EdgeData> view(mat.data(), mat.rows(), mat.cols());
    return impl::make_graph_from_matrix<VertexData, EdgeData>(view, noEdgeValue);
}

template<typename VertexData, typename EdgeData>
UndirectedAbstractGraph<VertexData, EdgeData>
make_graph_from_matrix(const MatrixView<EdgeData>& mat, EdgeData noEdgeValue = EdgeData{}) {
    return impl::make_graph_from_matrix<VertexData, EdgeData>(mat, noEdgeValue);
}

template<typename VertexData, typename EdgeData, MatrixLike M>
UndirectedAbstractGraph<VertexData, EdgeData>
make_graph_from_matrix(M&& mat, EdgeData noEdgeValue = EdgeData{}) {
    return impl::make_graph_from_matrix<VertexData, EdgeData>(MatrixView<EdgeData>(std::forward<M>(mat)), noEdgeValue);
}

} // namespace exx::incident

#endif // EXX_CREATION_OF_UNDIRECTEDABSTRACTGRAPH_HPP

#ifndef EXX_MSTPRIM_HPP
#define EXX_MSTPRIM_HPP

#include <queue>
#include <unordered_set>
#include <vector>
#include <expected>
#include <string>

#ifndef EXX_UNDIRECTEDGRAPH_HPP
#define EXX_UNDIRECTEDGRAPH_HPP

#include <optional>
#ifndef EXX_UNDIRECTEDMULTIGRAPH_HPP
#define EXX_UNDIRECTEDMULTIGRAPH_HPP

#include <optional>
#ifndef EXX_UNDIRECTEDPSEUDOGRAPH_HPP
#define EXX_UNDIRECTEDPSEUDOGRAPH_HPP

#include <unordered_map>
#include <optional>
#include <algorithm>

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
        [[no_unique_address]] _EdgeData _data;
        _EraseAccelerationMetaData _meta{};

        template<typename... Args>
            requires (!std::is_void_v<EdgeData>)
        _Edge(_VertexLabel v1, _VertexLabel v2, Args&&... args)
            : _v1(v1), _v2(v2), _data(std::forward<Args>(args)...) {}

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

    // ---------- Descriptors ----------
    template<bool isConst>
    class _VertexDescriptor {
    private:
        using _ConditionalVertexLabel = std::conditional_t<isConst, _VertexConstLabel, _VertexLabel>;
        _ConditionalVertexLabel _label;

        friend class UndirectedAbstractGraph;

    public:
        _VertexDescriptor() = default;
        _VertexDescriptor(const _VertexDescriptor&) = default;
        _VertexDescriptor& operator=(const _VertexDescriptor&) = default;

        explicit _VertexDescriptor(_ConditionalVertexLabel label) : _label(label) {}

        // Conversion from non-const to const
        _VertexDescriptor(const _VertexDescriptor<false>& other) requires isConst
            : _label(other._label) {}

        auto& data()       requires (!isConst) { return _label->_data; }
        const auto& data() const               { return _label->_data; }

        auto incidentEdges() const {
            return std::views::transform(_label->_incidentEdges,
                                         [](const _EdgeLabel& el) { return _EdgeDescriptor<isConst>(el); });
        }

        bool operator==(const _VertexDescriptor& other) const { return _label == other._label; }
        bool operator!=(const _VertexDescriptor& other) const { return !(*this == other); }
    };

    template<bool isConst>
    class _EdgeDescriptor {
    private:
        using _ConditionalEdgeLabel = std::conditional_t<isConst, _EdgeConstLabel, _EdgeLabel>;
        _ConditionalEdgeLabel _label;

        friend class UndirectedAbstractGraph;

    public:
        _EdgeDescriptor() = default;
        _EdgeDescriptor(const _EdgeDescriptor&) = default;
        _EdgeDescriptor& operator=(const _EdgeDescriptor&) = default;

        explicit _EdgeDescriptor(_ConditionalEdgeLabel label) : _label(label) {}

        _EdgeDescriptor(const _EdgeDescriptor<false>& other) requires isConst
            : _label(other._label) {}

        auto& data()       requires (!std::is_void_v<EdgeData> && !isConst) { return _label->_data; }
        const auto& data() const requires (!std::is_void_v<EdgeData>) { return _label->_data; }

        _VertexDescriptor<isConst> v1() const { return _VertexDescriptor<isConst>(_label->_v1); }
        _VertexDescriptor<isConst> v2() const { return _VertexDescriptor<isConst>(_label->_v2); }

        std::optional<_VertexDescriptor<isConst>> otherEnd(const _VertexDescriptor<isConst>& vertex) const {
            if (vertex._label == _label->_v1)
                return _VertexDescriptor<isConst>(_label->_v2);
            else if (vertex._label == _label->_v2)
                return _VertexDescriptor<isConst>(_label->_v1);
            else
                return std::nullopt;
        }

        bool operator==(const _EdgeDescriptor& other) const { return _label == other._label; }
        bool operator!=(const _EdgeDescriptor& other) const { return !(*this == other); }
    };

    // ---------- Iterators ----------
    template<bool isConst>
    class _VertexIteratorImpl {
    private:
        using _ConditionalVertexLabel = std::conditional_t<isConst, _VertexConstLabel, _VertexLabel>;
        _ConditionalVertexLabel _it;

        friend class UndirectedAbstractGraph;

    public:
        using difference_type       = std::ptrdiff_t;
        using value_type            = _VertexDescriptor<isConst>;
        using reference             = value_type;
        using pointer               = void;
        using iterator_category     = std::forward_iterator_tag;
        using iterator_concept      = std::forward_iterator_tag;

        _VertexIteratorImpl() = default;
        _VertexIteratorImpl(const _VertexIteratorImpl&) = default;
        _VertexIteratorImpl& operator=(const _VertexIteratorImpl&) = default;

        explicit _VertexIteratorImpl(_ConditionalVertexLabel it) : _it(it) {}

        _VertexIteratorImpl(const _VertexIteratorImpl<false>& other) requires isConst
            : _it(other._it) {}

        reference operator*() const { return value_type(_it); }
        _VertexIteratorImpl& operator++() { ++_it; return *this; }
        _VertexIteratorImpl operator++(int) { auto tmp = *this; ++*this; return tmp; }

        bool operator==(const _VertexIteratorImpl& other) const { return _it == other._it; }
        bool operator!=(const _VertexIteratorImpl& other) const { return !(*this == other); }
    };

    template<bool isConst>
    class _EdgeIteratorImpl {
    private:
        using _ConditionalEdgeLabel = std::conditional_t<isConst, _EdgeConstLabel, _EdgeLabel>;
        _ConditionalEdgeLabel _it;

        friend class UndirectedAbstractGraph;

    public:
        using difference_type       = std::ptrdiff_t;
        using value_type            = _EdgeDescriptor<isConst>;
        using reference             = value_type;
        using pointer               = void;
        using iterator_category     = std::forward_iterator_tag;
        using iterator_concept      = std::forward_iterator_tag;

        _EdgeIteratorImpl() = default;
        _EdgeIteratorImpl(const _EdgeIteratorImpl&) = default;
        _EdgeIteratorImpl& operator=(const _EdgeIteratorImpl&) = default;

        explicit _EdgeIteratorImpl(_ConditionalEdgeLabel it) : _it(it) {}

        _EdgeIteratorImpl(const _EdgeIteratorImpl<false>& other) requires isConst
            : _it(other._it) {}

        reference operator*() const { return value_type(_it); }
        _EdgeIteratorImpl& operator++() { ++_it; return *this; }
        _EdgeIteratorImpl operator++(int) { auto tmp = *this; ++*this; return tmp; }

        bool operator==(const _EdgeIteratorImpl& other) const { return _it == other._it; }
        bool operator!=(const _EdgeIteratorImpl& other) const { return !(*this == other); }
    };

public:
    // Public types
    using VertexDescriptor      = _VertexDescriptor<false>;
    using ConstVertexDescriptor = _VertexDescriptor<true>;
    using EdgeDescriptor        = _EdgeDescriptor<false>;
    using ConstEdgeDescriptor   = _EdgeDescriptor<true>;

    using VertexIterator      = _VertexIteratorImpl<false>;
    using ConstVertexIterator = _VertexIteratorImpl<true>;
    using EdgeIterator        = _EdgeIteratorImpl<false>;
    using ConstEdgeIterator   = _EdgeIteratorImpl<true>;

    UndirectedAbstractGraph() = default;

    // Конструктор копирования (глубокое копирование)
    UndirectedAbstractGraph(const UndirectedAbstractGraph& other) {
        std::unordered_map<const _Vertex*, VertexDescriptor> origToNew;

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

    // Конструктор перемещения
    UndirectedAbstractGraph(UndirectedAbstractGraph&&) noexcept = default;

    // Копирующее присваивание (через copy-and-swap)
    UndirectedAbstractGraph& operator=(const UndirectedAbstractGraph& other) {
        if (this != &other) {
            UndirectedAbstractGraph temp(other);
            *this = std::move(temp);
        }
        return *this;
    }

    // Перемещающее присваивание
    UndirectedAbstractGraph& operator=(UndirectedAbstractGraph&&) noexcept = default;

    // ---------- Vertex operations ----------
    template<typename... Args>
    VertexDescriptor emplaceVertex(Args&&... args) {
        _vertices.emplace_back(std::forward<Args>(args)...);
        auto it = std::prev(_vertices.end());
        return VertexDescriptor(it);
    }

    VertexDescriptor addVertex(const VertexData& data) { return emplaceVertex(data); }
    VertexDescriptor addVertex(VertexData&& data) { return emplaceVertex(std::move(data)); }

    void removeVertex(VertexDescriptor vertex) {
        // Make a copy because the incident edge list will be modified during iteration
        auto incidentCopy = vertex._label->_incidentEdges;
        for (auto edgeLabel : incidentCopy) {
            removeEdge(EdgeDescriptor(edgeLabel));
        }
        _vertices.erase(vertex._label);
    }

    // ---------- Edge operations ----------
    template<typename... Args>
        requires (!std::is_void_v<EdgeData>)
    EdgeDescriptor emplaceEdge(VertexDescriptor from, VertexDescriptor to, Args&&... args) {
        _edges.emplace_back(from._label, to._label, std::forward<Args>(args)...);
        auto it = std::prev(_edges.end());
        from._label->_incidentEdges.push_back(it);
        to._label->_incidentEdges.push_back(it);
        it->_meta._posInV1 = std::prev(from._label->_incidentEdges.end());
        it->_meta._posInV2 = std::prev(to._label->_incidentEdges.end());
        return EdgeDescriptor(it);
    }

    // Шаблонная версия addEdge
    template<typename T = EdgeData>
        requires (!std::is_void_v<EdgeData>)
    EdgeDescriptor addEdge(VertexDescriptor from, VertexDescriptor to, T&& data)
    { return emplaceEdge(from, to, std::forward<T>(data)); }

    // Специализация для void EdgeData
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

    // ---------- Vertex iteration ----------
    VertexIterator beginVertices() { return VertexIterator(_vertices.begin()); }
    VertexIterator endVertices()   { return VertexIterator(_vertices.end()); }
    ConstVertexIterator beginVertices() const { return ConstVertexIterator(_vertices.begin()); }
    ConstVertexIterator endVertices()   const { return ConstVertexIterator(_vertices.end()); }
    ConstVertexIterator cbeginVertices() const { return ConstVertexIterator(_vertices.begin()); }
    ConstVertexIterator cendVertices()   const { return ConstVertexIterator(_vertices.end()); }

    auto vertices() {
        return std::ranges::subrange<VertexIterator, VertexIterator>(beginVertices(), endVertices());
    }
    auto vertices() const {
        return std::ranges::subrange<ConstVertexIterator, ConstVertexIterator>(beginVertices(), endVertices());
    }
    auto constVertices() const {
        return std::ranges::subrange<ConstVertexIterator, ConstVertexIterator>(cbeginVertices(), cendVertices());
    }

    // ---------- Edge iteration ----------
    EdgeIterator beginEdges() { return EdgeIterator(_edges.begin()); }
    EdgeIterator endEdges()   { return EdgeIterator(_edges.end()); }
    ConstEdgeIterator beginEdges() const { return ConstEdgeIterator(_edges.begin()); }
    ConstEdgeIterator endEdges()   const { return ConstEdgeIterator(_edges.end()); }
    ConstEdgeIterator cbeginEdges() const { return ConstEdgeIterator(_edges.begin()); }
    ConstEdgeIterator cendEdges()   const { return ConstEdgeIterator(_edges.end()); }

    auto edges() {
        return std::ranges::subrange<EdgeIterator, EdgeIterator>(beginEdges(), endEdges());
    }
    auto edges() const {
        return std::ranges::subrange<ConstEdgeIterator, ConstEdgeIterator>(beginEdges(), endEdges());
    }
    auto constEdges() const {
        return std::ranges::subrange<ConstEdgeIterator, ConstEdgeIterator>(cbeginEdges(), cendEdges());
    }

    // ---------- Capacity ----------
    std::size_t vertexCount() const { return _vertices.size(); }
    std::size_t edgeCount() const { return _edges.size(); }

    std::optional<EdgeDescriptor> findEdge(VertexDescriptor from, VertexDescriptor to) const {
        for(auto e : from.incidentEdges()) if(*e.otherEnd(from) == to) return e;
        return std::nullopt;
    }

    bool hasEdge(VertexDescriptor from, VertexDescriptor to) const { return findEdge(from, to).has_value(); }
};

} // namespace exx::incident

#endif // EXX_UNDIRECTEDABSTRACTGRAPH_HPP

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

namespace exx::incident {

enum class PrimError {
    DisconnectedGraph
};

inline std::string to_string(PrimError e) {
    switch (e) {
    case PrimError::DisconnectedGraph:
        return "Граф не является связным";
    }
    return "Неизвестная ошибка";
}

template<typename VertexData,
         typename EdgeData,
         typename VHash,
         typename EHash>
    requires (!std::is_void_v<EdgeData> && !std::is_void_v<VHash> && std::is_copy_constructible_v<EdgeData>)
std::expected<UndirectedGraph<VertexData, EdgeData, VHash, EHash>, PrimError>
mstPrim(const UndirectedGraph<VertexData, EdgeData, VHash, EHash>& graph)
{
    using GraphType = UndirectedGraph<VertexData, EdgeData, VHash, EHash>;
    using VertexDesc = typename GraphType::VertexDescriptor;
    using ConstVertexDesc = typename GraphType::ConstVertexDescriptor;
    if (graph.vertexCount() == 0) return GraphType{};

    GraphType mst;
    for (auto v : graph.constVertices()) mst.addVertex(v.data());

    struct QueueElement {
        EdgeData weight;
        VertexData vertexData;
        VertexData parentData;
    };
    auto cmp = [](const QueueElement& a, const QueueElement& b) {
        return a.weight > b.weight;
    };
    std::priority_queue<QueueElement, std::vector<QueueElement>, decltype(cmp)> pq(cmp);

    std::unordered_set<VertexData, VHash> visited(0, VHash{});
    VertexData startData = (*graph.constVertices().begin()).data();
    visited.insert(startData);

    ConstVertexDesc startDesc = (*graph.beginVertices());

    for (auto edge : startDesc.incidentEdges()) {
        auto otherDesc = edge.otherEnd(startDesc);
        VertexData otherData = otherDesc->data();
        if (!visited.contains(otherData))
            pq.push({edge.data(), otherData, startData});
    }

    while (!pq.empty()) {
        auto [weight, vData, parentData] = pq.top();
        pq.pop();

        if (visited.contains(vData)) continue;
        visited.insert(vData);

        auto parentDescOpt = mst.findVertex(parentData);
        auto vDescOpt = mst.findVertex(vData);
        if (!parentDescOpt || !vDescOpt) throw ("bebe");
        mst.addEdge(*parentDescOpt, *vDescOpt, weight);

        auto vDescOrigOpt = graph.findVertex(vData);
        if (!vDescOrigOpt) throw ("bubu");
        VertexDesc vDesc = *vDescOrigOpt;

        for (auto edge : vDesc.incidentEdges()) {
            auto otherDesc = edge.otherEnd(vDesc);
            VertexData otherData = otherDesc->data();
            if (!visited.contains(otherData))
                pq.push({edge.data(), otherData, vData});
        }
    }

    if (visited.size() != graph.vertexCount())
        return std::unexpected(PrimError::DisconnectedGraph);

    return mst;
}

} // namespace exx::incident

#endif // EXX_MSTPRIM_HPP

#endif // EXX_ALGORITHMS_HPP

#endif // EXX_INCIDENT_HPP
