#ifndef EXX_DFS_HPP
#define EXX_DFS_HPP

#include <optional>
#include <unordered_map>
#include <forward_list>
#include <vector>
#include <functional>

#include "../details/graph_concepts.hpp"

namespace exx::incident {

template<GraphConcept Graph>
class DfsForest {
    using Descriptor = typename Graph::ConstVertexDescriptor;
public:
    bool isReachable(Descriptor v) const { return _ht.at(v)._color != Color::White; }

    std::optional<Descriptor> parent(Descriptor v) const { return _ht.at(v)._parent; }

    std::optional<std::size_t> discoveryTime(Descriptor v) const { return _ht.at(v)._discovery; }

    std::optional<std::size_t> finishTime(Descriptor v) const { return _ht.at(v)._finish; }

    auto path(Descriptor v) const
        -> std::optional<std::forward_list<Descriptor>>
    {
        if (!isReachable(v)) return std::nullopt;
        std::forward_list<Descriptor> res;
        auto cur = v;
        while (true) {
            res.push_front(cur);
            auto p = parent(cur);
            if (!p) break;
            cur = *p;
        }
        return res;
    }

    const std::vector<Descriptor>& preorder() const { return _preorder; }
    const std::vector<Descriptor>& postorder() const { return _postorder; }

private:
    enum class Color { White, Gray, Black };

    struct Data {
        std::optional<Descriptor> _parent = std::nullopt;
        std::optional<std::size_t> _discovery = std::nullopt;
        std::optional<std::size_t> _finish = std::nullopt;
        Color _color = Color::White;
    };

    std::unordered_map<Descriptor, Data> _ht;
    std::vector<Descriptor> _preorder;
    std::vector<Descriptor> _postorder;

    template<GraphConcept G>
    friend DfsForest<G> dfs(const G&, const std::vector<typename G::ConstVertexDescriptor>&);
};

template<GraphConcept Graph>
DfsForest<Graph> dfs(const Graph& G,
                     const std::vector<typename Graph::ConstVertexDescriptor>& starts)
{
    using Descriptor = typename Graph::ConstVertexDescriptor;
    using Col = typename DfsForest<Graph>::Color;

    DfsForest<Graph> res;
    for (auto v : G.vertices())
        res._ht.emplace(v, typename DfsForest<Graph>::Data{});

    std::size_t time = 0;

    std::function<void(Descriptor, std::optional<Descriptor>)> dfs_visit =
        [&](Descriptor u, std::optional<Descriptor> parent) {
            res._ht[u]._color = Col::Gray;
            res._ht[u]._discovery = ++time;
            res._ht[u]._parent = parent;
            res._preorder.push_back(u);

            for (auto v : u.adjacentVertices())
                if (res._ht[v]._color == Col::White)
                    dfs_visit(v, u);

            res._ht[u]._color = Col::Black;
            res._ht[u]._finish = ++time;
            res._postorder.push_back(u);
        };

    for (auto start : starts)
        if (res._ht[start]._color == Col::White)
            dfs_visit(start, std::nullopt);

    return res;
}

template<GraphConcept Graph>
DfsForest<Graph> dfs(const Graph& G) {
    std::vector<typename Graph::ConstVertexDescriptor> all_starts;
    all_starts.reserve(G.vertexCount());
    for (auto v : G.vertices()) all_starts.push_back(v);
    return dfs(G, all_starts);
}

template<GraphConcept Graph>
DfsForest<Graph> dfs(const Graph& G, typename Graph::ConstVertexDescriptor start)
{ return dfs(G, std::vector<typename Graph::ConstVertexDescriptor>{start}); }

} // namespace exx::incident

#endif // EXX_DFS_HPP
