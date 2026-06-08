#ifndef EXX_UNDIRECTEDGRAPHTOMATRIX_HPP
#define EXX_UNDIRECTEDGRAPHTOMATRIX_HPP

#include "../undirected/UndirectedGraph.hpp"
#include "../details/Matrix.hpp"

namespace exx::incident {

template<typename VertexData>
Matrix<bool> toAdjacencyMatrix(const UndirectedGraph<VertexData, void>& g) {
    const std::size_t n = g.vertexCount();
    Matrix<bool> mat(n, n);

    for (std::size_t i = 0; i < n; ++i)
        for (std::size_t j = 0; j < n; ++j)
            mat(i, j) = false;

    for (auto v : g.vertices()) {
        for (auto e : v.incidentEdges()) {
            auto u = *e.otherEnd(v);
            mat(v.data(), u.data()) = true;
            mat(u.data(), v.data()) = true;
        }
    }
    return mat;
}

template<typename VertexData, typename EdgeData>
    requires (!std::is_void_v<EdgeData>)
Matrix<EdgeData> toAdjacencyMatrix(const UndirectedGraph<VertexData, EdgeData>& g,
                                   EdgeData default_value = EdgeData{}) {
    const std::size_t n = g.vertexCount();
    Matrix<EdgeData> mat(n, n);

    for (std::size_t i = 0; i < n; ++i)
        for (std::size_t j = 0; j < n; ++j)
            mat(i, j) = default_value;

    for (auto v : g.vertices()) {
        for (auto e : v.incidentEdges()) {
            auto u = *e.otherEnd(v);
            mat(v.data(), u.data()) = e.data();
            mat(u.data(), v.data()) = e.data();
        }
    }
    return mat;
}

} // namespace exx::incident

#endif // EXX_UNDIRECTEDGRAPHTOMATRIX_HPP
