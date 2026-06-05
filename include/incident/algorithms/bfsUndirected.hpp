#ifndef EXX_BFSUNDIRECTED_HPP
#define EXX_BFSUNDIRECTED_HPP

#include <queue>
#include <unordered_set>

#include "../graph_concepts.hpp"

namespace exx::incident {

template<TraversableGraph Graph,
         typename Cmp = std::less<typename Graph::VertexValueType>>
auto bfs(Graph& G, typename Graph::VertexDescriptor start)
    ->std::vector<typename Graph::VertexDescriptor>
{
    using Descriptor = typename Graph::VertexDescriptor;

    std::queue<Descriptor> queue;
    std::unordered_set<Descriptor> visited;
    std::vector<Descriptor> res;

    queue.push(start);
    visited.insert(start);

    while(!queue.empty()) {
        auto current = queue.front();
        queue.pop();

        res.push_back(current);

        for(auto adjV : current.template adjacentVertices<Cmp>()) {
            if(!visited.contains(adjV)) {
                queue.push(adjV);
                visited.insert(adjV);
            }
        }
    }

    return res;
}

} // namespace exx::incident

#endif // EXX_BFSUNDIRECTED_HPP
