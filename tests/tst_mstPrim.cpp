#include <catch2/catch_test_macros.hpp>

#include "incident/UndirectedGraph.hpp"
#include "incident/algorithms/mstPrim.hpp"

using namespace exx::incident;

template<typename VD, typename ED>
ED totalWeight(const UndirectedGraph<VD, ED>& g) {
    ED sum{};
    for (auto e : g.edges()) sum = sum + e.data();
    return sum;
}

TEST_CASE("mstPrim algorithm", "[prim][mst]") {
    // 1. Граф с одной вершиной
    SECTION("Single vertex") {
        UndirectedGraph<int, int> g;
        g.addVertex(1);
        auto result = mstPrim(g);
        REQUIRE(result.has_value());
        auto& mst = result.value();
        CHECK(mst.vertexCount() == 1);
        CHECK(mst.edgeCount() == 0);
    }

    // 2. Две вершины, одно ребро
    SECTION("Two vertices one edge") {
        UndirectedGraph<int, int> g;
        g.addVertex(1);
        g.addVertex(2);
        g.addEdge(1, 2, 10);
        auto result = mstPrim(g);
        REQUIRE(result.has_value());
        auto& mst = result.value();
        CHECK(mst.vertexCount() == 2);
        CHECK(mst.edgeCount() == 1);
        auto edge = mst.findEdge(1, 2);
        REQUIRE(edge.has_value());
        CHECK(edge->data() == 10);
    }

    // 3. Три вершины, три ребра (выбираем два наименьших)
    SECTION("Three vertices triangle") {
        UndirectedGraph<int, int> g;
        for (int v : {0,1,2}) g.addVertex(v);
        g.addEdge(0,1,5);
        g.addEdge(1,2,3);
        g.addEdge(0,2,7);
        auto result = mstPrim(g);
        REQUIRE(result.has_value());
        auto& mst = result.value();
        CHECK(mst.vertexCount() == 3);
        CHECK(mst.edgeCount() == 2);
        CHECK(totalWeight(mst) == 8);
        CHECK(g.hasEdge(0,1));
        CHECK(g.hasEdge(1,2));
    }

    // 4. Одинаковые веса – любое MST, проверяем вес и связность
    SECTION("Equal weights") {
        UndirectedGraph<int, int> g;
        for (int v : {0,1,2}) g.addVertex(v);
        g.addEdge(0,1, 2);
        g.addEdge(1,2, 2);
        g.addEdge(0,2, 2);
        auto result = mstPrim(g);
        REQUIRE(result.has_value());
        auto& mst = result.value();
        CHECK(mst.vertexCount() == 3);
        CHECK(mst.edgeCount() == 2);
        CHECK(totalWeight(mst) == 4);
    }

    // 5. Полный граф на 4 вершинах
    SECTION("Complete graph K4") {
        UndirectedGraph<int, int> g;
        for (int v : {0,1,2,3}) g.addVertex(v);
        g.addEdge(0,1, 1);
        g.addEdge(0,2, 2);
        g.addEdge(0,3, 3);
        g.addEdge(1,2, 4);
        g.addEdge(1,3, 5);
        g.addEdge(2,3, 6);
        auto result = mstPrim(g);
        REQUIRE(result.has_value());
        auto& mst = result.value();
        CHECK(mst.vertexCount() == 4);
        CHECK(mst.edgeCount() == 3);
        // Ожидаемый вес: 1+2+3 = 6 (ребра 0-1,0-2,0-3)
        CHECK(totalWeight(mst) == 6);
        for (auto e : g.edges())
            CHECK(g.hasEdge(e.v1(), e.v2()));
    }

    // 6. Отрицательные веса
    SECTION("Negative weights") {
        UndirectedGraph<int, int> g;
        for (int v : {0,1,2}) g.addVertex(v);
        g.addEdge(0,1, -5);
        g.addEdge(1,2, 1);
        g.addEdge(0,2, 10);
        auto result = mstPrim(g);
        REQUIRE(result.has_value());
        auto& mst = result.value();
        CHECK(mst.edgeCount() == 2);
        // MST должен взять -5 и 1 -> сумма -4
        CHECK(totalWeight(mst) == -4);
        CHECK(g.hasEdge(0,1));
        CHECK(g.hasEdge(1,2));
    }

    // 7. Несвязный граф – ошибка
    SECTION("Disconnected graph") {
        UndirectedGraph<int, int> g;
        g.addVertex(0);
        g.addVertex(1);
        g.addVertex(2);
        g.addEdge(0,1,5);
        // вершина 2 изолирована
        auto result = mstPrim(g);
        REQUIRE(!result.has_value());
        CHECK(result.error() == PrimError::DisconnectedGraph);
    }

    // 8. Более сложный граф (5 вершин, не полный)
    SECTION("Custom 5-vertex graph") {
        // Вершины: 0-1-2-3-4, дополнительные рёбра
        UndirectedGraph<int, int> g;
        for (int v : {0,1,2,3,4}) g.addVertex(v);
        g.addEdge(0,1, 2);
        g.addEdge(1,2, 3);
        g.addEdge(2,3, 1);
        g.addEdge(3,4, 4);
        g.addEdge(0,4, 7);
        g.addEdge(1,3, 5);
        g.addEdge(0,2, 6);
        // Ожидаемый MST: рёбра (0-1:2), (1-2:3), (2-3:1), (3-4:4) сумма = 10
        auto result = mstPrim(g);
        REQUIRE(result.has_value());
        auto& mst = result.value();
        CHECK(mst.vertexCount() == 5);
        CHECK(mst.edgeCount() == 4);
        CHECK(totalWeight(mst) == 10);
        for (int v : {0,1,2,3,4}) REQUIRE(mst.findVertex(v).has_value());
    }

    // 9. Пустой граф (0 вершин)
    SECTION("Empty graph") {
        UndirectedGraph<int, int> g;
        auto result = mstPrim(g);
        REQUIRE(result.has_value());
        auto& mst = result.value();
        CHECK(mst.vertexCount() == 0);
        CHECK(mst.edgeCount() == 0);
    }
}
