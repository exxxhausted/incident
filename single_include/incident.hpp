#ifndef EXX_INCIDENT_HPP
#define EXX_INCIDENT_HPP

#ifndef EXX_DIRECTEDABSTRACTGRAPH_HPP
#define EXX_DIRECTEDABSTRACTGRAPH_HPP

#include <list>
#include <ranges>
#include <stdexcept>
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
    };

    // ---------- Итераторы (только для обхода) ----------
    template<bool isConst>
    class _VertexIteratorImpl {
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

        _VertexIteratorImpl() = default;
        _VertexIteratorImpl(const _VertexIteratorImpl&) = default;
        _VertexIteratorImpl& operator = (const _VertexIteratorImpl&) = default;

        explicit _VertexIteratorImpl(_ConditionalVertexLabel it) : _it(it) {}

        // Конвертация из неконстантного в константный
        _VertexIteratorImpl(const _VertexIteratorImpl<false>& other) requires isConst
            : _it(other._it) {}

        reference operator*() const { return value_type(_it); }
        _VertexIteratorImpl& operator++() { ++_it; return *this; }
        _VertexIteratorImpl operator++(int) { auto tmp = *this; ++*this; return tmp; }

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
        using difference_type       = std::ptrdiff_t;
        using value_type            = _ArcDescriptor<isConst>;
        using reference             = value_type;
        using pointer               = void;
        using iterator_category     = std::forward_iterator_tag;
        using iterator_concept      = std::forward_iterator_tag;

        _ArcIteratorImpl() = default;
        _ArcIteratorImpl(const _ArcIteratorImpl&) = default;
        _ArcIteratorImpl& operator = (const _ArcIteratorImpl&) = default;

        explicit _ArcIteratorImpl(_ConditionalArcLabel it) : _it(it) {}

        _ArcIteratorImpl(const _ArcIteratorImpl<false>& other) requires isConst
            : _it(other._it) {}

        reference operator*() const { return value_type(_it); }
        _ArcIteratorImpl& operator++() { ++_it; return *this; }
        _ArcIteratorImpl operator++(int) { auto tmp = *this; ++*this; return tmp; }

        bool operator==(const _ArcIteratorImpl& other) const { return _it == other._it; }
        bool operator!=(const _ArcIteratorImpl& other) const { return !(*this == other); }
    };

