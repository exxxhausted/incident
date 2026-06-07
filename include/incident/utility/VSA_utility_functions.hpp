#ifndef EXX_VSAUTILITYFUNCTIONS_HPP
#define EXX_VSAUTILITYFUNCTIONS_HPP

#include "../details/graph_concepts.hpp"

namespace exx::incident {

template<Graph G, typename Acc>
auto add_vertex(G& g, Acc& acc, const typename G::VertexValueType& data) {
    auto v = g.addVertex(data);
    acc.add(v);
    return v;
}

template<Graph G, typename Acc>
    requires (!std::is_same_v<typename Acc::DescriptorType,
                              typename G::ConstVertexDescriptor>)
void remove_vertex(G& g, Acc& acc, typename Acc::DescriptorType v) {
    acc.remove(v);
    g.removeVertex(v);
}

} // namespace exx::incident

#endif // EXX_VSAUTILITYFUNCTIONS_HPP
