#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "incident/algorithms/bfs.hpp"
#include "incident/undirected/UndirectedGraph.hpp"
#include "incident/directed/DirectedGraph.hpp".hpp"

using namespace exx::incident;

TEST_CASE("BFS on undirected graphs", "[bfs][undirected]") {
    SECTION("Path graph") {
        UndirectedGraph<void, void> g;
        auto v1 = g.addVertex(); // 0
        auto v2 = g.addVertex(); // 1
        auto v3 = g.addVertex(); // 2
        auto v4 = g.addVertex(); // 3
        g.addEdge(v1, v2);
        g.addEdge(v2, v3);
        g.addEdge(v3, v4);

        SECTION("Start from one end") {
            auto forest = bfs(g, v1);
            REQUIRE(forest.isReachable(v1));
            REQUIRE(forest.isReachable(v2));
            REQUIRE(forest.isReachable(v3));
            REQUIRE(forest.isReachable(v4));

            REQUIRE(forest.depth(v1) == 0);
            REQUIRE(forest.depth(v2) == 1);
            REQUIRE(forest.depth(v3) == 2);
            REQUIRE(forest.depth(v4) == 3);

            REQUIRE(forest.parent(v1) == std::nullopt);
            REQUIRE(forest.parent(v2) == v1);
            REQUIRE(forest.parent(v3) == v2);
            REQUIRE(forest.parent(v4) == v3);
        }

        SECTION("Start from middle") {
            auto forest = bfs(g, v2);
            REQUIRE(forest.depth(v2) == 0);
            REQUIRE(forest.depth(v1) == 1);
            REQUIRE(forest.depth(v3) == 1);
            REQUIRE(forest.depth(v4) == 2);
        }
    }

    SECTION("Triangle graph") {
        UndirectedGraph<void, void> g;
        auto v1 = g.addVertex();
        auto v2 = g.addVertex();
        auto v3 = g.addVertex();
        g.addEdge(v1, v2);
        g.addEdge(v2, v3);
        g.addEdge(v3, v1);

        auto forest = bfs(g, v1);
        REQUIRE(forest.isReachable(v1));
        REQUIRE(forest.isReachable(v2));
        REQUIRE(forest.isReachable(v3));

        REQUIRE(forest.depth(v1) == 0);
        REQUIRE(forest.depth(v2) == 1);
        REQUIRE(forest.depth(v3) == 1);
    }
}

TEST_CASE("BFS on directed graphs", "[bfs][directed]") {
    SECTION("Directed cycle") {
        DirectedGraph<void, void> g;
        auto v1 = g.addVertex();
        auto v2 = g.addVertex();
        auto v3 = g.addVertex();
        g.addArc(v1, v2);
        g.addArc(v2, v3);
        g.addArc(v3, v1);

        SECTION("Start from v1") {
            auto forest = bfs(g, v1);
            REQUIRE(forest.isReachable(v1));
            REQUIRE(forest.isReachable(v2));
            REQUIRE(forest.isReachable(v3));

            REQUIRE(forest.depth(v1) == 0);
            REQUIRE(forest.depth(v2) == 1);
            REQUIRE(forest.depth(v3) == 2);
            REQUIRE(forest.parent(v3) == v2);
        }

        SECTION("Start from all vertices") {
            auto forest = bfs(g);
            for (auto v : {v1, v2, v3})
                REQUIRE(forest.isReachable(v));

            int rootCount = 0;
            for (auto v : {v1, v2, v3})
                if (forest.depth(v) == 0) ++rootCount;
            REQUIRE(rootCount == 1);
        }
    }

    SECTION("Directed path (no cycles)") {
        DirectedGraph<void, void> g;
        auto a = g.addVertex();
        auto b = g.addVertex();
        auto c = g.addVertex();
        g.addArc(a, b);
        g.addArc(b, c);

        SECTION("Start from a") {
            auto forest = bfs(g, a);
            REQUIRE(forest.isReachable(a));
            REQUIRE(forest.isReachable(b));
            REQUIRE(forest.isReachable(c));
            REQUIRE(forest.depth(a) == 0);
            REQUIRE(forest.depth(b) == 1);
            REQUIRE(forest.depth(c) == 2);
        }

        SECTION("Start from b") {
            auto forest = bfs(g, b);
            REQUIRE_FALSE(forest.isReachable(a));
            REQUIRE(forest.isReachable(b));
            REQUIRE(forest.isReachable(c));
        }
    }
}
