#include <catch2/catch_test_macros.hpp>

#include <set>
#include <vector>

#include "incident/algorithms/bfs.hpp"
#include "incident/undirected/UndirectedGraph.hpp"
#include "incident/undirected/UndirectedMultiGraph.hpp"
#include "incident/undirected/UndirectedPseudoGraph.hpp"
#include "incident/directed/DirectedGraph.hpp"
#include "incident/directed/DirectedMultiGraph.hpp"
#include "incident/directed/DirectedPseudoGraph.hpp"
#include "incident/utility/UniqueVertexIndexedView.hpp"

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

TEST_CASE("BFS on undirected graph", "[bfs][undirected]") {
    SECTION("BFS on simple connected graph") {
        /*
            0 -- 1 -- 3
            |    |
            2 __/
        */
        std::vector<bool> mat = {
            0,1,1,0,
            1,0,1,1,
            1,1,0,0,
            0,1,0,0
        };
        auto g = makeUndirectedGraphFromBoolMatrix(mat, 4);
        auto start = *g.beginVertices(); // 0
        auto result = bfs(g, start);

        REQUIRE(result.size() == 4);
        REQUIRE(result[0].data() == 0);
        std::set<int> level1 = {result[1].data(), result[2].data()};
        REQUIRE(level1 == std::set<int>{1,2});
        REQUIRE(result[3].data() == 3);
    }

    SECTION("BFS handles disconnected graph") {
        /*
        0 -- 1    2 -- 3
        */
        std::vector<bool> mat = {
            0,1,0,0,
            1,0,0,0,
            0,0,0,1,
            0,0,1,0
        };
        auto g = makeUndirectedGraphFromBoolMatrix(mat, 4);
        auto start = *g.beginVertices(); // 0
        auto result = bfs(g, start);
        REQUIRE(result.size() == 2);
        REQUIRE(result[0].data() == 0);
        REQUIRE(result[1].data() == 1);
    }

    SECTION("BFS ignores multiedge revisits") {
        /*
        Base: 0-1-2, add extra parallel edges 0-1
        */
        std::vector<bool> mat = {0,1,0, 1,0,1, 0,1,0};
        auto simple = makeUndirectedGraphFromBoolMatrix(mat, 3);
        UndirectedMultiGraph<int, void> multi(simple.baseMultiGraph());
        auto view = UniqueVertexIndexedView(multi);
        auto v0 = *view.findVertex(0);
        auto v1 = *view.findVertex(1);
        multi.addEdge(v0, v1);
        multi.addEdge(v0, v1);
        auto result = bfs(multi, v0);
        REQUIRE(result.size() == 3);
        REQUIRE(result[0].data() == 0);
        REQUIRE(result[1].data() == 1);
        REQUIRE(result[2].data() == 2);
    }

    SECTION("BFS handles self-loops") {
        /*
        Base: 0-1-2, add self-loop on 1
        */
        std::vector<bool> mat = {0,1,0, 1,0,1, 0,1,0};
        auto simple = makeUndirectedGraphFromBoolMatrix(mat, 3);
        UndirectedPseudoGraph<int, void> pseudo(simple.baseMultiGraph().basePseudoGraph());
        auto view = UniqueVertexIndexedView(pseudo);
        auto v1 = *view.findVertex(1);
        pseudo.addEdge(v1, v1);
        auto v0 = *view.findVertex(0);
        auto result = bfs(pseudo, v0);
        REQUIRE(result.size() == 3);
        REQUIRE(result[0].data() == 0);
        REQUIRE(result[1].data() == 1);
        REQUIRE(result[2].data() == 2);
    }
}

TEST_CASE("BFS on directed graph", "[bfs][directed]") {
    SECTION("Simple DAG") {
        /*
        0 -> 1 -> 3
        |    |
        v    v
        2 <--
        */
        std::vector<std::vector<bool>> mat = {
            {0,1,1,0},
            {0,0,1,1},
            {0,0,0,0},
            {0,0,0,0}
        };
        auto g = makeDirectedGraphFromBoolMatrix(mat);
        auto start = *g.beginVertices(); // 0
        auto result = bfs(g, start);
        REQUIRE(result.size() == 4);
        REQUIRE(result[0].data() == 0);
        std::set<int> secondLayer = {result[1].data(), result[2].data()};
        REQUIRE(secondLayer == std::set<int>{1,2});
        REQUIRE(result[3].data() == 3);
    }

    SECTION("Disconnected directed graph") {
        /*
        Comp1: 0->1, Comp2: 2->3
        */
        std::vector<std::vector<bool>> mat = {
            {0,1,0,0},
            {0,0,0,0},
            {0,0,0,1},
            {0,0,0,0}
        };
        auto g = makeDirectedGraphFromBoolMatrix(mat);
        auto start = *g.beginVertices(); // 0
        auto result = bfs(g, start);
        REQUIRE(result.size() == 2);
        REQUIRE(result[0].data() == 0);
        REQUIRE(result[1].data() == 1);
    }

    SECTION("Graph with cycle") {
        /*
        0 -> 1 -> 2
        ^         |
        |_________|
        */
        std::vector<std::vector<bool>> mat = {
            {0,1,0},
            {0,0,1},
            {1,0,0}
        };
        auto g = makeDirectedGraphFromBoolMatrix(mat);
        auto start = *g.beginVertices(); // 0
        auto result = bfs(g, start);
        REQUIRE(result.size() == 3);
        REQUIRE(result[0].data() == 0);
        std::set<int> rest = {result[1].data(), result[2].data()};
        REQUIRE(rest == std::set<int>{1,2});
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
        auto result = bfs(g, v0);
        REQUIRE(result.size() == 3);
        REQUIRE(result[0].data() == 0);
        REQUIRE(result[1].data() == 1);
        REQUIRE(result[2].data() == 2);
    }

    SECTION("Self-loop does not cause infinite loop") {
        DirectedPseudoGraph<int, void> pseudo;
        auto v0 = pseudo.addVertex(0);
        auto v1 = pseudo.addVertex(1);
        pseudo.addArc(v0, v0);
        pseudo.addArc(v0, v1);
        DirectedGraph<int, void> g(pseudo);
        auto result = bfs(g, v0);
        REQUIRE(result.size() == 2);
        REQUIRE(result[0].data() == 0);
        REQUIRE(result[1].data() == 1);
    }
}
