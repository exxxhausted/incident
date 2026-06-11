#include <catch2/catch_test_macros.hpp>

#include <set>
#include <vector>
#include <unordered_map>

#include "incident/algorithms/bfs.hpp"
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

TEST_CASE("BFS on undirected graph", "[bfs][undirected]") {
    SECTION("BFS on simple connected graph") {
        // 0 -- 1 -- 3
        // |    |
        // 2 __/
        std::vector<bool> mat = {
            0,1,1,0,
            1,0,1,1,
            1,1,0,0,
            0,1,0,0
        };
        auto g = makeUndirectedGraphFromBoolMatrix(mat, 4);
        auto start = *g.beginVertices(); // вершина 0
        auto bfs_result = bfs(g, start);

        const auto& order = bfs_result.order();
        REQUIRE(order.size() == 4);
        REQUIRE(order[0].data() == 0);

        // Второй слой (глубина 1) должен содержать 1 и 2 в любом порядке
        std::set<int> depth1_vertices;
        for (auto v : order) {
            if (*bfs_result.depth(v) == 1)
                depth1_vertices.insert(v.data());
        }
        REQUIRE(depth1_vertices == std::set<int>{1, 2});
        // Глубина 3: вершина 3
        REQUIRE(*bfs_result.depth(order[3]) == 2); // расстояние от 0 до 3 равно 2
        REQUIRE(order[3].data() == 3);
    }

    SECTION("BFS handles disconnected graph") {
        // 0 -- 1    2 -- 3
        std::vector<bool> mat = {
            0,1,0,0,
            1,0,0,0,
            0,0,0,1,
            0,0,1,0
        };
        auto g = makeUndirectedGraphFromBoolMatrix(mat, 4);
        auto start = *g.beginVertices(); // 0
        auto bfs_result = bfs(g, start);

        const auto& order = bfs_result.order();
        REQUIRE(order.size() == 2);
        REQUIRE(order[0].data() == 0);
        REQUIRE(order[1].data() == 1);
        // Вершины 2 и 3 недостижимы
        for (auto v : g.vertices()) {
            if (v.data() == 2 || v.data() == 3)
                REQUIRE(!bfs_result.isReachable(v));
        }
    }

    SECTION("BFS ignores multiedge revisits") {
        // Base: 0-1-2, добавим параллельные рёбра 0-1
        std::vector<bool> mat = {0,1,0, 1,0,1, 0,1,0};
        auto simple = makeUndirectedGraphFromBoolMatrix(mat, 3);
        UndirectedMultiGraph<int, void> multi(simple.baseMultiGraph());

        // Сохраним отображение индекс -> дескриптор
        std::unordered_map<int, decltype(multi)::VertexDescriptor> desc;
        for (auto v : multi.vertices())
            desc[v.data()] = v;

        auto v0 = desc[0];
        auto v1 = desc[1];
        multi.addEdge(v0, v1); // параллельное ребро
        multi.addEdge(v0, v1);
        auto bfs_result = bfs(multi, v0);

        const auto& order = bfs_result.order();
        REQUIRE(order.size() == 3);
        REQUIRE(order[0].data() == 0);
        REQUIRE(order[1].data() == 1);
        REQUIRE(order[2].data() == 2);
        // Проверим, что глубина увеличилась корректно
        REQUIRE(*bfs_result.depth(v1) == 1);
        REQUIRE(*bfs_result.depth(desc[2]) == 2);
    }

    SECTION("BFS handles self-loops") {
        // Base: 0-1-2, добавим петлю на вершине 1
        std::vector<bool> mat = {0,1,0, 1,0,1, 0,1,0};
        auto simple = makeUndirectedGraphFromBoolMatrix(mat, 3);
        UndirectedPseudoGraph<int, void> pseudo(simple.baseMultiGraph().basePseudoGraph());

        std::unordered_map<int, decltype(pseudo)::VertexDescriptor> desc;
        for (auto v : pseudo.vertices())
            desc[v.data()] = v;

        auto v1 = desc[1];
        pseudo.addEdge(v1, v1);
        auto v0 = desc[0];
        auto bfs_result = bfs(pseudo, v0);
        const auto& order = bfs_result.order();
        REQUIRE(order.size() == 3);
        REQUIRE(order[0].data() == 0);
        REQUIRE(order[1].data() == 1);
        REQUIRE(order[2].data() == 2);
        // Петля не должна повлиять на глубину
        REQUIRE(*bfs_result.depth(v1) == 1);
    }
}

