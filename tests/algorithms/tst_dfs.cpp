#include <catch2/catch_test_macros.hpp>

#include <unordered_set>
#include <vector>
#include <unordered_map>

#include "incident/algorithms/dfs.hpp"
#include "incident/undirected/UndirectedGraph.hpp"
#include "incident/undirected/UndirectedMultiGraph.hpp"
#include "incident/undirected/UndirectedPseudoGraph.hpp"
#include "incident/directed/DirectedGraph.hpp"
#include "incident/directed/DirectedPseudoGraph.hpp"

using namespace exx::incident;

static UndirectedGraph<int, void> makeUndirectedGraphFromBoolMatrix(const std::vector<bool>& matrix,
                                                                    std::size_t n)
{
    UndirectedGraph<int, void> g;
    std::vector<UndirectedGraph<int, void>::VertexDescriptor> verts(n);
    for (std::size_t i = 0; i < n; ++i)
        verts[i] = g.addVertex(static_cast<int>(i));

    for (std::size_t i = 0; i < n; ++i)
        for (std::size_t j = i + 1; j < n; ++j)
            if (matrix[i * n + j])
                g.addEdge(verts[i], verts[j]);
    return g;
}

static DirectedGraph<int, void> makeDirectedGraphFromBoolMatrix(const std::vector<std::vector<bool>>& matrix)
{
    std::size_t n = matrix.size();
    DirectedGraph<int, void> g;
    std::vector<DirectedGraph<int, void>::VertexDescriptor> verts(n);
    for (std::size_t i = 0; i < n; ++i)
        verts[i] = g.addVertex(static_cast<int>(i));

    for (std::size_t i = 0; i < n; ++i)
        for (std::size_t j = 0; j < n; ++j)
            if (matrix[i][j])
                g.addArc(verts[i], verts[j]);
    return g;
}

TEST_CASE("DFS on undirected graph", "[dfs][undirected]") {
    SECTION("Simple connected graph") {
        std::vector<bool> mat = {
            0,1,1,0,
            1,0,1,1,
            1,1,0,0,
            0,1,0,0
        };
        auto g = makeUndirectedGraphFromBoolMatrix(mat, 4);
        auto start = *g.beginVertices(); // vertex 0
        auto result = dfs(g, start);
        std::unordered_set<int> expected = {0,1,2,3};
        std::unordered_set<int> got;
        for (auto v : result) got.insert(v.data());
        REQUIRE(got == expected);
    }

    SECTION("Disconnected graph") {
        std::vector<bool> mat = {
            0,1,0,0,
            1,0,0,0,
            0,0,0,1,
            0,0,1,0
        };
        auto g = makeUndirectedGraphFromBoolMatrix(mat, 4);
        auto start = *g.beginVertices(); // 0
        auto result = dfs(g, start);
        std::unordered_set<int> expected = {0,1};
        std::unordered_set<int> got;
        for (auto v : result) got.insert(v.data());
        REQUIRE(got == expected);
    }

    SECTION("Handles multiedges (ignores duplicates)") {
        // start with simple graph 0-1, 1-2
        std::vector<bool> mat = {
            0,1,0,
            1,0,1,
            0,1,0
        };
        auto simple = makeUndirectedGraphFromBoolMatrix(mat, 3);
        UndirectedMultiGraph<int, void> multi(simple.baseMultiGraph());

        // Build local index map
        std::unordered_map<int, UndirectedMultiGraph<int, void>::VertexDescriptor> desc;
        for (auto v : multi.vertices())
            desc[v.data()] = v;

        auto v0 = desc[0];
        auto v1 = desc[1];
        multi.addEdge(v0, v1); // parallel edge
        multi.addEdge(v0, v1);
        auto result = dfs(multi, v0);
        std::unordered_set<int> expected = {0,1,2};
        std::unordered_set<int> got;
        for (auto v : result) got.insert(v.data());
        REQUIRE(got == expected);
    }

    SECTION("Handles self-loops") {
        std::vector<bool> mat = {
            0,1,0,
            1,0,1,
            0,1,0
        };
        auto simple = makeUndirectedGraphFromBoolMatrix(mat, 3);
        UndirectedPseudoGraph<int, void> pseudo(simple.baseMultiGraph().basePseudoGraph());

        std::unordered_map<int, UndirectedPseudoGraph<int, void>::VertexDescriptor> desc;
        for (auto v : pseudo.vertices())
            desc[v.data()] = v;

        auto v1 = desc[1];
        pseudo.addEdge(v1, v1); // self-loop
        auto v0 = desc[0];
        auto result = dfs(pseudo, v0);
        std::unordered_set<int> expected = {0,1,2};
        std::unordered_set<int> got;
        for (auto v : result) got.insert(v.data());
        REQUIRE(got == expected);
    }
}

TEST_CASE("DFS on directed graph", "[dfs][directed]") {
    SECTION("Simple DAG") {
        std::vector<std::vector<bool>> mat = {
            {0,1,1,0},
            {0,0,1,1},
            {0,0,0,0},
            {0,0,0,0}
        };
        auto g = makeDirectedGraphFromBoolMatrix(mat);
        auto start = *g.beginVertices(); // 0
        auto result = dfs(g, start);
        std::unordered_set<int> expected = {0,1,2,3};
        std::unordered_set<int> got;
        for (auto v : result) got.insert(v.data());
        REQUIRE(got == expected);
    }

    SECTION("Disconnected directed graph") {
        std::vector<std::vector<bool>> mat = {
            {0,1,0,0},
            {0,0,0,0},
            {0,0,0,1},
            {0,0,0,0}
        };
        auto g = makeDirectedGraphFromBoolMatrix(mat);
        auto start = *g.beginVertices(); // 0
        auto result = dfs(g, start);
        std::unordered_set<int> expected = {0,1};
        std::unordered_set<int> got;
        for (auto v : result) got.insert(v.data());
        REQUIRE(got == expected);
    }

    SECTION("Graph with cycle") {
        std::vector<std::vector<bool>> mat = {
            {0,1,0},
            {0,0,1},
            {1,0,0}
        };
        auto g = makeDirectedGraphFromBoolMatrix(mat);
        auto start = *g.beginVertices(); // 0
        auto result = dfs(g, start);
        std::unordered_set<int> expected = {0,1,2};
        std::unordered_set<int> got;
        for (auto v : result) got.insert(v.data());
        REQUIRE(got == expected);
    }

    SECTION("Multiple arcs (parallel) ignored") {
        DirectedPseudoGraph<int, void> pseudo;
        auto v0 = pseudo.addVertex(0);
        auto v1 = pseudo.addVertex(1);
        auto v2 = pseudo.addVertex(2);
        pseudo.addArc(v0, v1);
        pseudo.addArc(v0, v1); // parallel
        pseudo.addArc(v1, v2);
        DirectedGraph<int, void> g(pseudo);
        auto result = dfs(g, v0);
        std::unordered_set<int> expected = {0,1,2};
        std::unordered_set<int> got;
        for (auto v : result) got.insert(v.data());
        REQUIRE(got == expected);
    }

    SECTION("Self-loop does not cause infinite recursion") {
        DirectedPseudoGraph<int, void> pseudo;
        auto v0 = pseudo.addVertex(0);
        auto v1 = pseudo.addVertex(1);
        pseudo.addArc(v0, v0);
        pseudo.addArc(v0, v1);
        DirectedGraph<int, void> g(pseudo);
        auto result = dfs(g, v0);
        std::unordered_set<int> expected = {0,1};
        std::unordered_set<int> got;
        for (auto v : result) got.insert(v.data());
        REQUIRE(got == expected);
    }
}
