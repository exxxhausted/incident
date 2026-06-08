#ifndef EXX_UNIQUEINDEXEDGRAPHVIEW_HPP
#define EXX_UNIQUEINDEXEDGRAPHVIEW_HPP

#include <unordered_map>
#include <optional>

#include "../details/graph_concepts.hpp"

namespace exx::incident {

template<GraphConcept Graph,
         typename Hash = std::hash<typename Graph::VertexValueType>,
         typename KeyEqual = std::equal_to<typename Graph::VertexValueType>>
class UniqueIndexedGraphView {
public:
    using VertexValueType = typename Graph::VertexValueType;
    using ConstVertexDescriptor = typename Graph::ConstVertexDescriptor;

    explicit UniqueIndexedGraphView(const Graph& graph) : graph_(&graph)
    { rebuild(); }

    void rebuild() {
        index_.clear();
        for (auto v : graph_->vertices())
            index_.emplace(v.data(), v);
    }

    std::optional<ConstVertexDescriptor> findVertex(const VertexValueType& key) const {
        auto it = index_.find(key);
        if (it != index_.end()) return it->second;
        return std::nullopt;
    }

    bool containsVertex(const VertexValueType& key) const
    { return findVertex(key).has_value(); }

    auto begin() const { return index_.begin(); }
    auto end() const   { return index_.end(); }

    size_t size() const { return index_.size(); }
    bool empty() const  { return index_.empty(); }

    const Graph& graph() const { return *graph_; }

private:
    const Graph* graph_;
    std::unordered_map<VertexValueType, ConstVertexDescriptor, Hash, KeyEqual> index_;
};

} // namespace exx::incident

#endif // EXX_UNIQUEINDEXEDGRAPHVIEW_HPP
