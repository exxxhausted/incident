#ifndef EXX_DIRECTEDPSEUDOGRAPH_HPP
#define EXX_DIRECTEDPSEUDOGRAPH_HPP

#include <list>
#include <ranges>
#include <type_traits>
#include <optional>
#include <unordered_set>
#include <algorithm>

#include "../details/hash_std_injection.hpp" // IWYU pragma: keep

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
