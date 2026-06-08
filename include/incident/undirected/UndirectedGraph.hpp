#ifndef EXX_UNDIRECTEDGRAPH_HPP
#define EXX_UNDIRECTEDGRAPH_HPP

#include <optional>
#include <expected>

#include "UndirectedMultiGraph.hpp"
#include "../details/Matrix.hpp"

namespace exx::incident {

enum class GraphBuildingError {
    NullPointer,
    EmptyVector,
    ZeroSize,
    NonSquareMatrix
};

inline std::string to_string(GraphBuildingError error) noexcept {
    using enum GraphBuildingError;
    switch (error) {
    case GraphBuildingError::NullPointer:          return "NullPointer: input matrix pointer is null";
    case GraphBuildingError::EmptyVector:          return "EmptVector: there are no data in vector";
    case GraphBuildingError::ZeroSize:             return "ZeroSize: matrix size is zero";
    case GraphBuildingError::NonSquareMatrix:      return "NonSquareMatrix: matrix is not square";
    default:                                       return "Unknown error";
    }
}

template<typename VertexData, typename EdgeData>
class UndirectedGraph {
public:

    using VertexValueType = VertexData;
    using EdgeValueType   = EdgeData;

    using VertexDescriptor      = typename UndirectedMultiGraph<VertexData, EdgeData>::VertexDescriptor;
    using ConstVertexDescriptor = typename UndirectedMultiGraph<VertexData, EdgeData>::ConstVertexDescriptor;
    using VertexDescriptorHash  = typename UndirectedMultiGraph<VertexData, EdgeData>::VertexDescriptorHash;

    using EdgeDescriptor        = typename UndirectedMultiGraph<VertexData, EdgeData>::EdgeDescriptor;
    using ConstEdgeDescriptor   = typename UndirectedMultiGraph<VertexData, EdgeData>::ConstEdgeDescriptor;
    using EdgeDescriptorHash    = typename UndirectedMultiGraph<VertexData, EdgeData>::EdgeDescriptorHash;

    using VertexIterator        = typename UndirectedMultiGraph<VertexData, EdgeData>::VertexIterator;
    using ConstVertexIterator   = typename UndirectedMultiGraph<VertexData, EdgeData>::ConstVertexIterator;
    using EdgeIterator          = typename UndirectedMultiGraph<VertexData, EdgeData>::EdgeIterator;
    using ConstEdgeIterator     = typename UndirectedMultiGraph<VertexData, EdgeData>::ConstEdgeIterator;

    UndirectedGraph() = default;

    explicit UndirectedGraph(const UndirectedPseudoGraph<VertexData, EdgeData>& graph)
        : _multiGraph(graph) {}

    UndirectedGraph(const UndirectedGraph&) = default;
    UndirectedGraph(UndirectedGraph&&) noexcept = default;

    UndirectedGraph& operator=(const UndirectedGraph&) = default;
    UndirectedGraph& operator=(UndirectedGraph&&) noexcept = default;

    void swap(UndirectedGraph& other) noexcept { _multiGraph.swap(other._multiGraph); }

    template<typename... Args>
    VertexDescriptor emplaceVertex(Args&&... args)
    { return _multiGraph.emplaceVertex(std::forward<Args>(args)...); }

    VertexDescriptor addVertex(const VertexData& data)
    { return _multiGraph.addVertex(data); }

    VertexDescriptor addVertex(VertexData&& data)
    { return _multiGraph.addVertex(std::move(data)); }

    void removeVertex(VertexDescriptor v) { _multiGraph.removeVertex(v); }

    template<typename... Args>
    std::optional<EdgeDescriptor> emplaceEdge(VertexDescriptor from,
                                              VertexDescriptor to,
                                              Args&&... args)
    {
        if (from == to) return std::nullopt;
        if (_multiGraph.hasEdge(from, to)) return std::nullopt;
        return _multiGraph.emplaceEdge(from, to, std::forward<Args>(args)...);
    }

    template<typename T = EdgeData>
        requires (!std::is_void_v<EdgeData>)
    std::optional<EdgeDescriptor> addEdge(VertexDescriptor from,
                                          VertexDescriptor to,
                                          T&& data)
    { return emplaceEdge(from, to, std::forward<T>(data)); }

    std::optional<EdgeDescriptor> addEdge(VertexDescriptor from, VertexDescriptor to)
        requires (std::is_void_v<EdgeData>)
    { return emplaceEdge(from, to); }

    void removeEdge(EdgeDescriptor e) { _multiGraph.removeEdge(e); }

    std::optional<EdgeDescriptor> findEdge(VertexDescriptor from, VertexDescriptor to)
    { return _multiGraph.findEdge(from, to); }
    std::optional<ConstEdgeDescriptor> findEdge(ConstVertexDescriptor from, ConstVertexDescriptor to) const
    { return _multiGraph.findEdge(from, to); }

    bool hasEdge(ConstVertexDescriptor from, ConstVertexDescriptor to) const
    { return _multiGraph.hasEdge(from, to); }

