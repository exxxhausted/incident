#include <catch2/catch_test_macros.hpp>

#include <unordered_set>

#include "incident/algorithms/dfs.hpp"
#include "incident/undirected/undirected.hpp" // IWYU pragma: keep
#include "incident/directed/directed.hpp"     // IWYU pragma: keep
#include "incident/utility/UniqueVertexIndexedView.hpp"

using namespace exx::incident;

// ------------------ Неориентированные тесты (проверка достижимости) ------------------
TEST_CASE("DFS on undirected graph", "[dfs][undirected]") {
    using Graph = UndirectedGraph<int, void>;

    SECTION("Simple connected graph") {
        std::vector<bool> matrix = {
            0,1,1,0,
            1,0,1,1,
            1,1,0,0,
            0,1,0,0
        };
        auto g = Graph::fromAdjacencyMatrix(matrix, 4);
        REQUIRE(g.has_value());
        auto start = *g->beginVertices(); // vertex 0
        auto result = dfs(*g, start);
        std::unordered_set<int> expected = {0,1,2,3};
        std::unordered_set<int> got;
        for (auto v : result) got.insert(v.data());
        REQUIRE(got == expected);
    }

    SECTION("Disconnected graph") {
        std::vector<bool> matrix = {
            0,1,0,0,
            1,0,0,0,
            0,0,0,1,
            0,0,1,0
        };
        auto g = Graph::fromAdjacencyMatrix(matrix, 4);
        REQUIRE(g.has_value());
        auto start = *g->beginVertices(); // 0
        auto result = dfs(*g, start);
        std::unordered_set<int> expected = {0,1};
        std::unordered_set<int> got;
        for (auto v : result) got.insert(v.data());
        REQUIRE(got == expected);
    }

    SECTION("Handles multiedges (ignores duplicates)") {
        std::vector<bool> matrix = {
            0,1,0,
            1,0,1,
            0,1,0
        };
        auto simple = Graph::fromAdjacencyMatrix(matrix, 3);
        REQUIRE(simple.has_value());
        UndirectedMultiGraph<int, void> g(simple->baseMultiGraph());
        auto gv = UniqueVertexIndexedView(g);
        auto v0 = *gv.findVertex(0);
        auto v1 = *gv.findVertex(1);
        g.addEdge(v0, v1);
        g.addEdge(v0, v1); // parallel edges
        auto result = dfs(g, v0);
        std::unordered_set<int> expected = {0,1,2};
        std::unordered_set<int> got;
        for (auto v : result) got.insert(v.data());
        REQUIRE(got == expected);
    }

    SECTION("Handles self-loops") {
        std::vector<bool> matrix = {
            0,1,0,
            1,0,1,
            0,1,0
        };
        auto simple = Graph::fromAdjacencyMatrix(matrix, 3);
        REQUIRE(simple.has_value());
        UndirectedPseudoGraph<int, void> g(simple->baseMultiGraph().basePseudoGraph());
        auto gv = UniqueVertexIndexedView(g);
        auto v1 = *gv.findVertex(1);
        g.addEdge(v1, v1);
        auto v0 = *gv.findVertex(0);
        auto result = dfs(g, v0);
        std::unordered_set<int> expected = {0,1,2};
        std::unordered_set<int> got;
        for (auto v : result) got.insert(v.data());
        REQUIRE(got == expected);
    }
}

TEST_CASE("DFS on directed graph", "[dfs][directed]") {
    using Graph = DirectedGraph<int, void>;

    SECTION("Simple DAG") {
        std::vector<std::vector<bool>> mat = {
            {0,1,1,0},
            {0,0,1,1},
            {0,0,0,0},
            {0,0,0,0}
        };
        auto g = Graph::fromAdjacencyMatrix(mat);
        REQUIRE(g.has_value());
        auto start = *g->beginVertices(); // 0
        auto result = dfs(*g, start);
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
        auto g = Graph::fromAdjacencyMatrix(mat);
        REQUIRE(g.has_value());
        auto start = *g->beginVertices(); // 0
        auto result = dfs(*g, start);
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
        auto g = Graph::fromAdjacencyMatrix(mat);
        REQUIRE(g.has_value());
        auto start = *g->beginVertices(); // 0
        auto result = dfs(*g, start);
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
        pseudo.addArc(v0, v1);
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
