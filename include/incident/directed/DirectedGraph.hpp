#ifndef EXX_DIRECTEDGRAPH_HPP
#define EXX_DIRECTEDGRAPH_HPP

#include <optional>
#include <expected>

#include "DirectedMultiGraph.hpp"
#include "../details/Matrix.hpp"
#include "../details/errors.hpp"

namespace exx::incident {

template<typename VertexData, typename ArcData>
class DirectedGraph {
public:

    using VertexValueType = VertexData;
    using ArcValueType    = ArcData;

    using VertexDescriptor      = typename DirectedMultiGraph<VertexData, ArcData>::VertexDescriptor;
    using ConstVertexDescriptor = typename DirectedMultiGraph<VertexData, ArcData>::ConstVertexDescriptor;
    using VertexDescriptorHash  = typename DirectedMultiGraph<VertexData, ArcData>::VertexDescriptorHash;

    using ArcDescriptor         = typename DirectedMultiGraph<VertexData, ArcData>::ArcDescriptor;
    using ConstArcDescriptor    = typename DirectedMultiGraph<VertexData, ArcData>::ConstArcDescriptor;
    using ArcDescriptorHash     = typename DirectedMultiGraph<VertexData, ArcData>::ArcDescriptorHash;

    using VertexIterator        = typename DirectedMultiGraph<VertexData, ArcData>::VertexIterator;
    using ConstVertexIterator   = typename DirectedMultiGraph<VertexData, ArcData>::ConstVertexIterator;
    using ArcIterator           = typename DirectedMultiGraph<VertexData, ArcData>::ArcIterator;
    using ConstArcIterator      = typename DirectedMultiGraph<VertexData, ArcData>::ConstArcIterator;

    DirectedGraph() = default;

    explicit DirectedGraph(const DirectedPseudoGraph<VertexData, ArcData>& graph)
        : _multiGraph(graph) {}

    DirectedGraph(const DirectedGraph&) = default;
    DirectedGraph(DirectedGraph&&) noexcept = default;

    DirectedGraph& operator=(const DirectedGraph&) = default;
    DirectedGraph& operator=(DirectedGraph&&) noexcept = default;

    void swap(DirectedGraph& other) noexcept { _multiGraph.swap(other._multiGraph); }

    template<typename... Args>
    VertexDescriptor emplaceVertex(Args&&... args)
    { return _multiGraph.emplaceVertex(std::forward<Args>(args)...); }

    VertexDescriptor addVertex(const VertexData& data)
    { return _multiGraph.addVertex(data); }

    VertexDescriptor addVertex(VertexData&& data)
    { return _multiGraph.addVertex(std::move(data)); }

    void removeVertex(VertexDescriptor v) { _multiGraph.removeVertex(v); }

    template<typename... Args>
    std::optional<ArcDescriptor> emplaceArc(VertexDescriptor from,
                                            VertexDescriptor to,
                                            Args&&... args)
    {
        if (from == to) return std::nullopt;
        if (_multiGraph.hasArc(from, to)) return std::nullopt;
        return _multiGraph.emplaceArc(from, to, std::forward<Args>(args)...);
    }

    template<typename T = ArcData>
        requires (!std::is_void_v<ArcData>)
    std::optional<ArcDescriptor> addArc(VertexDescriptor from,
                                        VertexDescriptor to,
                                        T&& data)
    { return emplaceArc(from, to, std::forward<T>(data)); }

    std::optional<ArcDescriptor> addArc(VertexDescriptor from, VertexDescriptor to)
        requires (std::is_void_v<ArcData>)
    { return emplaceArc(from, to); }

    void removeArc(ArcDescriptor a) { _multiGraph.removeArc(a); }

    std::optional<ArcDescriptor> findArc(VertexDescriptor from, VertexDescriptor to)
    { return _multiGraph.findArc(from, to); }
    std::optional<ConstArcDescriptor> findArc(ConstVertexDescriptor from, ConstVertexDescriptor to) const
    { return _multiGraph.findArc(from, to); }

    bool hasArc(ConstVertexDescriptor from, ConstVertexDescriptor to) const
    { return _multiGraph.hasArc(from, to); }

    VertexIterator beginVertices()             { return _multiGraph.beginVertices(); }
    VertexIterator endVertices()               { return _multiGraph.endVertices(); }
    ConstVertexIterator beginVertices()  const { return _multiGraph.beginVertices(); }
    ConstVertexIterator endVertices()    const { return _multiGraph.endVertices(); }
    ConstVertexIterator cbeginVertices() const { return _multiGraph.cbeginVertices(); }
    ConstVertexIterator cendVertices()   const { return _multiGraph.cendVertices(); }

    auto vertices()            { return _multiGraph.vertices(); }
    auto vertices()      const { return _multiGraph.vertices(); }
    auto constVertices() const { return _multiGraph.constVertices(); }

    ArcIterator beginArcs()             { return _multiGraph.beginArcs(); }
    ArcIterator endArcs()               { return _multiGraph.endArcs(); }
    ConstArcIterator beginArcs()  const { return _multiGraph.beginArcs(); }
    ConstArcIterator endArcs()    const { return _multiGraph.endArcs(); }
    ConstArcIterator cbeginArcs() const { return _multiGraph.cbeginArcs(); }
    ConstArcIterator cendArcs()   const { return _multiGraph.cendArcs(); }