TEST_CASE("BFS on directed graph", "[bfs][directed]") {
    SECTION("Simple DAG") {
        // 0 -> 1 -> 3
        // |    |
        // v    v
        // 2 <--
        std::vector<std::vector<bool>> mat = {
            {0,1,1,0},
            {0,0,1,1},
            {0,0,0,0},
            {0,0,0,0}
        };
        auto g = makeDirectedGraphFromBoolMatrix(mat);
        auto start = *g.beginVertices(); // 0
        auto bfs_result = bfs(g, start);

        const auto& order = bfs_result.order();
        REQUIRE(order.size() == 4);
        REQUIRE(order[0].data() == 0);

        // Второй слой: 1 и 2 (глубина 1)
        std::set<int> depth1_vertices;
        for (auto v : order) {
            if (*bfs_result.depth(v) == 1)
                depth1_vertices.insert(v.data());
        }
        REQUIRE(depth1_vertices == std::set<int>{1, 2});

        // Вершина 3 на глубине 2
        REQUIRE(*bfs_result.depth(order[3]) == 2);
        REQUIRE(order[3].data() == 3);
    }

    SECTION("Disconnected directed graph") {
        // Comp1: 0->1, Comp2: 2->3
        std::vector<std::vector<bool>> mat = {
            {0,1,0,0},
            {0,0,0,0},
            {0,0,0,1},
            {0,0,0,0}
        };
        auto g = makeDirectedGraphFromBoolMatrix(mat);
        auto start = *g.beginVertices(); // 0
        auto bfs_result = bfs(g, start);

        const auto& order = bfs_result.order();
        REQUIRE(order.size() == 2);
        REQUIRE(order[0].data() == 0);
        REQUIRE(order[1].data() == 1);
        // Вершины 2 и 3 недостижимы
        for (auto v : g.vertices()) {
            if (v.data() == 2 || v.data() == 3)
                REQUIRE(!bfs_result.isReachable(v));
        }
    }

    SECTION("Graph with cycle") {
        // 0 -> 1 -> 2
        // ^         |
        // |_________|
        std::vector<std::vector<bool>> mat = {
            {0,1,0},
            {0,0,1},
            {1,0,0}
        };
        auto g = makeDirectedGraphFromBoolMatrix(mat);
        auto start = *g.beginVertices(); // 0
        auto bfs_result = bfs(g, start);

        const auto& order = bfs_result.order();
        REQUIRE(order.size() == 3);
        REQUIRE(order[0].data() == 0);
        REQUIRE(order[1].data() == 1);
        REQUIRE(order[2].data() == 2);

        // Глубины: 0 на глубине 0, 1 на глубине 1, 2 на глубине 2
        REQUIRE(*bfs_result.depth(order[0]) == 0);
        REQUIRE(*bfs_result.depth(order[1]) == 1);
        REQUIRE(*bfs_result.depth(order[2]) == 2);
    }

    SECTION("Multiple arcs (parallel) ignored") {
        DirectedPseudoGraph<int, void> pseudo;
        auto v0 = pseudo.addVertex(0);
        auto v1 = pseudo.addVertex(1);
        auto v2 = pseudo.addVertex(2);
        pseudo.addArc(v0, v1);
        pseudo.addArc(v0, v1); // параллельная дуга
        pseudo.addArc(v1, v2);
        DirectedGraph<int, void> g(pseudo);
        auto bfs_result = bfs(g, v0);

        const auto& order = bfs_result.order();
        REQUIRE(order.size() == 3);
        REQUIRE(order[0].data() == 0);
        REQUIRE(order[1].data() == 1);
        REQUIRE(order[2].data() == 2);
        REQUIRE(*bfs_result.depth(v1) == 1);
        REQUIRE(*bfs_result.depth(v2) == 2);
    }

    SECTION("Self-loop does not cause infinite loop") {
        DirectedPseudoGraph<int, void> pseudo;
        auto v0 = pseudo.addVertex(0);
        auto v1 = pseudo.addVertex(1);
        pseudo.addArc(v0, v0);
        pseudo.addArc(v0, v1);
        DirectedGraph<int, void> g(pseudo);
        auto bfs_result = bfs(g, v0);

        const auto& order = bfs_result.order();
        REQUIRE(order.size() == 2);
        REQUIRE(order[0].data() == 0);
        REQUIRE(order[1].data() == 1);
        REQUIRE(*bfs_result.depth(v1) == 1);
        // Петля не создала дополнительных вершин
        REQUIRE(!bfs_result.parent(v0).has_value()); // у корня нет родителя
    }
}
