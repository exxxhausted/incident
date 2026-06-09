#include <catch2/catch_test_macros.hpp>

#include <set>

#include "incident/algorithms/bfs.hpp"
#include "incident/undirected/undirected.hpp" // IWYU pragma: keep
#include "incident/directed/directed.hpp"     // IWYU pragma: keep
#include "incident/utility/UniqueVertexIndexedView.hpp"

using namespace exx::incident;

TEST_CASE("BFS on undirected graph", "[bfs][undirected]") {
    SECTION("BFS on simple connected graph") {
        /*
        0 -- 1 -- 3
        |    |
        2 __/

        Expected BFS from 0:
        0 1 2 3
        */

        std::vector<bool> matrix =
            {
                0,1,1,0,
                1,0,1,1,
                1,1,0,0,
                0,1,0,0
            };

        auto g = UndirectedGraph<int, void>::
            fromAdjacencyMatrix(matrix, 4);

        auto start = *g->beginVertices();

        auto result = bfs(*g, start);

        REQUIRE(result.size() == 4);

        REQUIRE(result[0].data() == 0);
        REQUIRE(result[1].data() == 1);
        REQUIRE(result[2].data() == 2);
        REQUIRE(result[3].data() == 3);
    }

    SECTION("BFS handles disconnected graph") {
        /*
        0 -- 1

        2 -- 3

        BFS from 0 should visit only:
        0 1
        */

        std::vector<bool> matrix =
            {
                0,1,0,0,
                1,0,0,0,
                0,0,0,1,
                0,0,1,0
            };

        auto g = UndirectedGraph<int, void>
            ::fromAdjacencyMatrix(matrix, 4);

        auto start = *g->beginVertices();

        auto result = bfs(*g, start);

        REQUIRE(result.size() == 2);

        REQUIRE(result[0].data() == 0);
        REQUIRE(result[1].data() == 1);
    }

    SECTION("BFS ignores multiedge revisits") {
        /*
        Base graph:

        0 -- 1 -- 2

        Add extra edges:
            0 -- 1
            0 -- 1

        Expected BFS:
        0 1 2
        */

        std::vector<bool> matrix =
            {
                0,1,0,
                1,0,1,
                0,1,0
            };

        auto simple = UndirectedGraph<int, void>::
            fromAdjacencyMatrix(matrix, 3);

        UndirectedMultiGraph<int, void> g(simple->baseMultiGraph());
        auto gv = UniqueVertexIndexedView(g);

        auto v0 = *gv.findVertex(0);
        auto v1 = *gv.findVertex(1);

        g.addEdge(v0, v1);
        g.addEdge(v0, v1);

        auto result = bfs(g, v0);

        REQUIRE(result.size() == 3);

        REQUIRE(result[0].data() == 0);
        REQUIRE(result[1].data() == 1);
        REQUIRE(result[2].data() == 2);
    }

    SECTION("BFS handles self-loops") {
        /*
        Base graph:
             __
            \ /
        0 -- 1 -- 2

        Expected BFS:
        0 1 2
        */

        std::vector<bool> matrix =
            {
                0,1,0,
                1,0,1,
                0,1,0
            };

        auto simple = UndirectedGraph<int, void>::
            fromAdjacencyMatrix(matrix, 3);

        UndirectedPseudoGraph<int, void> g(simple->baseMultiGraph()
                                               .basePseudoGraph());
        auto gv = UniqueVertexIndexedView(g);

        auto v1 = *gv.findVertex(1);

        g.addEdge(v1, v1);

        auto v0 = *gv.findVertex(0);

        auto result = bfs(g, v0);

        REQUIRE(result.size() == 3);

        REQUIRE(result[0].data() == 0);
        REQUIRE(result[1].data() == 1);
        REQUIRE(result[2].data() == 2);
    }
}

TEST_CASE("BFS on directed graph", "[bfs][directed]") {
    SECTION("Simple DAG") {
        /*
        Graph:
        0 -> 1 -> 3
        |    |
        v    v
        2 <--

        Expected BFS from 0: 0 1 2 3 (level order)
        */
        std::vector<std::vector<int>> mat = {
            {0, 1, 1, 0},
            {0, 0, 1, 1},
            {0, 0, 0, 0},
            {0, 0, 0, 0}
        };
        auto g = DirectedGraph<int, int>::fromAdjacencyMatrix(mat);
        REQUIRE(g.has_value());
        auto start = *g->beginVertices(); // vertex 0
        auto result = bfs(*g, start);
        REQUIRE(result.size() == 4);
        REQUIRE(result[0].data() == 0);
        std::set<int> secondLayer = {result[1].data(), result[2].data()};
        REQUIRE(secondLayer == std::set<int>{1, 2});
        REQUIRE(result[3].data() == 3);
    }

    SECTION("Disconnected directed graph") {
        /*
        Component 1: 0 -> 1
        Component 2: 2 -> 3

        BFS from 0 visits only {0,1}
        */
        std::vector<std::vector<int>> mat = {
            {0,1,0,0},
            {0,0,0,0},
            {0,0,0,1},
            {0,0,0,0}
        };
        auto g = DirectedGraph<int,int>::fromAdjacencyMatrix(mat);
        REQUIRE(g.has_value());
        auto start = *g->beginVertices(); // 0
        auto result = bfs(*g, start);
        REQUIRE(result.size() == 2);
        REQUIRE(result[0].data() == 0);
        REQUIRE(result[1].data() == 1);
    }

    SECTION("Graph with cycles (directed)") {
        /*
        0 -> 1 -> 2
        ^         |
        |_________|

        BFS from 0 should visit all three.
        */
        std::vector<std::vector<int>> mat = {
            {0,1,0},
            {0,0,1},
            {1,0,0}
        };
        auto g = DirectedGraph<int,int>::fromAdjacencyMatrix(mat);
        REQUIRE(g.has_value());
        auto start = *g->beginVertices();
        auto result = bfs(*g, start);
        REQUIRE(result.size() == 3);
        REQUIRE(result[0].data() == 0);
        std::set<int> rest = {result[1].data(), result[2].data()};
        REQUIRE(rest == std::set<int>{1,2});
    }

    SECTION("Multiple arcs (parallel) are ignored") {
        // 0 -> 1 (two arcs), 1 -> 2
        DirectedPseudoGraph<int, void> pseudo;
        auto v0 = pseudo.addVertex(0);
        auto v1 = pseudo.addVertex(1);
        auto v2 = pseudo.addVertex(2);
        pseudo.addArc(v0, v1);
        pseudo.addArc(v0, v1); // second parallel
        pseudo.addArc(v1, v2);
        DirectedGraph<int, void> g(pseudo);
        auto result = bfs(g, v0);
        REQUIRE(result.size() == 3);
        REQUIRE(result[0].data() == 0);
        REQUIRE(result[1].data() == 1);
        REQUIRE(result[2].data() == 2);
    }

    SECTION("Self-loop does not cause infinite loop") {
        // 0 -> 0 (loop), 0 -> 1
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
