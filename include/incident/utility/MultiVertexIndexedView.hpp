#ifndef EXX_MULTIVERTEXINDEXEDVIEW_HPP
#define EXX_MULTIVERTEXINDEXEDVIEW_HPP

#include <unordered_map>
#include <ranges>

#include "../details/graph_concepts.hpp"

namespace exx::incident {


template<GraphConcept Graph,
         typename Hash = std::hash<typename Graph::VertexValueType>,
         typename KeyEqual = std::equal_to<typename Graph::VertexValueType>>
class MultiVertexIndexedView {
public:
    using VertexValueType = typename Graph::VertexValueType;
    using VertexDescriptor = typename Graph::VertexDescriptor;

    explicit MultiVertexIndexedView(Graph& graph) : graph_(graph)
    { rebuild(); }

    void rebuild() {
        index_.clear();
        for (auto v : graph_.vertices())
            index_.emplace(v.data(), v);
    }

    auto findVertices(const VertexValueType& key) const {
        auto equal_range = index_.equal_range(key);
        auto range = std::ranges::subrange(equal_range.first, equal_range.second);
        return range
               | std::views::transform([](const auto& pair) { return pair.second; });
    }

    bool containsVertex(const VertexValueType& key) const
    { return index_.find(key) != index_.end(); }

    size_t count(const VertexValueType& key) const
    { return index_.count(key); }

    auto begin() const { return index_.begin(); }
    auto end() const   { return index_.end(); }

    size_t size() const { return index_.size(); }
    bool empty() const  { return index_.empty(); }

    Graph& graph() { return graph_; }

private:
    Graph& graph_;
    std::unordered_multimap<VertexValueType, VertexDescriptor, Hash, KeyEqual> index_;
};

} // namespace exx::incident

#endif // EXX_MULTIVERTEXINDEXEDVIEW_HPP
