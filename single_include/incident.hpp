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
    struct Vertex;
    struct Arc;

    using VertexList       = std::list<Vertex>;
    using ArcList          = std::list<Arc>;
    using VertexLabel      = typename VertexList::iterator;
    using VertexConstLabel = typename VertexList::const_iterator;
    using ArcLabel         = typename ArcList::iterator;
    using ArcConstLabel    = typename ArcList::const_iterator;

    struct EmptyArcData final {};
    using ConditionalArcData = std::conditional_t<std::is_void_v<ArcData>, EmptyArcData, ArcData>;

    struct EraseAccelerationMetaData {
        typename std::list<ArcLabel>::iterator _posInFrom{};
    };

    struct Arc {
        VertexLabel _from;
        VertexLabel _to;
        [[no_unique_address]] ConditionalArcData _data;
        EraseAccelerationMetaData _meta{};

        template<typename... Args>
            requires (!std::is_void_v<ArcData>)
        Arc(VertexLabel from, VertexLabel to, Args&&... args)
            : _from(from), _to(to), _data(std::forward<Args>(args)...) {}

        Arc(VertexLabel from, VertexLabel to)
            requires (std::is_void_v<ArcData>)
            : _from(from), _to(to) {}
    };

    struct Vertex {
        std::list<ArcLabel> _adjacentArcs;
        VertexData _data;
        template<typename... Args>
        explicit Vertex(Args&&... args) : _data(std::forward<Args>(args)...) {}
    };

    VertexList _vertices;
    ArcList    _arcs;

    // ---------- Внутренние дескрипторы (без итераторных операций) ----------
    template<bool isConst>
    class VertexDescriptorImpl {
    private:
        using ConditionalVertexLabel = std::conditional_t<isConst, VertexConstLabel, VertexLabel>;
        ConditionalVertexLabel _label;

        friend class DirectedAbstractGraph;

    public:
        VertexDescriptorImpl() = default;
        VertexDescriptorImpl(const VertexDescriptorImpl&) = default;
        VertexDescriptorImpl& operator=(const VertexDescriptorImpl&) = default;

        explicit VertexDescriptorImpl(ConditionalVertexLabel label) : _label(label) {}

        // Конвертация из неконстантного в константный
        VertexDescriptorImpl(const VertexDescriptorImpl<false>& other) requires isConst
            : _label(other._label) {}

        auto& data()       requires (!isConst) { return _label->_data; }
        const auto& data() const               { return _label->_data; }

        auto adjacentArcs() const {
            return std::views::transform(_label->_adjacentArcs,
                                         [](const ArcLabel& al) { return ArcDescriptorImpl<isConst>(al); });
        }

        bool operator==(const VertexDescriptorImpl& other) const { return _label == other._label; }
        bool operator!=(const VertexDescriptorImpl& other) const { return !(*this == other); }
    };

    template<bool isConst>
    class ArcDescriptorImpl {
    private:
        using ConditionalArcLabel = std::conditional_t<isConst, ArcConstLabel, ArcLabel>;
        ConditionalArcLabel _label;

        friend class DirectedAbstractGraph;

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

        bool operator==(const ArcDescriptorImpl& other) const { return _label == other._label; }
        bool operator!=(const ArcDescriptorImpl& other) const { return !(*this == other); }
    };

    // ---------- Итераторы (только для обхода) ----------
    template<bool isConst>
    class VertexIteratorImpl {
    private:
        using ConditionalVertexLabel = std::conditional_t<isConst, VertexConstLabel, VertexLabel>;
        ConditionalVertexLabel _it;

        friend class DirectedAbstractGraph;

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

        // Конвертация из неконстантного в константный
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
        using ConditionalArcLabel = std::conditional_t<isConst, ArcConstLabel, ArcLabel>;
        ConditionalArcLabel _it;

        friend class DirectedAbstractGraph;

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
    // Публичные типы
    using VertexDescriptor      = VertexDescriptorImpl<false>;
    using ConstVertexDescriptor = VertexDescriptorImpl<true>;
    struct VertexDescriptorHash {
        template<bool isConst>
        std::size_t operator()(const VertexDescriptorImpl<isConst>& desc) const
        { return std::hash<const void*>()( &(*desc._label) ); }
    };

    using ArcDescriptor         = ArcDescriptorImpl<false>;
    using ConstArcDescriptor    = ArcDescriptorImpl<true>;
    struct ArcDescriptorHash {
        template<bool isConst>
        std::size_t operator()(const ArcDescriptorImpl<isConst>& desc) const
        { return std::hash<const void*>()( &(*desc._label) ); }
    };

    using VertexIterator      = VertexIteratorImpl<false>;
    using ConstVertexIterator = VertexIteratorImpl<true>;
    using ArcIterator         = ArcIteratorImpl<false>;
    using ConstArcIterator    = ArcIteratorImpl<true>;

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

namespace exx::incident {

template<typename VertexData,
         typename EdgeData,
         typename VertexHash = std::hash<VertexData>>
class UndirectedMultiGraph {
public:

    using VertexDescriptor      = typename UndirectedPseudoGraph<VertexData, EdgeData, VertexHash>::VertexDescriptor;
    using ConstVertexDescriptor = typename UndirectedPseudoGraph<VertexData, EdgeData, VertexHash>::ConstVertexDescriptor;
    using VertexDescriptorHash  = typename UndirectedPseudoGraph<VertexData, EdgeData>::VertexDescriptorHash;

    using EdgeDescriptor        = typename UndirectedPseudoGraph<VertexData, EdgeData, VertexHash>::EdgeDescriptor;
    using ConstEdgeDescriptor   = typename UndirectedPseudoGraph<VertexData, EdgeData, VertexHash>::ConstEdgeDescriptor;
    using EdgeDescriptorHash    = typename UndirectedPseudoGraph<VertexData, EdgeData>::EdgeDescriptorHash;

    using VertexIterator        = typename UndirectedPseudoGraph<VertexData, EdgeData, VertexHash>::VertexIterator;
    using ConstVertexIterator   = typename UndirectedPseudoGraph<VertexData, EdgeData, VertexHash>::ConstVertexIterator;
    using EdgeIterator          = typename UndirectedPseudoGraph<VertexData, EdgeData, VertexHash>::EdgeIterator;
    using ConstEdgeIterator     = typename UndirectedPseudoGraph<VertexData, EdgeData, VertexHash>::ConstEdgeIterator;

    UndirectedMultiGraph() = default;

    explicit UndirectedMultiGraph(const UndirectedAbstractGraph<VertexData, EdgeData>& graph)
        : _pseudoGraph(graph) {}

    UndirectedMultiGraph(const UndirectedMultiGraph&) = default;
    UndirectedMultiGraph(UndirectedMultiGraph&&) noexcept = default;

    UndirectedMultiGraph& operator=(const UndirectedMultiGraph&) = default;
    UndirectedMultiGraph& operator=(UndirectedMultiGraph&&) noexcept = default;

    void swap(UndirectedMultiGraph& other) noexcept { _pseudoGraph.swap(other._pseudoGraph); }

    template<typename... Args>
    std::optional<VertexDescriptor> emplaceVertex(Args&&... args)
    { return _pseudoGraph.emplaceVertex(std::forward<Args>(args)...); }

    std::optional<VertexDescriptor> addVertex(const VertexData& data)
    { return _pseudoGraph.addVertex(data); }

    std::optional<VertexDescriptor> addVertex(VertexData&& data)
    { return _pseudoGraph.addVertex(std::move(data)); }

    void removeVertex(VertexDescriptor v)
    { _pseudoGraph.removeVertex(v); }

    std::optional<VertexDescriptor> findVertex(const VertexData& data) const
    { return _pseudoGraph.findVertex(data); }

    bool containsVertex(const VertexData& data) const
        requires (!std::is_void_v<VertexHash>)
    { return _pseudoGraph.containsVertex(data); }

    template<typename... Args>
        requires (!std::is_void_v<EdgeData>)
    std::optional<EdgeDescriptor> emplaceEdge(VertexDescriptor from,
                                              VertexDescriptor to,
                                              Args&&... args)
    {
        if (from == to) return std::nullopt;
        return _pseudoGraph.emplaceEdge(from, to, std::forward<Args>(args)...);
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
        return _pseudoGraph.addEdge(from, to);
    }

    template<typename... Args>
        requires (std::is_void_v<EdgeData>)
    EdgeDescriptor addEdge(const VertexData& fromData, const VertexData& toData) {
        auto fromOpt = findVertex(fromData);
        auto toOpt = findVertex(toData);
        if(!fromOpt || ! toOpt) return;
        return addEdge(*fromOpt, *toOpt);
    }

    void removeEdge(EdgeDescriptor e) { _pseudoGraph.removeEdge(e); }

    void removeEdge(const VertexData& fromData, const VertexData& toData)
    { _pseudoGraph.removeEdge(fromData, toData); }

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
    std::optional<ConstEdgeDescriptor> findEdge(VertexDescriptor from, VertexDescriptor to) const
    { return _pseudoGraph.findEdge(from, to); }

    std::optional<EdgeDescriptor> findEdge(const VertexData& fromData, const VertexData& toData)
    { return _pseudoGraph.findEdge(fromData, toData); }
    std::optional<ConstEdgeDescriptor> findEdge(const VertexData& fromData, const VertexData& toData) const
    { return _pseudoGraph.findEdge(fromData, toData); }

    bool hasEdge(VertexDescriptor from, VertexDescriptor to) const
    { return findEdge(from, to).has_value(); }

    bool hasEdge(const VertexData& fromData, const VertexData& toData) const
    { return _pseudoGraph.hasEdge(fromData, toData); }

    const UndirectedPseudoGraph<VertexData, EdgeData, VertexHash>& basePseudoGraph() const
    { return _pseudoGraph; }

private:

    UndirectedPseudoGraph<VertexData, EdgeData, VertexHash> _pseudoGraph;

};

} // namespace exx::incident

#endif // EXX_UNDIRECTEDMULTIGRAPH_HPP

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

    std::optional<VertexDescriptor> findVertex(const VertexData& data) const
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

    const UndirectedMultiGraph<VertexData, EdgeData, VertexHash>& baseMultiGraph() const
    { return _multiGraph; }

private:

    UndirectedMultiGraph<VertexData, EdgeData, VertexHash> _multiGraph;

};

} // namespace exx::incident

