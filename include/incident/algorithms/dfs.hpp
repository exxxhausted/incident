#ifndef EXX_DFS_HPP
#define EXX_DFS_HPP

#include <vector>
#include <stack>
#include <unordered_set>

#include "../details/graph_concepts.hpp"

namespace exx::incident {

template<GraphConcept Graph,
         typename Cmp = std::less<typename Graph::VertexValueType>>
auto dfs(Graph& G, typename Graph::VertexDescriptor start)
    ->std::vector<typename Graph::VertexDescriptor>
{
    using Descriptor = typename Graph::VertexDescriptor;

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

#endif // EXX_DFS_HPP
