#ifndef EXX_VERTEXSEARCHACCELERATOR_HPP
#define EXX_VERTEXSEARCHACCELERATOR_HPP

#include <unordered_map>
#include <ranges>
#include <type_traits>
#include "../details/graph_concepts.hpp"

namespace exx::incident {

template<Graph G,
         typename Descriptor = typename G::VertexDescriptor,
         typename VertexHash = std::hash<typename G::VertexValueType>,
         typename KeyEqual = std::equal_to<typename G::VertexValueType>>
class VertexSearchAccelerator {
    static_assert(std::is_same_v<Descriptor, typename G::VertexDescriptor> ||
                      std::is_same_v<Descriptor, typename G::ConstVertexDescriptor>,
                  "Descriptor must be VertexDescriptor or ConstVertexDescriptor");

public:
    using DescriptorType = Descriptor;
    using MapType = std::unordered_multimap<typename G::VertexValueType, Descriptor, VertexHash, KeyEqual>;
    using const_iterator = typename MapType::const_iterator;
    using iterator_range = std::ranges::subrange<const_iterator>;

    VertexSearchAccelerator() = default;
    explicit VertexSearchAccelerator(G& graph) { rebuild(graph); }

    void add(Descriptor v) { _mht.emplace(v.data(), v); }

    void remove(Descriptor v) {
        auto range = _mht.equal_range(v.data());
        for (auto it = range.first; it != range.second; ++it) {
            if (it->second == v) {
                _mht.erase(it);
                return;
            }
        }
    }

    void update(Descriptor v, const typename G::VertexValueType& oldData) {
        auto range = _mht.equal_range(oldData);
        for (auto it = range.first; it != range.second; ++it) {
            if (it->second == v) {
                _mht.erase(it);
                break;
            }
        }
        _mht.emplace(v.data(), v);
    }

    void rebuild(G& graph) {
        clear();
        for (auto v : graph.vertices())
            _mht.emplace(v.data(), Descriptor(v));
    }

    void clear() { _mht.clear(); }

    iterator_range find(const typename G::VertexValueType& data) const {
        auto range = _mht.equal_range(data);
        return iterator_range(range.first, range.second);
    }

    bool contains(const typename G::VertexValueType& data) const
    { return _mht.find(data) != _mht.end(); }

    std::size_t size() const { return _mht.size(); }
    bool empty() const { return _mht.empty(); }

private:
    MapType _mht;
};

} // namespace exx::incident

#endif // EXX_VERTEXSEARCHACCELERATOR_HPP