public:
    // Публичные типы
    using VertexDescriptor      = _VertexDescriptor<false>;
    using ConstVertexDescriptor = _VertexDescriptor<true>;
    using ArcDescriptor         = _ArcDescriptor<false>;
    using ConstArcDescriptor    = _ArcDescriptor<true>;

    using VertexIterator      = _VertexIteratorImpl<false>;
    using ConstVertexIterator = _VertexIteratorImpl<true>;
    using ArcIterator         = _ArcIteratorImpl<false>;
    using ConstArcIterator    = _ArcIteratorImpl<true>;

    // ---------- Методы графа ----------
    template<typename... Args>
    VertexDescriptor emplaceVertex(Args&&... args) {
        _vertices.emplace_back(std::forward<Args>(args)...);
        auto it = std::prev(_vertices.end());
        return VertexDescriptor(it);
    }

    VertexDescriptor addVertex(const VertexData& data) { return emplaceVertex(data); }

    void removeVertex(VertexDescriptor vertex) {
        // Удаляем все дуги, где вершина участвует
        for (auto it = _arcs.begin(); it != _arcs.end(); ) {
            if (it->_from == vertex._label || it->_to == vertex._label) {
                if (it->_from == vertex._label) {
                    it->_from->_adjacentArcs.erase(it->_meta._posInFrom);
                }
                // Для входящих дуг ничего не удаляем из списков, они не хранятся в to
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
    std::vector<typename UndirectedAbstractGraph<VertexData, EdgeData>::VertexIterator> vertIters;
    vertIters.reserve(n);

    for (size_t i = 0; i < n; ++i) {
        vertIters.push_back(g.addVertex(VertexData{static_cast<VertexData>(i)}));
    }

    for (size_t i = 0; i < n; ++i) {
        for (size_t j = i + 1; j < n; ++j) {
            EdgeData w = mat(i, j);
            if (w != noEdgeValue) {
                g.addEdge(vertIters[i], vertIters[j], w);
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
make_graph_from_matrix(std::vector<std::vector<EdgeData>> mat, EdgeData noEdgeValue = EdgeData{}) {
    if (mat.empty()) {
        MatrixView<EdgeData> emptyView(nullptr, 0, 0);
        return impl::make_graph_from_matrix<VertexData, EdgeData>(emptyView, noEdgeValue);
    }
    std::size_t rows = mat.size();
    std::size_t cols = mat[0].size();
    for (const auto& row : mat) {
        if (row.size() != cols)
            throw std::invalid_argument("Non-rectangular matrix");
    }
    auto range = mat | std::views::join;
    std::vector<EdgeData> flat(range.begin(), range.end());
    MatrixView<EdgeData> view(flat.data(), rows, cols);
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
    return impl::make_graph_from_matrix<VertexData, EdgeData>(
        MatrixView<EdgeData>(std::forward<M>(mat)), noEdgeValue);
}

} // namespace exx::incident

#endif // EXX_CREATION_OF_UNDIRECTEDABSTRACTGRAPH_HPP

#ifndef EXX_MSTPRIM_HPP
#define EXX_MSTPRIM_HPP

#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <stdexcept>

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

namespace exx::incident {

template<typename VertexData,
         typename EdgeData,
         typename VHash = std::hash<VertexData>,
         typename VEqual = std::equal_to<VertexData>>
UndirectedAbstractGraph<VertexData, EdgeData>
mstPrim(const UndirectedAbstractGraph<VertexData, EdgeData>& graph,
             VHash vHash = VHash{},
             VEqual vEqual = VEqual{})
{
    using _Graph = UndirectedAbstractGraph<VertexData, EdgeData>;

    using _СHashMap = std::unordered_map<VertexData,
                                       typename _Graph::ConstVertexIterator,
                                       VHash,
                                       VEqual>;

    using _HashMap = std::unordered_map<VertexData,
                                        typename _Graph::VertexIterator,
                                        VHash,
                                        VEqual>;

    {
        std::unordered_set<VertexData, VHash, VEqual> uniqueChecker(0, vHash, vEqual);
        for (auto v : graph.vertices()) {
            const VertexData& data = v.data();
            if (!uniqueChecker.insert(data).second)
                throw std::invalid_argument("mstPrim: VertexData values must be unique in the graph");
        }
    }

    if (graph.vertexCount() == 0) return _Graph{};

    _СHashMap originalVerticesIteratorsCollection(0, vHash, vEqual);

    for (auto vit = graph.cbeginVertices(); vit != graph.cendVertices(); ++vit)
        originalVerticesIteratorsCollection[(*vit).data()] = vit;

    _Graph mst;
    _HashMap mstVerticesIteratorsCollection(0, vHash, vEqual);

    for (const auto& [vData, _] : originalVerticesIteratorsCollection) {
        VertexData id = vData;
        mstVerticesIteratorsCollection[id] = mst.addVertex(id);
    }

    if (graph.edgeCount() == 0) return mst;

    struct _QueueEl {
        EdgeData _eData;
        VertexData _1;
        VertexData _2;
    };

    auto cmp = [](const _QueueEl& a, const _QueueEl& b) { return a._eData > b._eData; };

    std::priority_queue<_QueueEl, std::vector<_QueueEl>, decltype(cmp)> pq(cmp);

    std::unordered_set<VertexData, VHash, VEqual> visited(0, vHash, vEqual);
    VertexData startData = originalVerticesIteratorsCollection.begin()->first;
    visited.insert(startData);

    auto startIt = originalVerticesIteratorsCollection[startData];
    for (auto edge : (*startIt).incidentEdges()) {
        auto other = edge.otherEnd(startIt);
        VertexData otherData = (*other).data();
        EdgeData w = edge.data();
        pq.push( { w, otherData, startData } );
    }

    while (!pq.empty()) {
        auto [w, vData, parentData] = pq.top();
        pq.pop();
        if (visited.contains(vData)) continue;

        visited.insert(vData);
        mst.addEdge(mstVerticesIteratorsCollection[parentData],
                    mstVerticesIteratorsCollection[vData],
                    w);

        auto vIt = originalVerticesIteratorsCollection[vData];
        for (auto edge : (*vIt).incidentEdges()) {
            auto other = edge.otherEnd(vIt);
            VertexData otherData = (*other).data();
            if (!visited.contains(otherData))
                pq.push( { edge.data(), otherData, vData } );
        }
    }

    if (visited.size() != graph.vertexCount())
        throw std::logic_error("Graph is disconnected, cannot build MST");

    return mst;
}

} // namespace exx::incident

#endif // EXX_MSTPRIM_HPP

#endif // EXX_ALGORITHMS_HPP

#endif // EXX_INCIDENT_HPP