#endif // EXX_UNDIRECTEDGRAPH_HPP

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
[[deprecated]]
UndirectedAbstractGraph<VertexData, EdgeData>
make_graph_from_matrix(std::initializer_list<std::initializer_list<EdgeData>> list, EdgeData noEdgeValue = EdgeData{}) {
    MatrixOwningView<EdgeData> mat(list);
    MatrixView<EdgeData> view(mat.data(), mat.rows(), mat.cols());
    return impl::make_graph_from_matrix<VertexData, EdgeData>(view, noEdgeValue);
}

template<typename VertexData, typename EdgeData>
[[deprecated]]
UndirectedAbstractGraph<VertexData, EdgeData>
make_graph_from_matrix(std::vector<std::vector<EdgeData>> vec, EdgeData noEdgeValue = EdgeData{}) {
    MatrixOwningView<EdgeData> mat(vec);
    MatrixView<EdgeData> view(mat.data(), mat.rows(), mat.cols());
    return impl::make_graph_from_matrix<VertexData, EdgeData>(view, noEdgeValue);
}

template<typename VertexData, typename EdgeData>
[[deprecated]]
UndirectedAbstractGraph<VertexData, EdgeData>
make_graph_from_matrix(const MatrixView<EdgeData>& mat, EdgeData noEdgeValue = EdgeData{}) {
    return impl::make_graph_from_matrix<VertexData, EdgeData>(mat, noEdgeValue);
}

