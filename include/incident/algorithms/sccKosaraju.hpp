#ifndef EXX_SCCKOSARAJU_HPP
#define EXX_SCCKOSARAJU_HPP

#include "../details/graph_concepts.hpp"
#include "../utility/TransposedGraphView.hpp"
#include "dfs.hpp"

#include <unordered_set>
#include <vector>
#include <stack>
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
    for (auto v : G.vertices())
        vertices.push_back(v);

    std::ranges::sort(vertices, [&](auto lhs, auto rhs) {
        return *forest.finishTime(lhs) > *forest.finishTime(rhs);
    });

    auto G_t = TransposedGraphView(G);

    std::unordered_set<typename Graph::ConstVertexDescriptor> visited;
    std::vector<StronglyConnectedComponent<Graph>> components;

    for (auto start : vertices) {
        if (visited.contains(start)) continue;

        StronglyConnectedComponent<Graph> comp;
        std::stack<typename Graph::ConstVertexDescriptor> stk;
        stk.push(start);
        visited.insert(start);

        while (!stk.empty()) {
            auto u = stk.top();
            stk.pop();
            comp._vertices.insert(u);

            auto u_wrapped = typename TransposedGraphView<Graph>::VertexDescriptor(u);
            for (auto w_wrapped : u_wrapped.adjacentVertices()) {
                auto w = w_wrapped.base();
                if (!visited.contains(w)) {
                    visited.insert(w);
                    stk.push(w);
                }
            }
        }
        components.push_back(std::move(comp));
    }

    StronglyConnectedComponents<Graph> result;
    result._scc = std::move(components);
    return result;
}

} // namespace exx::incident

#endif // EXX_SCCKOSARAJU_HPP
