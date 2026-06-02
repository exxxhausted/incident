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
