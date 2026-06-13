#ifndef EXX_SCCKOSARAJU_HPP
#define EXX_SCCKOSARAJU_HPP

#include "../details/graph_concepts.hpp"
#include "../utility/TransposedGraphView.hpp"
#include "dfs.hpp"

#include <unordered_set>
#include <vector>
#include <algorithm>

namespace exx::incident {

template<DirectedGraphConcept Graph>
class StronglyConnectedComponents;

template<DirectedGraphConcept Graph>
class StronglyConnectedComponent {
    using Descriptor = typename Graph::ConstVertexDescriptor;
public:

    bool contains(Descriptor v) const { return _vertices.contains(v); }

    std::size_t size() const { return _vertices.size(); }

    auto vertices() const
        ->const std::unordered_set<Descriptor>&
    { return _vertices; }

private:

    std::unordered_set<Descriptor> _vertices = {};

    template<DirectedGraphConcept G>
    friend StronglyConnectedComponents<G> sccKosaraju(const G&);
};

template<DirectedGraphConcept Graph>
class StronglyConnectedComponents {
    using Descriptor = typename Graph::ConstVertexDescriptor;
public:

    auto componentOf(Descriptor v) const
        ->std::optional<StronglyConnectedComponent<Graph>>
    {
        for(const auto& scc : _scc)
            if(scc.contains(v))
                return scc;
        return std::nullopt;
    }

    bool areStronglyConnected(Descriptor u, Descriptor v) const {
        auto scc_u = componentOf(u);
        if(!scc_u.has_value()) return false;
        if(scc_u->contains(v)) return true;
        return false;
    }

    std::size_t componentCount() const { return _scc.size(); }

    auto components() const
        ->const std::vector<StronglyConnectedComponent<Graph>>&
    { return _scc; }

private:

    std::vector<StronglyConnectedComponent<Graph>> _scc = {};

    template<DirectedGraphConcept G>
    friend StronglyConnectedComponents<G> sccKosaraju(const G&);

};

template<DirectedGraphConcept Graph>
StronglyConnectedComponents<Graph> sccKosaraju(const Graph& G) {
    auto forest = dfs(G);

    std::vector<typename Graph::ConstVertexDescriptor> vertices;
    vertices.reserve(G.vertexCount);
    for (auto v : G.vertices())
        vertices.push_back(v);

    std::ranges::sort(vertices, [&](auto lhs, auto rhs) {
        return *forest.finishTime(lhs) > *forest.finishTime(rhs);
    });

    std::vector<typename TransposedGraphView<Graph>::VertexDescriptor> order;
    order.reserve(G.vertexCount());
    std::ranges::transform(vertices,
                           std::back_inserter(order),
                           [](auto v){ return typename TransposedGraphView<Graph>::VertexDescriptor(v); });

    auto G_t = TransposedGraphView(G);

    auto transposedForest = dfs(G_t, order);

    std::unordered_map<typename Graph::ConstVertexDescriptor,
                       StronglyConnectedComponent<Graph>> rootToComp;

    for (auto v : G_t.vertices()) {

        auto cur = v;
        while (auto p = transposedForest.parent(cur)) cur = *p;

        rootToComp[cur.base()]._vertices.insert(v.base());
    }

    StronglyConnectedComponents<Graph> result;
    result._scc.reserve(rootToComp.size());
    for (auto& [root, comp] : rootToComp)
        result._scc.push_back(std::move(comp));
    return result;
}

} // namespace exx::incident

#endif // EXX_SCCKOSARAJU_HPP
