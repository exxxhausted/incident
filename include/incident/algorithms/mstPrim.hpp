#ifndef EXX_MSTPRIM_HPP
#define EXX_MSTPRIM_HPP

#include <queue>
#include <unordered_set>
#include <vector>
#include <expected>

#include "../UndirectedGraph.hpp"

namespace exx::incident {

enum class PrimError {
    DisconnectedGraph
};

inline std::string to_string(PrimError e) {
    switch (e) {
    case PrimError::DisconnectedGraph:
        return "Граф не является связным";
    }
    return "Неизвестная ошибка";
}

template<typename VertexData,
         typename EdgeData,
         typename VHash>
    requires (!std::is_void_v<EdgeData> &&
            std::is_copy_constructible_v<EdgeData>)
auto mstPrim(const UndirectedGraph<VertexData, EdgeData, VHash>& graph)
    -> std::expected<UndirectedGraph<VertexData, EdgeData, VHash>, PrimError>
{
    using GraphType = UndirectedGraph<VertexData, EdgeData, VHash>;
    using ConstVertexDesc = typename GraphType::ConstVertexDescriptor;

    if (graph.vertexCount() == 0) return GraphType{};

    GraphType mst;
    for (auto v : graph.constVertices()) mst.addVertex(v.data());

    struct QueueElement {
        EdgeData weight;
        ConstVertexDesc vertex;
        ConstVertexDesc parent;
    };
    auto cmp = [](const QueueElement& a, const QueueElement& b) {
        return a.weight > b.weight;
    };
    std::priority_queue<QueueElement, std::vector<QueueElement>, decltype(cmp)> pq(cmp);

    std::unordered_set<ConstVertexDesc, typename GraphType::VertexDescriptorHash> visited;

    ConstVertexDesc startDesc = *graph.constVertices().begin();
    visited.insert(startDesc);

    for (auto edge : startDesc.incidentEdges()) {
        auto otherDesc = edge.otherEnd(startDesc); // always not-nullopt optional
        if (!visited.contains(*otherDesc)) pq.push({edge.data(), *otherDesc, startDesc});
    }

    while (!pq.empty()) {
        auto [weight, vDesc, parentDesc] = pq.top();
        pq.pop();

        if (visited.contains(vDesc)) continue;
        visited.insert(vDesc);

        mst.addEdge(parentDesc.data(), vDesc.data(), weight);

        for (auto edge : vDesc.incidentEdges()) {
            auto otherDesc = edge.otherEnd(vDesc); // always not-nullopt optional
            if (!visited.contains(*otherDesc))
                pq.push({edge.data(), *otherDesc, vDesc});
        }
    }

    if (visited.size() != graph.vertexCount())
        return std::unexpected(PrimError::DisconnectedGraph);

    return mst;
}

} // namespace exx::incident

#endif // EXX_MSTPRIM_HPP
