#ifndef EXX_BFS_HPP
#define EXX_BFS_HPP

#include <queue>
#include <optional>
#include <unordered_map>
#include <forward_list>

#include "../details/graph_concepts.hpp"

namespace exx::incident {

template<GraphConcept Graph>
class BfsTree {
    using Descriptor = Graph::ConstVertexDescriptor;
public:

    bool isReachable(Descriptor v) const { return _ht.at(v)._color == Color::Black; }

    std::optional<Descriptor> parent(Descriptor v) const {
        if(_ht.at(v)._depth == 0) return std::nullopt;
        if(_ht.at(v)._color != Color::White) return _ht.at(v)._parent;
        return std::nullopt;
    }

    std::optional<std::size_t> depth(Descriptor v) const {
        if(_ht.at(v)._color != Color::Black) return std::nullopt;
        return _ht.at(v)._depth;
    }

    std::forward_list<Descriptor> path(Descriptor v) const  {
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

    std::vector<Descriptor> layer(std::size_t d) const {
        std::vector<Descriptor> res;

        for(const auto& [v, data] : _ht)
            if(data._color == Color::Black && data._depth == d) res.push_back(v);

        return res;
    }

    const std::vector<Descriptor>& order() const { return _order; }

private:
    enum class Color {
        White,
        Gray,
        Black
    };

    struct Data {
        std::size_t _depth = 0;
        Descriptor _parent = {};
        Color _color = Color::White;
    };

    std::unordered_map<Descriptor, Data> _ht = {};
    std::vector<Descriptor> _order = {};

    template<GraphConcept G>
    friend BfsTree<G> bfs(const G& g, G::ConstVertexDescriptor start);
};

template<GraphConcept Graph>
BfsTree<Graph> bfs(const Graph& G,
                   typename Graph::ConstVertexDescriptor start)
{
    using Descriptor = typename Graph::ConstVertexDescriptor;
    using Col = BfsTree<Graph>::Color;

    BfsTree<Graph> res;

    for(auto v : G.vertices()) res._ht.insert( { v, {} } );
    res._order.reserve(G.vertexCount());

    std::queue<Descriptor> queue;

    queue.push(start);
    res._ht[start]._color = Col::Gray;

    while(!queue.empty()) {
        auto current = queue.front();
        queue.pop();

        for(auto adjV : current.adjacentVertices()) {
            if(res._ht[adjV]._color == Col::White) {
                res._ht[adjV]._color = Col::Gray;
                res._ht[adjV]._depth = res._ht[current]._depth + 1;
                res._ht[adjV]._parent = current;
                queue.push(adjV);
            }
        }

        res._order.push_back(current);
        res._ht[current]._color = Col::Black;
    }

    return res;
}

} // namespace exx::incident

#endif // EXX_BFS_HPP
