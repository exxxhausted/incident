#include <catch2/catch_test_macros.hpp>
#include <vector>

#include "incident/algorithms/dfsUndirected.hpp"
#include "incident/io/UndirectedGraphFromMatrix.hpp"

using namespace exx::incident;

// Вспомогательная функция для получения вектора идентификаторов из вектора дескрипторов
std::vector<std::size_t> toIds(const auto& descriptorVec) {
    std::vector<std::size_t> ids;
    for (const auto& desc : descriptorVec) {
        ids.push_back(desc.data());
    }
    return ids;
}

TEST_CASE("DFS order on undirected graph", "[dfs][order]") {
    using Graph = UndirectedGraph<std::size_t, int>;

    SECTION("Single vertex") {
        int matrix[1][1] = {{0}};
        auto graphExpected = buildUndirectedGraph(&matrix[0][0], 1);
        REQUIRE(graphExpected.has_value());
        auto& G = *graphExpected;
        auto start = G.findVertex(0);
        REQUIRE(start);

        auto result = dfs(G.baseMultiGraph().basePseudoGraph().baseAbstractGraph(), *start);
        auto ids = toIds(result);

        REQUIRE(ids == std::vector<std::size_t>{0});
    }

    SECTION("Line of two vertices: 0-1, start 0") {
        int matrix[2][2] = {{0,1},{1,0}};
        auto graphExpected = buildUndirectedGraph(&matrix[0][0], 2);
        REQUIRE(graphExpected.has_value());
        auto& G = *graphExpected;
        auto start = G.findVertex(0);

        auto result = dfs(G.baseMultiGraph().basePseudoGraph().baseAbstractGraph(), *start);
        auto ids = toIds(result);

        // Единственный возможный порядок: 0, затем 1
        REQUIRE(ids == std::vector<std::size_t>{0, 1});
    }

    SECTION("Line of three vertices: 0-1-2, start 0") {
        int matrix[3][3] = {
            {0,1,0},
            {1,0,1},
            {0,1,0}
        };
        auto graphExpected = buildUndirectedGraph(&matrix[0][0], 3);
        REQUIRE(graphExpected.has_value());
        auto& G = *graphExpected;
        auto start = G.findVertex(0);

        auto result = dfs(G.baseMultiGraph().basePseudoGraph().baseAbstractGraph(), *start);
        auto ids = toIds(result);

        // DFS вдоль цепи: 0 -> 1 -> 2
        REQUIRE(ids == std::vector<std::size_t>{0, 1, 2});
    }

    SECTION("Line of three vertices, start at middle (1)") {
        int matrix[3][3] = {
            {0,1,0},
            {1,0,1},
            {0,1,0}
        };
        auto graphExpected = buildUndirectedGraph(&matrix[0][0], 3);
        REQUIRE(graphExpected.has_value());
        auto& G = *graphExpected;
        auto start = G.findVertex(1);

        auto result = dfs(G.baseMultiGraph().basePseudoGraph().baseAbstractGraph(), *start);
        auto ids = toIds(result);
        REQUIRE(ids == std::vector<std::size_t>{1, 2, 0});
    }

    SECTION("Star graph: center 0 connected to 1,2,3; start 0") {
        // 0 связан с 1,2,3; остальные только с 0
        int matrix[4][4] = {
            {0,1,1,1},
            {1,0,0,0},
            {1,0,0,0},
            {1,0,0,0}
        };
        auto graphExpected = buildUndirectedGraph(&matrix[0][0], 4);
        REQUIRE(graphExpected.has_value());
        auto& G = *graphExpected;
        auto start = G.findVertex(0);

        auto result = dfs(G.baseMultiGraph().basePseudoGraph().baseAbstractGraph(), *start);
        auto ids = toIds(result);
        REQUIRE(ids == std::vector<std::size_t>{0, 3, 2, 1});
    }

    SECTION("Graph with a loop: 0-1 and loop on 0, start 0") {
        int matrix[2][2] = {{1,1},{1,0}};
        auto graphExpected = buildUndirectedGraph(&matrix[0][0], 2);
        REQUIRE(graphExpected.has_value());
        auto& G = *graphExpected;
        auto start = G.findVertex(0);

        auto result = dfs(G.baseMultiGraph().basePseudoGraph().baseAbstractGraph(), *start);
        auto ids = toIds(result);

        REQUIRE(ids == std::vector<std::size_t>{0, 1});
    }

    SECTION("Graph with parallel edges: 0-1 (two edges), start 0") {
        int matrix[2][2] = {{0,2},{2,0}};
        auto graphExpected = buildUndirectedGraph(&matrix[0][0], 2);
        REQUIRE(graphExpected.has_value());
        auto& G = *graphExpected;
        auto start = G.findVertex(0);

        auto result = dfs(G.baseMultiGraph().basePseudoGraph().baseAbstractGraph(), *start);
        auto ids = toIds(result);

        // Кратные рёбра не меняют порядок: 0,1.
        REQUIRE(ids == std::vector<std::size_t>{0, 1});
    }

    SECTION("Two components, start in first, order within first") {
        // Компонента1: 0-1, компонента2: 2-3
        int matrix[4][4] = {
            {0,1,0,0},
            {1,0,0,0},
            {0,0,0,1},
            {0,0,1,0}
        };
        auto graphExpected = buildUndirectedGraph(&matrix[0][0], 4);
        REQUIRE(graphExpected.has_value());
        auto& G = *graphExpected;
        auto start = G.findVertex(0);

        auto result = dfs(G.baseMultiGraph().basePseudoGraph().baseAbstractGraph(), *start);
        auto ids = toIds(result);

        REQUIRE(ids == std::vector<std::size_t>{0, 1});
    }
}
