#ifndef EXX_UNDIRECTEDGRAPHFROMMATRIX_HPP
#define EXX_UNDIRECTEDGRAPHFROMMATRIX_HPP

#include "../undirected/UndirectedGraph.hpp"

#include <expected>
#include <unordered_map>

namespace exx::incident {

enum class GraphBuildingError {
    NullPointer,
    ZeroSize,
    NonSquareMatrix
};

inline std::string to_string(GraphBuildingError error) noexcept {
    using enum GraphBuildingError;
    switch (error) {
    case GraphBuildingError::NullPointer:          return "NullPointer: input matrix pointer is null";
    case GraphBuildingError::ZeroSize:             return "ZeroSize: matrix size is zero";
    case GraphBuildingError::NonSquareMatrix:      return "NonSquareMatrix: matrix is not square";
    default:                                       return "Unknown error";
    }
}

template<typename EdgeData>
auto buildUndirectedGraph(const EdgeData* matrix, std::size_t n)
    ->std::expected<UndirectedGraph<std::size_t, EdgeData>, GraphBuildingError>
{
    if (!matrix) return std::unexpected(GraphBuildingError::NullPointer);
    if (n == 0) return std::unexpected(GraphBuildingError::ZeroSize);

    UndirectedGraph<std::size_t, EdgeData> g;
    std::unordered_map<std::size_t,
                       typename UndirectedGraph<std::size_t, EdgeData>::VertexDescriptor> ht;

    for (std::size_t i = 0; i < n; ++i) {
        auto desc = g.addVertex(i);
        ht.emplace(i, desc);
    }

    for (std::size_t i = 0; i < n; ++i) {
        for (std::size_t j = i + 1; j < n; ++j) {
            auto val = matrix[i * n + j];
            if (val != EdgeData{})
                g.addEdge(ht[i], ht[j], val);
        }
    }

    return g;
}

template<typename EdgeData>
auto buildUndirectedGraph(const std::vector<std::vector<EdgeData>>& matrix)
    ->std::expected<UndirectedGraph<std::size_t, EdgeData>, GraphBuildingError>
{
    if (matrix.empty())
        return std::unexpected(GraphBuildingError::ZeroSize);

    std::size_t n = matrix.size();
    std::vector<EdgeData> flat;
    flat.reserve(n * n);
    for (const auto& row : matrix) {
        if (row.size() != n) return std::unexpected(GraphBuildingError::NonSquareMatrix);
        flat.insert(flat.end(), row.begin(), row.end());
    }
    return buildUndirectedGraph<EdgeData>(flat.data(), n);
}

inline auto buildUndirectedGraph(const std::vector<bool> matrix, std::size_t n)
    ->std::expected<UndirectedGraph<std::size_t, void>, GraphBuildingError>
{
    if (n == 0) return std::unexpected(GraphBuildingError::ZeroSize);

    UndirectedGraph<std::size_t, void> g;
    std::unordered_map<std::size_t,
                       typename UndirectedGraph<std::size_t, void>::VertexDescriptor> ht;

    for (std::size_t i = 0; i < n; ++i) {
        auto desc = g.addVertex(i);
        ht.emplace(i, desc);
    }

    for (std::size_t i = 0; i < n; ++i) {
        for (std::size_t j = i + 1; j < n; ++j) {
            auto val = matrix[i * n + j];
            if (val) g.addEdge(ht[i], ht[j]);
        }
    }

    return g;
}

inline auto buildUndirectedGraph(const std::vector<std::vector<bool>>& matrix)
    ->std::expected<UndirectedGraph<std::size_t, void>, GraphBuildingError>
{
    if (matrix.empty())
        return std::unexpected(GraphBuildingError::ZeroSize);

    std::size_t n = matrix.size();
    std::vector<bool> flat;
    flat.reserve(n * n);
    for (const auto& row : matrix) {
        if (row.size() != n) return std::unexpected(GraphBuildingError::NonSquareMatrix);
        flat.insert(flat.end(), row.begin(), row.end());
    }
    return buildUndirectedGraph(flat, n);
}

} // namespace exx::incident

#endif // EXX_UNDIRECTEDGRAPHFROMMATRIX_HPP