template<typename VertexData, typename EdgeData, MatrixLike M>
[[deprecated]]
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

namespace exx::incident {

template<typename VertexData,
         typename EdgeData,
         typename VertexHash = std::hash<VertexData>>
class UndirectedMultiGraph {
public:

    using VertexDescriptor      = typename UndirectedPseudoGraph<VertexData, EdgeData, VertexHash>::VertexDescriptor;
    using ConstVertexDescriptor = typename UndirectedPseudoGraph<VertexData, EdgeData, VertexHash>::ConstVertexDescriptor;
    using VertexDescriptorHash  = typename UndirectedPseudoGraph<VertexData, EdgeData>::VertexDescriptorHash;

    using EdgeDescriptor        = typename UndirectedPseudoGraph<VertexData, EdgeData, VertexHash>::EdgeDescriptor;
    using ConstEdgeDescriptor   = typename UndirectedPseudoGraph<VertexData, EdgeData, VertexHash>::ConstEdgeDescriptor;
    using EdgeDescriptorHash    = typename UndirectedPseudoGraph<VertexData, EdgeData>::EdgeDescriptorHash;

    using VertexIterator        = typename UndirectedPseudoGraph<VertexData, EdgeData, VertexHash>::VertexIterator;
    using ConstVertexIterator   = typename UndirectedPseudoGraph<VertexData, EdgeData, VertexHash>::ConstVertexIterator;
    using EdgeIterator          = typename UndirectedPseudoGraph<VertexData, EdgeData, VertexHash>::EdgeIterator;
    using ConstEdgeIterator     = typename UndirectedPseudoGraph<VertexData, EdgeData, VertexHash>::ConstEdgeIterator;

