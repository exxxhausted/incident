#ifndef EXX_BFSUNDIRECTED_HPP
#define EXX_BFSUNDIRECTED_HPP

#include <queue>
#include <unordered_set>

#include "../UndirectedAbstractGraph.hpp"

namespace exx::incident {

template<typename VD, typename ED, typename Cmp = std::less<VD>>
auto bfs(UndirectedAbstractGraph<VD, ED>& G,
         typename UndirectedAbstractGraph<VD, ED>::VertexDescriptor start)
    ->std::vector<typename UndirectedAbstractGraph<VD, ED>::VertexDescriptor>
{
    using Descriptor = typename UndirectedAbstractGraph<VD, ED>::VertexDescriptor;

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
