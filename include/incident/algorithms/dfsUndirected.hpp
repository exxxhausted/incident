#ifndef EXX_DFSUNDIRECTED_HPP
#define EXX_DFSUNDIRECTED_HPP

#include <stack>
#include <unordered_set>

#include "../UndirectedAbstractGraph.hpp"

namespace exx::incident {

template<typename VD, typename ED, typename Cmp = std::less<VD>>
auto dfs(UndirectedAbstractGraph<VD, ED>& G,
         typename UndirectedAbstractGraph<VD, ED>::VertexDescriptor start)
    ->std::vector<typename UndirectedAbstractGraph<VD, ED>::VertexDescriptor>
{
    using Descriptor = typename UndirectedAbstractGraph<VD, ED>::VertexDescriptor;

    std::stack<Descriptor> stack;
    std::unordered_set<Descriptor> visited;
    std::vector<Descriptor> res;

    stack.push(start);

    while(!stack.empty()) {
        auto current = stack.top();
        stack.pop();

        if(!visited.contains(current)) {
            visited.insert(current);

            res.push_back(current);

            for(auto adjV : current.template adjacentVertices<Cmp>())
                if(!visited.contains(adjV)) stack.push(adjV);
        }
    }

    return res;
}

} // namespace exx::incident

#endif // EXX::DFSUNDIRECTED_HPP