    UndirectedMultiGraph() = default;

    explicit UndirectedMultiGraph(const UndirectedAbstractGraph<VertexData, EdgeData>& graph)
        : _pseudoGraph(graph) {}

    UndirectedMultiGraph(const UndirectedMultiGraph&) = default;
    UndirectedMultiGraph(UndirectedMultiGraph&&) noexcept = default;

    UndirectedMultiGraph& operator=(const UndirectedMultiGraph&) = default;
    UndirectedMultiGraph& operator=(UndirectedMultiGraph&&) noexcept = default;

    void swap(UndirectedMultiGraph& other) noexcept { _pseudoGraph.swap(other._pseudoGraph); }

    template<typename... Args>
    std::optional<VertexDescriptor> emplaceVertex(Args&&... args)
    { return _pseudoGraph.emplaceVertex(std::forward<Args>(args)...); }

    std::optional<VertexDescriptor> addVertex(const VertexData& data)
    { return _pseudoGraph.addVertex(data); }

    std::optional<VertexDescriptor> addVertex(VertexData&& data)
    { return _pseudoGraph.addVertex(std::move(data)); }

    void removeVertex(VertexDescriptor v)
    { _pseudoGraph.removeVertex(v); }

    std::optional<VertexDescriptor> findVertex(const VertexData& data) const
    { return _pseudoGraph.findVertex(data); }

    bool containsVertex(const VertexData& data) const
        requires (!std::is_void_v<VertexHash>)
    { return _pseudoGraph.containsVertex(data); }

    template<typename... Args>
        requires (!std::is_void_v<EdgeData>)
    std::optional<EdgeDescriptor> emplaceEdge(VertexDescriptor from,
                                              VertexDescriptor to,
                                              Args&&... args)
    {
        if (from == to) return std::nullopt;
        return _pseudoGraph.emplaceEdge(from, to, std::forward<Args>(args)...);
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
        return _pseudoGraph.addEdge(from, to);
    }

