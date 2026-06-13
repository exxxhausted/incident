#ifndef EXX_BFS_HPP
#define EXX_BFS_HPP

#include <queue>
#include <optional>
#include <unordered_map>
#include <forward_list>
#include <vector>

#include "../details/graph_concepts.hpp"

namespace exx::incident {

template<GraphConcept Graph>
class BfsForest {
    using Descriptor = typename Graph::ConstVertexDescriptor;
public:
    bool isReachable(Descriptor v) const { return _ht.at(v)._color == Color::Black; }

    std::optional<Descriptor> parent(Descriptor v) const { return _ht.at(v)._parent; }

    std::optional<Descriptor> root(Descriptor v) const {
        if (_ht.at(v)._color == Color::White) return std::nullopt;
        return _ht.at(v)._root;
    }

    std::optional<std::size_t> depth(Descriptor v) const { return _ht.at(v)._depth; }

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

    const std::vector<Descriptor>& order() const { return _order; }

    std::vector<Descriptor> roots() const {
        std::vector<Descriptor> r;

        for (const auto& [v, data] : _ht)
            if (data._color == Color::Black && !data._parent.has_value())
                r.push_back(v);

        return r;
    }

private:
    enum class Color { White, Gray, Black };

    struct Data {
        std::optional<std::size_t> _depth = std::nullopt;
        std::optional<Descriptor> _parent = std::nullopt;
        std::optional<Descriptor> _root   = std::nullopt;
        Color _color = Color::White;
    };

    std::unordered_map<Descriptor, Data> _ht;
    std::vector<Descriptor> _order;

    template<GraphConcept G>
    friend BfsForest<G> bfs(const G&, const std::vector<typename G::ConstVertexDescriptor>&);

};

template<GraphConcept Graph>
BfsForest<Graph> bfs(const Graph& G,
                     const std::vector<typename Graph::ConstVertexDescriptor>& starts)
{
    using Descriptor = typename Graph::ConstVertexDescriptor;
    using Col = typename BfsForest<Graph>::Color;

    BfsForest<Graph> res;
    for (auto v : G.vertices())
        res._ht.emplace(v, typename BfsForest<Graph>::Data{});

    res._order.reserve(G.vertexCount());
    std::queue<Descriptor> queue;

    for (auto start : starts) {
        if (res._ht[start]._color != Col::White) continue;

        queue.push(start);
        res._ht[start]._color = Col::Gray;
        res._ht[start]._depth = 0;
        res._ht[start]._root  = start;

        while (!queue.empty()) {
            auto cur = queue.front();
            queue.pop();

            for (auto adj : cur.adjacentVertices()) {
                if (res._ht[adj]._color == Col::White) {
                    res._ht[adj]._color = Col::Gray;
                    res._ht[adj]._depth = *res._ht[cur]._depth + 1;
                    res._ht[adj]._parent = cur;
                    res._ht[adj]._root   = res._ht[cur]._root;
                    queue.push(adj);
                }
            }

            res._order.push_back(cur);
            res._ht[cur]._color = Col::Black;
        }
    }

    return res;
}

template<GraphConcept Graph>
BfsForest<Graph> bfs(const Graph& G, typename Graph::ConstVertexDescriptor start) {
    return bfs(G, std::vector<typename Graph::ConstVertexDescriptor>{start});
}

template<GraphConcept Graph>
BfsForest<Graph> bfs(const Graph& G) {
    std::vector<typename Graph::ConstVertexDescriptor> all_starts;
    all_starts.reserve(G.vertexCount());
    for (auto v : G.vertices()) all_starts.push_back(v);
    return bfs(G, all_starts);
}

} // namespace exx::incident

#endif // EXX_BFS_HPP
