#ifndef EXX_TRANSPOSEDGRAPHVIEW_SIMPLE_HPP
#define EXX_TRANSPOSEDGRAPHVIEW_SIMPLE_HPP

#include <ranges>
#include <optional>
#include <unordered_set>
#include <vector>

#include "../details/hash_std_injection.hpp" // IWYU pragma: keep

namespace exx::incident {

template<typename Graph>
class TransposedGraphView {
public:
    using VertexValueType = typename Graph::VertexValueType;
    using ArcValueType    = typename Graph::ArcValueType;

    class VertexDescriptor;
    using ConstVertexDescriptor = VertexDescriptor;
    class ArcDescriptor;
    using ConstArcDescriptor = VertexDescriptor;

    using VertexIterator      = typename Graph::VertexIterator;
    using ConstVertexIterator = typename Graph::ConstVertexIterator;
    using ArcIterator         = typename Graph::ArcIterator;
    using ConstArcIterator    = typename Graph::ConstArcIterator;

    struct VertexDescriptorHash {
        std::size_t operator()(const VertexDescriptor& desc) const {
            using OrigHasher = typename Graph::ConstVertexDescriptor::CustomHasherProvidedByExxIncident;
            return OrigHasher{}(desc.base());
        }
    };
    struct ArcDescriptorHash {
        std::size_t operator()(const ArcDescriptor& desc) const {
            using OrigHasher = typename Graph::ConstArcDescriptor::CustomHasherProvidedByExxIncident;
            return OrigHasher{}(desc.base());
        }
    };

    explicit TransposedGraphView(const Graph& graph) : _graph(graph) {}

    auto vertices() const {
        return _graph.vertices()
               | std::views::transform([this](auto v) { return VertexDescriptor(v); });
    }

    auto arcs() const {
        return _graph.arcs()
               | std::views::transform([this](auto a) { return ArcDescriptor(a); });
    }

    auto beginVertices() const { return vertices().begin(); }
    auto endVertices()   const { return vertices().end(); }
    auto beginArcs()     const { return arcs().begin(); }
    auto endArcs()       const { return arcs().end(); }

    std::size_t vertexCount() const { return _graph.vertexCount(); }
    std::size_t arcCount()   const { return _graph.arcCount(); }

    std::optional<ArcDescriptor> findArc(VertexDescriptor from, VertexDescriptor to) const {
        auto origArc = _graph.findArc(to._orig, from._orig);
        if (origArc) return ArcDescriptor(*origArc);
        return std::nullopt;
    }

    bool hasArc(VertexDescriptor from, VertexDescriptor to) const
    { return findArc(from, to).has_value(); }

    auto data(const VertexDescriptor& v) const { return v.data(); }
    auto data(const ArcDescriptor& a)   const { return a.data(); }

    const Graph& original() const { return _graph; }

    class VertexDescriptor {
    public:
        VertexDescriptor(typename Graph::ConstVertexDescriptor orig)
            : _orig(orig) {}

        const auto& data() const { return _orig.data(); }

        std::size_t outDegree() const { return _orig.inDegree(); }
        std::size_t inDegree()  const { return _orig.outDegree(); }

        auto outgoingArcs() const {
            return _orig.incomingArcs()
                   | std::views::transform([this](auto a) { return ArcDescriptor(a); });
        }

        auto incomingArcs() const {
            return _orig.outgoingArcs()
                   | std::views::transform([this](auto a) { return ArcDescriptor(a); });
        }

        std::vector<VertexDescriptor> adjacentVertices() const {
            std::unordered_set<VertexDescriptor> unique;
            for (auto a : outgoingArcs()) {
                auto other = a.followArcDirection(*this);
                unique.insert(*other);
            }
            return std::vector<VertexDescriptor>{unique.begin(), unique.end()};
        }

        std::vector<VertexDescriptor> incomingVertices() const {
            std::unordered_set<VertexDescriptor> unique;
            for (auto a : incomingArcs())
                unique.insert(a.from());
            return std::vector<VertexDescriptor>{unique.begin(), unique.end()};
        }

        bool operator==(const VertexDescriptor& other) const { return _orig == other._orig; }
        bool operator!=(const VertexDescriptor& other) const { return !(*this == other); }

        const typename Graph::ConstVertexDescriptor& base() const { return _orig; }

        using CustomHasherProvidedByExxIncident = VertexDescriptorHash;

    private:

        typename Graph::ConstVertexDescriptor _orig;
        friend class ArcDescriptor;
    };

    class ArcDescriptor {
    public:
        ArcDescriptor(typename Graph::ConstArcDescriptor orig)
            : _orig(orig) {}

        const auto& data() const { return _orig.data(); }

        VertexDescriptor from() const { return VertexDescriptor(_orig.to()); }
        VertexDescriptor to()   const { return VertexDescriptor(_orig.from()); }

        std::optional<VertexDescriptor> followArcDirection(const VertexDescriptor& vertex) const {
            if (vertex._orig == _orig.to())
                return VertexDescriptor(_orig.from());
            return std::nullopt;
        }

        std::optional<VertexDescriptor> otherEnd(const VertexDescriptor& vertex) const {
            if (vertex._orig == _orig.from())
                return VertexDescriptor(_orig.to());
            if (vertex._orig == _orig.to())
                return VertexDescriptor(_orig.from());
            return std::nullopt;
        }

        bool operator==(const ArcDescriptor& other) const { return _orig == other._orig; }
        bool operator!=(const ArcDescriptor& other) const { return !(*this == other); }

        const typename Graph::ConstArcDescriptor& base() const { return _orig; }

        using CustomHasherProvidedByExxIncident = VertexDescriptorHash;

    private:

        typename Graph::ConstArcDescriptor _orig;
    };

private:
    const Graph& _graph;
};

} // namespace exx::incident

#endif // EXX_TRANSPOSEDGRAPHVIEW_SIMPLE_HPP
