#ifndef EXX_MSTPRIM_HPP
#define EXX_MSTPRIM_HPP

#include <queue>
#include <unordered_set>
#include <vector>
#include <expected>
#include <string>

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
          !std::is_void_v<VHash> &&
          std::is_copy_constructible_v<EdgeData>)
auto mstPrim(const UndirectedGraph<VertexData, EdgeData, VHash>& graph)
    -> std::expected<UndirectedGraph<VertexData, EdgeData, VHash>, PrimError>
{
    using GraphType = UndirectedGraph<VertexData, EdgeData, VHash>;
    using VertexDesc = typename GraphType::VertexDescriptor;
    using ConstVertexDesc = typename GraphType::ConstVertexDescriptor;
    if (graph.vertexCount() == 0) return GraphType{};

    GraphType mst;
    for (auto v : graph.constVertices()) mst.addVertex(v.data());

    struct QueueElement {
        EdgeData weight;
        VertexData vertexData;
        VertexData parentData;
    };
    auto cmp = [](const QueueElement& a, const QueueElement& b) {
        return a.weight > b.weight;
    };
    std::priority_queue<QueueElement, std::vector<QueueElement>, decltype(cmp)> pq(cmp);

    std::unordered_set<VertexData, VHash> visited(0, VHash{});
    VertexData startData = (*graph.constVertices().begin()).data();
    visited.insert(startData);

    ConstVertexDesc startDesc = (*graph.beginVertices());

    for (auto edge : startDesc.incidentEdges()) {
        auto otherDesc = edge.otherEnd(startDesc);
        VertexData otherData = otherDesc->data();
        if (!visited.contains(otherData))
            pq.push({edge.data(), otherData, startData});
    }

    while (!pq.empty()) {
        auto [weight, vData, parentData] = pq.top();
        pq.pop();

        if (visited.contains(vData)) continue;
        visited.insert(vData);

        auto parentDescOpt = mst.findVertex(parentData);
        auto vDescOpt = mst.findVertex(vData);
        if (!parentDescOpt || !vDescOpt) throw ("bebe");
        mst.addEdge(*parentDescOpt, *vDescOpt, weight);

        auto vDescOrigOpt = graph.findVertex(vData);
        if (!vDescOrigOpt) throw ("bubu");
        VertexDesc vDesc = *vDescOrigOpt;

        for (auto edge : vDesc.incidentEdges()) {
            auto otherDesc = edge.otherEnd(vDesc);
            VertexData otherData = otherDesc->data();
            if (!visited.contains(otherData))
                pq.push({edge.data(), otherData, vData});
        }
    }

    if (visited.size() != graph.vertexCount())
        return std::unexpected(PrimError::DisconnectedGraph);

    return mst;
}

} // namespace exx::incident

#endif // EXX_MSTPRIM_HPP