    template<typename... Args>
        requires (std::is_void_v<EdgeData>)
    EdgeDescriptor addEdge(const VertexData& fromData, const VertexData& toData) {
        auto fromOpt = findVertex(fromData);
        auto toOpt = findVertex(toData);
        if(!fromOpt || ! toOpt) return;
        return addEdge(*fromOpt, *toOpt);
    }

    void removeEdge(EdgeDescriptor e) { _pseudoGraph.removeEdge(e); }

    void removeEdge(const VertexData& fromData, const VertexData& toData)
    { _pseudoGraph.removeEdge(fromData, toData); }

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
    std::optional<ConstEdgeDescriptor> findEdge(VertexDescriptor from, VertexDescriptor to) const
    { return _pseudoGraph.findEdge(from, to); }

    std::optional<EdgeDescriptor> findEdge(const VertexData& fromData, const VertexData& toData)
    { return _pseudoGraph.findEdge(fromData, toData); }
    std::optional<ConstEdgeDescriptor> findEdge(const VertexData& fromData, const VertexData& toData) const
    { return _pseudoGraph.findEdge(fromData, toData); }

    bool hasEdge(VertexDescriptor from, VertexDescriptor to) const
    { return findEdge(from, to).has_value(); }

    bool hasEdge(const VertexData& fromData, const VertexData& toData) const
    { return _pseudoGraph.hasEdge(fromData, toData); }

    const UndirectedPseudoGraph<VertexData, EdgeData, VertexHash>& basePseudoGraph() const
    { return _pseudoGraph; }

private:

    UndirectedPseudoGraph<VertexData, EdgeData, VertexHash> _pseudoGraph;

};

} // namespace exx::incident

#endif // EXX_UNDIRECTEDMULTIGRAPH_HPP

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

    std::optional<VertexDescriptor> findVertex(const VertexData& data) const
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

    const UndirectedMultiGraph<VertexData, EdgeData, VertexHash>& baseMultiGraph() const
    { return _multiGraph; }

private:

    UndirectedMultiGraph<VertexData, EdgeData, VertexHash> _multiGraph;

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
         typename VHash>
    requires (!std::is_void_v<EdgeData> &&
            std::is_copy_constructible_v<EdgeData>)
auto mstPrim(const UndirectedGraph<VertexData, EdgeData, VHash>& graph)
    -> std::expected<UndirectedGraph<VertexData, EdgeData, VHash>, PrimError>
{
    using GraphType = UndirectedGraph<VertexData, EdgeData, VHash>;
    using ConstVertexDesc = typename GraphType::ConstVertexDescriptor;

    if (graph.vertexCount() == 0) return GraphType{};

    GraphType mst;
    for (auto v : graph.constVertices()) mst.addVertex(v.data());

    struct QueueElement {
        EdgeData weight;
        ConstVertexDesc vertex;
        ConstVertexDesc parent;
    };
    auto cmp = [](const QueueElement& a, const QueueElement& b) {
        return a.weight > b.weight;
    };
    std::priority_queue<QueueElement, std::vector<QueueElement>, decltype(cmp)> pq(cmp);

    std::unordered_set<ConstVertexDesc, typename GraphType::VertexDescriptorHash> visited;

    ConstVertexDesc startDesc = *graph.constVertices().begin();
    visited.insert(startDesc);

    for (auto edge : startDesc.incidentEdges()) {
        auto otherDesc = edge.otherEnd(startDesc); // always not-nullopt optional
        if (!visited.contains(*otherDesc)) pq.push({edge.data(), *otherDesc, startDesc});
    }

    while (!pq.empty()) {
        auto [weight, vDesc, parentDesc] = pq.top();
        pq.pop();

        if (visited.contains(vDesc)) continue;
        visited.insert(vDesc);

        mst.addEdge(parentDesc.data(), vDesc.data(), weight);

        for (auto edge : vDesc.incidentEdges()) {
            auto otherDesc = edge.otherEnd(vDesc); // always not-nullopt optional
            if (!visited.contains(*otherDesc))
                pq.push({edge.data(), *otherDesc, vDesc});
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
