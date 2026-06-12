#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "incident/algorithms/dfs.hpp"
#include "incident/undirected/UndirectedGraph.hpp"
#include "incident/directed/DirectedGraph.hpp"

using namespace exx::incident;

TEST_CASE("DFS on undirected graphs", "[dfs][undirected]") {
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
            auto forest = dfs(g, v1);
            REQUIRE(forest.isReachable(v1));
            REQUIRE(forest.isReachable(v2));
            REQUIRE(forest.isReachable(v3));
            REQUIRE(forest.isReachable(v4));

            REQUIRE(forest.parent(v1) == std::nullopt);
            REQUIRE(forest.parent(v2) == v1);
            REQUIRE((forest.parent(v3) == v2)); // может быть v2 или v4, но в пути v3 через v2
            REQUIRE(forest.parent(v4) == v3);

            REQUIRE(forest.preorder().size() == 4);
            REQUIRE(forest.postorder().size() == 4);
        }

        SECTION("Start from middle") {
            auto forest = dfs(g, v2);
            REQUIRE(forest.isReachable(v1));
            REQUIRE(forest.isReachable(v2));
            REQUIRE(forest.isReachable(v3));
            REQUIRE(forest.isReachable(v4));

            REQUIRE(forest.parent(v2) == std::nullopt);
            // v1 и v3 – дети v2
            REQUIRE((forest.parent(v1) == v2));
            REQUIRE((forest.parent(v3) == v2));
            // v4 – ребёнок v3
            REQUIRE(forest.parent(v4) == v3);
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

        auto forest = dfs(g, v1);
        REQUIRE(forest.isReachable(v1));
        REQUIRE(forest.isReachable(v2));
        REQUIRE(forest.isReachable(v3));

        REQUIRE(forest.preorder().size() == 3);
        REQUIRE(forest.postorder().size() == 3);
        REQUIRE(forest.parent(v1) == std::nullopt);
    }
}

TEST_CASE("DFS on directed graphs", "[dfs][directed]") {
    SECTION("Directed cycle") {
        DirectedGraph<void, void> g;
        auto v1 = g.addVertex();
        auto v2 = g.addVertex();
        auto v3 = g.addVertex();
        g.addArc(v1, v2);
        g.addArc(v2, v3);
        g.addArc(v3, v1);

        auto forest = dfs(g, v1);
        REQUIRE(forest.isReachable(v1));
        REQUIRE(forest.isReachable(v2));
        REQUIRE(forest.isReachable(v3));
        REQUIRE(forest.parent(v1) == std::nullopt);
        REQUIRE(forest.parent(v2) == v1);
        REQUIRE(forest.parent(v3) == v2);
        REQUIRE(forest.preorder().size() == 3);
        REQUIRE(forest.postorder().size() == 3);
    }

    SECTION("Directed path (no cycles)") {
        DirectedGraph<void, void> g;
        auto a = g.addVertex();
        auto b = g.addVertex();
        auto c = g.addVertex();
        g.addArc(a, b);
        g.addArc(b, c);

        SECTION("Start from a") {
            auto forest = dfs(g, a);
            REQUIRE(forest.isReachable(a));
            REQUIRE(forest.isReachable(b));
            REQUIRE(forest.isReachable(c));
            REQUIRE(forest.parent(a) == std::nullopt);
            REQUIRE(forest.parent(b) == a);
            REQUIRE(forest.parent(c) == b);
        }

        SECTION("Start from b") {
            auto forest = dfs(g, b);
            REQUIRE_FALSE(forest.isReachable(a));
            REQUIRE(forest.isReachable(b));
            REQUIRE(forest.isReachable(c));
            REQUIRE(forest.parent(b) == std::nullopt);
            REQUIRE(forest.parent(c) == b);
        }
    }

    SECTION("DFS on whole directed graph (multiple starts)") {
        DirectedGraph<void, void> g;
        auto v1 = g.addVertex();
        auto v2 = g.addVertex();
        g.addArc(v1, v2);
        auto v3 = g.addVertex();
        auto v4 = g.addVertex();
        g.addArc(v3, v4);

        auto forest = dfs(g); // обход со всеми вершинами в качестве стартовых
        REQUIRE(forest.isReachable(v1));
        REQUIRE(forest.isReachable(v2));
        REQUIRE(forest.isReachable(v3));
        REQUIRE(forest.isReachable(v4));

        int rootCount = 0;
        for (auto v : {v1, v2, v3, v4})
            if (!forest.parent(v).has_value()) ++rootCount;
        REQUIRE(rootCount == 2);
    }
}
