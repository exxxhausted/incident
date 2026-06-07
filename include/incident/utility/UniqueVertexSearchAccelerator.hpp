#ifndef EXX_UNIQUEVERTEXSEARCHACCELERATOR_HPP
#define EXX_UNIQUEVERTEXSEARCHACCELERATOR_HPP

#include <unordered_map>
#include <optional>
#include <type_traits>
#include "../details/graph_concepts.hpp"

namespace exx::incident {

template<Graph G,
         typename Descriptor = typename G::VertexDescriptor,
         typename VertexHash = std::hash<typename G::VertexValueType>>
class UniqueVertexSearchAccelerator {

    static_assert(std::is_same_v<Descriptor, typename G::VertexDescriptor> ||
                  std::is_same_v<Descriptor, typename G::ConstVertexDescriptor>,
                  "Descriptor must be VertexDescriptor or ConstVertexDescriptor");

public:
    using DescriptorType = Descriptor;

    UniqueVertexSearchAccelerator() = default;
    explicit UniqueVertexSearchAccelerator(G& g) { rebuild(g); }

    void add(Descriptor v) { _ht.emplace(v.data(), v); }

    void remove(Descriptor v) { _ht.erase(v.data()); }

    void update(Descriptor v, const typename G::VertexValueType& oldData) {
        auto it = _ht.find(oldData);
        if (it != _ht.end() && it->second == v)
            _ht.erase(it);
        _ht.emplace(v.data(), v);
    }

    void clear() { _ht.clear(); }

    void rebuild(G& g) {
        clear();
        for (auto v : g.vertices())
            _ht.emplace(v.data(), v);
    }

    std::optional<Descriptor> map(const typename G::VertexValueType& data) const {
        auto it = _ht.find(data);
        if (it != _ht.end()) return it->second;
        return std::nullopt;
    }

    bool contains(const typename G::VertexValueType& data) const
    { return _ht.find(data) != _ht.end(); }

    std::size_t size() const { return _ht.size(); }
    bool empty() const { return _ht.empty(); }

private:
    std::unordered_map<typename G::VertexValueType,
                       Descriptor,
                       VertexHash> _ht;
};

} // namespace exx::incident

#endif // EXX_UNIQUEVERTEXSEARCHACCELERATOR_HPP
