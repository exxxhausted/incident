#ifndef EXX_MSTPRIM_HPP
#define EXX_MSTPRIM_HPP

#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <stdexcept>

#include "../UndirectedAbstractGraph.hpp"

namespace exx::incident {

template<typename VertexData,
         typename EdgeData,
         typename VHash = std::hash<VertexData>,
         typename VEqual = std::equal_to<VertexData>>
UndirectedAbstractGraph<VertexData, EdgeData>
mstPrim(const UndirectedAbstractGraph<VertexData, EdgeData>& graph,
             VHash vHash = VHash{},
             VEqual vEqual = VEqual{})
{
    using _Graph = UndirectedAbstractGraph<VertexData, EdgeData>;

    using _СHashMap = std::unordered_map<VertexData,
                                       typename _Graph::ConstVertexIterator,
                                       VHash,
                                       VEqual>;

    using _HashMap = std::unordered_map<VertexData,
                                        typename _Graph::VertexIterator,
                                        VHash,
                                        VEqual>;

    {
        std::unordered_set<VertexData, VHash, VEqual> uniqueChecker(0, vHash, vEqual);
        for (auto v : graph.vertices()) {
            const VertexData& data = v.data();
            if (!uniqueChecker.insert(data).second)
                throw std::invalid_argument("mstPrim: VertexData values must be unique in the graph");
        }
    }

    if (graph.vertexCount() == 0) return _Graph{};

    _СHashMap originalVerticesIteratorsCollection(0, vHash, vEqual);

    for (auto vit = graph.cbeginVertices(); vit != graph.cendVertices(); ++vit)
        originalVerticesIteratorsCollection[(*vit).data()] = vit;

    _Graph mst;
    _HashMap mstVerticesIteratorsCollection(0, vHash, vEqual);

    for (const auto& [vData, _] : originalVerticesIteratorsCollection) {
        VertexData id = vData;
        mstVerticesIteratorsCollection[id] = mst.addVertex(id);
    }

    if (graph.edgeCount() == 0) return mst;

    struct _QueueEl {
        EdgeData _eData;
        VertexData _1;
        VertexData _2;
    };

    auto cmp = [](const _QueueEl& a, const _QueueEl& b) { return a._eData > b._eData; };

    std::priority_queue<_QueueEl, std::vector<_QueueEl>, decltype(cmp)> pq(cmp);

    std::unordered_set<VertexData, VHash, VEqual> visited(0, vHash, vEqual);
    VertexData startData = originalVerticesIteratorsCollection.begin()->first;
    visited.insert(startData);

    auto startIt = originalVerticesIteratorsCollection[startData];
    for (auto edge : (*startIt).incidentEdges()) {
        auto other = edge.otherEnd(startIt);
        VertexData otherData = (*other).data();
        EdgeData w = edge.data();
        pq.push( { w, otherData, startData } );
    }

    while (!pq.empty()) {
        auto [w, vData, parentData] = pq.top();
        pq.pop();
        if (visited.contains(vData)) continue;

        visited.insert(vData);
        mst.addEdge(mstVerticesIteratorsCollection[parentData],
                    mstVerticesIteratorsCollection[vData],
                    w);

        auto vIt = originalVerticesIteratorsCollection[vData];
        for (auto edge : (*vIt).incidentEdges()) {
            auto other = edge.otherEnd(vIt);
            VertexData otherData = (*other).data();
            if (!visited.contains(otherData))
                pq.push( { edge.data(), otherData, vData } );
        }
    }

    if (visited.size() != graph.vertexCount())
        throw std::logic_error("Graph is disconnected, cannot build MST");

    return mst;
}

} // namespace exx::incident

#endif // EXX_MSTPRIM_HPP