    VertexIterator beginVertices()             { return _multiGraph.beginVertices(); }
    VertexIterator endVertices()               { return _multiGraph.endVertices(); }
    ConstVertexIterator beginVertices()  const { return _multiGraph.beginVertices(); }
    ConstVertexIterator endVertices()    const { return _multiGraph.endVertices(); }
    ConstVertexIterator cbeginVertices() const { return _multiGraph.cbeginVertices(); }
    ConstVertexIterator cendVertices()   const { return _multiGraph.cendVertices(); }

    auto vertices()            { return _multiGraph.vertices(); }
    auto vertices()      const { return _multiGraph.vertices(); }
    auto constVertices() const { return _multiGraph.constVertices(); }

    EdgeIterator beginEdges()             { return _multiGraph.beginEdges(); }
    EdgeIterator endEdges()               { return _multiGraph.endEdges(); }
    ConstEdgeIterator beginEdges()  const { return _multiGraph.beginEdges(); }
    ConstEdgeIterator endEdges()    const { return _multiGraph.endEdges(); }
    ConstEdgeIterator cbeginEdges() const { return _multiGraph.cbeginEdges(); }
    ConstEdgeIterator cendEdges()   const { return _multiGraph.cendEdges(); }

    auto edges()            { return _multiGraph.edges(); }
    auto edges()      const { return _multiGraph.edges(); }
    auto constEdges() const { return _multiGraph.constEdges(); }

    std::size_t vertexCount() const { return _multiGraph.vertexCount(); }
    std::size_t edgeCount()   const { return _multiGraph.edgeCount(); }

    const UndirectedMultiGraph<VertexData, EdgeData>& baseMultiGraph() const
    { return _multiGraph; }

    void clear() { _multiGraph.clear(); }

    bool empty() const { return _multiGraph.empty(); }

    template <typename T = EdgeData>
        requires (!std::is_void_v<EdgeData>)
    Matrix<T> toAdjacencyMatrix(T default_value = T{}) const {
        const std::size_t n = vertexCount();
        Matrix<T> mat(n, n);

        for (std::size_t i = 0; i < n; ++i)
            for (std::size_t j = 0; j < n; ++j)
                mat(i, j) = default_value;

        for (auto v : vertices()) {
            for (auto e : v.incidentEdges()) {
                auto u = *e.otherEnd(v);
                mat(v.data(), u.data()) = e.data();
                mat(u.data(), v.data()) = e.data();
            }
        }
        return mat;
    }

    Matrix<bool> toAdjacencyMatrix() const
        requires std::is_void_v<EdgeData>
    {
        const std::size_t n = vertexCount();
        Matrix<bool> mat(n, n);

        for (std::size_t i = 0; i < n; ++i)
            for (std::size_t j = 0; j < n; ++j)
                mat(i, j) = false;

        for (auto v : vertices()) {
            for (auto e : v.incidentEdges()) {
                auto u = *e.otherEnd(v);
                mat(v.data(), u.data()) = true;
                mat(u.data(), v.data()) = true;
            }
        }
        return mat;
    }

    template <typename T = EdgeData>
        requires (!std::is_void_v<EdgeData>)
    static auto fromAdjacencyMatrix(const T* matrix, std::size_t n)
        ->std::expected<UndirectedGraph<VertexData, T>, GraphBuildingError>
    {
        if (!matrix) return std::unexpected(GraphBuildingError::NullPointer);
        if (n == 0) return std::unexpected(GraphBuildingError::ZeroSize);

        UndirectedGraph<VertexData, EdgeData> g;
        std::unordered_map<std::size_t,
                           typename UndirectedGraph<VertexData, EdgeData>::VertexDescriptor> ht;

        for (std::size_t i = 0; i < n; ++i) {
            auto desc = g.addVertex(static_cast<VertexData>(i));
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

    template <typename T = EdgeData>
        requires (!std::is_void_v<EdgeData>)
    static auto fromAdjacencyMatrix(const std::vector<std::vector<T>>& matrix)
        ->std::expected<UndirectedGraph<VertexData, T>, GraphBuildingError>
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
        return fromAdjacencyMatrix<EdgeData>(flat.data(), n);
    }

    static auto fromAdjacencyMatrix(const std::vector<bool>& matrix, std::size_t n)
        ->std::expected<UndirectedGraph<VertexData, void>, GraphBuildingError>
        requires std::is_void_v<EdgeData>
    {
        if (matrix.empty()) return std::unexpected(GraphBuildingError::EmptyVector);
        if (n == 0) return std::unexpected(GraphBuildingError::ZeroSize);

        UndirectedGraph<VertexData, void> g;
        std::unordered_map<VertexData,
                           typename UndirectedGraph<VertexData, void>::VertexDescriptor> ht;

        for (std::size_t i = 0; i < n; ++i) {
            auto desc = g.addVertex(static_cast<VertexData>(i));
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

    static auto fromAdjacencyMatrix(const std::vector<std::vector<bool>>& matrix)
        ->std::expected<UndirectedGraph<VertexData, void>, GraphBuildingError>
        requires std::is_void_v<EdgeData>
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
        return fromAdjacencyMatrix(flat, n);
    }

private:

    UndirectedMultiGraph<VertexData, EdgeData> _multiGraph;

};

} // namespace exx::incident

#endif // EXX_UNDIRECTEDGRAPH_HPP
