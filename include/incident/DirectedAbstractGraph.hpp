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