    auto arcs()            { return _multiGraph.arcs(); }
    auto arcs()      const { return _multiGraph.arcs(); }
    auto constArcs() const { return _multiGraph.constArcs(); }

    std::size_t vertexCount() const { return _multiGraph.vertexCount(); }
    std::size_t arcCount()   const { return _multiGraph.arcCount(); }

    const DirectedMultiGraph<VertexData, ArcData>& baseMultiGraph() const
    { return _multiGraph; }

    void clear() { _multiGraph.clear(); }

    bool empty() const { return _multiGraph.empty(); }

    bool roteteArc(ArcDescriptor a) { return _multiGraph.rotateArc(a); }

    template <typename T = ArcData>
        requires (!std::is_void_v<ArcData>)
    Matrix<T> toAdjacencyMatrix(T default_value = T{}) const {
        const std::size_t n = vertexCount();
        Matrix<T> mat(n, n);

        for (std::size_t i = 0; i < n; ++i)
            for (std::size_t j = 0; j < n; ++j)
                mat(i, j) = default_value;

        for (auto v : vertices()) {
            for (auto a : v.outgoingArcs()) {
                auto u = a.to();
                mat(v.data(), u.data()) = a.data();
            }
        }
        return mat;
    }

    Matrix<bool> toAdjacencyMatrix() const
        requires std::is_void_v<ArcData>
    {
        const std::size_t n = vertexCount();
        Matrix<bool> mat(n, n);

        for (std::size_t i = 0; i < n; ++i)
            for (std::size_t j = 0; j < n; ++j)
                mat(i, j) = false;

        for (auto v : vertices()) {
            for (auto a : v.outgoingArcs()) {
                auto u = a.to();
                mat(v.data(), u.data()) = true;
            }
        }
        return mat;
    }

    template <typename T = ArcData>
        requires (!std::is_void_v<ArcData>)
    static auto fromAdjacencyMatrix(const T* matrix, std::size_t n)
        ->std::expected<DirectedGraph<VertexData, T>, GraphBuildingError>
    {
        if (!matrix) return std::unexpected(GraphBuildingError::NullPointer);
        if (n == 0) return std::unexpected(GraphBuildingError::ZeroSize);

        DirectedGraph<VertexData, ArcData> g;
        std::unordered_map<std::size_t,
                           typename DirectedGraph<VertexData, ArcData>::VertexDescriptor> ht;

        for (std::size_t i = 0; i < n; ++i) {
            auto desc = g.addVertex(static_cast<VertexData>(i));
            ht.emplace(i, desc);
        }

        for (std::size_t i = 0; i < n; ++i) {
            for (std::size_t j = 0; j < n; ++j) {
                auto val = matrix[i * n + j];
                if (val != T{})
                    g.addArc(ht[i], ht[j], val);
            }
        }

        return g;
    }

    template <typename T = ArcData>
        requires (!std::is_void_v<ArcData>)
    static auto fromAdjacencyMatrix(const std::vector<std::vector<T>>& matrix)
        ->std::expected<DirectedGraph<VertexData, T>, GraphBuildingError>
    {
        if (matrix.empty())
            return std::unexpected(GraphBuildingError::ZeroSize);

        std::size_t n = matrix.size();
        std::vector<T> flat;
        flat.reserve(n * n);
        for (const auto& row : matrix) {
            if (row.size() != n) return std::unexpected(GraphBuildingError::NonSquareMatrix);
            flat.insert(flat.end(), row.begin(), row.end());
        }
        return fromAdjacencyMatrix<T>(flat.data(), n);
    }

    static auto fromAdjacencyMatrix(const std::vector<bool>& matrix, std::size_t n)
        ->std::expected<DirectedGraph<VertexData, void>, GraphBuildingError>
        requires std::is_void_v<ArcData>
    {
        if (matrix.empty()) return std::unexpected(GraphBuildingError::EmptyVector);
        if (n == 0) return std::unexpected(GraphBuildingError::ZeroSize);

        DirectedGraph<VertexData, void> g;
        std::unordered_map<std::size_t,
                           typename DirectedGraph<VertexData, void>::VertexDescriptor> ht;

        for (std::size_t i = 0; i < n; ++i) {
            auto desc = g.addVertex(static_cast<VertexData>(i));
            ht.emplace(i, desc);
        }

        for (std::size_t i = 0; i < n; ++i) {
            for (std::size_t j = 0; j < n; ++j) {
                auto val = matrix[i * n + j];
                if (val) g.addArc(ht[i], ht[j]);
            }
        }

        return g;
    }

    static auto fromAdjacencyMatrix(const std::vector<std::vector<bool>>& matrix)
        ->std::expected<DirectedGraph<VertexData, void>, GraphBuildingError>
        requires std::is_void_v<ArcData>
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

    DirectedMultiGraph<VertexData, ArcData> _multiGraph;

};

} // namespace exx::incident

#endif // EXX_DIRECTEDGRAPH_HPP
