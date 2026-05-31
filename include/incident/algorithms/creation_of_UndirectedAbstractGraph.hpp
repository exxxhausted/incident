#ifndef EXX_CREATION_OF_UNDIRECTEDABSTRACTGRAPH_HPP
#define EXX_CREATION_OF_UNDIRECTEDABSTRACTGRAPH_HPP

#include <vector>

#include "../UndirectedAbstractGraph.hpp"
#include "../infrastructure/MatrixOwningView.hpp"
#include "../infrastructure/MatrixView.hpp"

namespace exx::incident {

namespace impl {

template<typename VertexData, typename EdgeData>
static UndirectedAbstractGraph<VertexData, EdgeData>
make_graph_from_matrix(const MatrixView<EdgeData>& mat, EdgeData noEdgeValue) {
    using std::size_t;
    size_t n = mat.rows();
    if (n != mat.cols()) {
        throw std::invalid_argument("Matrix must be square for graph adjacency");
    }

    UndirectedAbstractGraph<VertexData, EdgeData> g;
    std::vector<typename UndirectedAbstractGraph<VertexData, EdgeData>::VertexIterator> vertIters;
    vertIters.reserve(n);

    for (size_t i = 0; i < n; ++i) {
        vertIters.push_back(g.addVertex(VertexData{static_cast<VertexData>(i)}));
    }

    for (size_t i = 0; i < n; ++i) {
        for (size_t j = i + 1; j < n; ++j) {
            EdgeData w = mat(i, j);
            if (w != noEdgeValue) {
                g.addEdge(vertIters[i], vertIters[j], w);
            }
        }
    }
    return g;
}

} // namespace impl

template<typename VertexData, typename EdgeData>
UndirectedAbstractGraph<VertexData, EdgeData>
make_graph_from_matrix(std::initializer_list<std::initializer_list<EdgeData>> list, EdgeData noEdgeValue = EdgeData{}) {
    MatrixOwningView<EdgeData> mat(list);
    MatrixView<EdgeData> view(mat.data(), mat.rows(), mat.cols());
    return impl::make_graph_from_matrix<VertexData, EdgeData>(view, noEdgeValue);
}

template<typename VertexData, typename EdgeData>
UndirectedAbstractGraph<VertexData, EdgeData>
make_graph_from_matrix(std::vector<std::vector<EdgeData>> mat, EdgeData noEdgeValue = EdgeData{}) {
    if (mat.empty()) {
        MatrixView<EdgeData> emptyView(nullptr, 0, 0);
        return impl::make_graph_from_matrix<VertexData, EdgeData>(emptyView, noEdgeValue);
    }
    std::size_t rows = mat.size();
    std::size_t cols = mat[0].size();
    for (const auto& row : mat) {
        if (row.size() != cols)
            throw std::invalid_argument("Non-rectangular matrix");
    }
    auto range = mat | std::views::join;
    std::vector<EdgeData> flat(range.begin(), range.end());
    MatrixView<EdgeData> view(flat.data(), rows, cols);
    return impl::make_graph_from_matrix<VertexData, EdgeData>(view, noEdgeValue);
}


template<typename VertexData, typename EdgeData>
UndirectedAbstractGraph<VertexData, EdgeData>
make_graph_from_matrix(const MatrixView<EdgeData>& mat, EdgeData noEdgeValue = EdgeData{}) {
    return impl::make_graph_from_matrix<VertexData, EdgeData>(mat, noEdgeValue);
}

template<typename VertexData, typename EdgeData, MatrixLike M>
UndirectedAbstractGraph<VertexData, EdgeData>
make_graph_from_matrix(M&& mat, EdgeData noEdgeValue = EdgeData{}) {
    return impl::make_graph_from_matrix<VertexData, EdgeData>(
        MatrixView<EdgeData>(std::forward<M>(mat)), noEdgeValue);
}

} // namespace exx::incident

#endif // EXX_CREATION_OF_UNDIRECTEDABSTRACTGRAPH_HPP
