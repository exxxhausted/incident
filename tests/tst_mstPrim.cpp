#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include "incident/UndirectedAbstractGraph.hpp"
#include "incident/algorithms/creation_of_UndirectedAbstractGraph.hpp"
#include "incident/algorithms/mstPrim.hpp"

using namespace exx::incident;

// Вспомогательная функция: суммарный вес рёбер графа
template<typename VD, typename ED>
ED totalWeight(const UndirectedAbstractGraph<VD, ED>& g) {
    ED sum{};
    for (auto e : g.edges()) sum += e.data();
    return sum;
}

// Вспомогательная функция: существуют ли рёбра с заданными парами (без учёта порядка)
template<typename VD, typename ED>
bool hasEdge(const UndirectedAbstractGraph<VD, ED>& g, int u, int v, ED w) {
    for (auto e : g.edges()) {
        int a = e.v1().data();
        int b = e.v2().data();
        if ((a == u && b == v) || (a == v && b == u)) return e.data() == w;
    }
    return false;
}

TEST_CASE("mstPrim works correctly on simple graphs", "[prim]") {
    SECTION("Graph 1: triangle with weights 1,2,3") {
        // Матрица смежности для треугольника (0-1:1, 0-2:2, 1-2:3)
        int raw[] = {
            0, 1, 2,
            1, 0, 3,
            2, 3, 0
        };
        auto view = MatrixView<int>(raw, 3, 3);
        auto graph = make_graph_from_matrix<int>(view, 0);
        auto mst = mstPrim(graph);

        // MST должен содержать 2 ребра, общий вес 1+2 = 3
        REQUIRE(mst.vertexCount() == 3);
        REQUIRE(mst.edgeCount() == 2);
        REQUIRE(totalWeight(mst) == 3);
        REQUIRE(hasEdge(mst, 0, 1, 1));
        REQUIRE(hasEdge(mst, 0, 2, 2));
        // Ребро 1-2 (вес 3) не должно быть
        REQUIRE_FALSE(hasEdge(mst, 1, 2, 3));
    }

    SECTION("Graph 2: square with diagonal (K4 with one missing)") {
        // 4 вершины, рёбра: 0-1:2, 0-2:1, 0-3:4, 1-2:3, 2-3:5
        int raw[] = {
            0, 2, 1, 4,
            2, 0, 3, 0,
            1, 3, 0, 5,
            4, 0, 5, 0
        };
        auto view = MatrixView<int>(raw, 4, 4);
        auto graph = make_graph_from_matrix<int>(view, 0);
        auto mst = mstPrim(graph);

        // MST должен весить 1+2+3 = 6 (рёбра 0-2, 0-1, 1-2? Нет, 1-2 даст цикл. Правильно: 0-2(1), 0-1(2), 0-3(4) – это вес 7, но есть вариант: 0-2(1), 1-2(3), 2-3(5) – вес 9; или 0-2(1), 0-1(2), 2-3(5) – вес 8. На самом деле минимальный MST: 0-2(1), 0-1(2), 2-3(5) сумма 8? Но есть 0-3(4) дешевле 5, значит: 0-2(1), 0-1(2), 0-3(4) = 7. Проверим: три ребра, все инцидентны 0, не образуют цикла. Да, это MST весом 7.
        REQUIRE(mst.vertexCount() == 4);
        REQUIRE(mst.edgeCount() == 3);
        REQUIRE(totalWeight(mst) == 7);
        REQUIRE(hasEdge(mst, 0, 1, 2));
        REQUIRE(hasEdge(mst, 0, 2, 1));
        REQUIRE(hasEdge(mst, 0, 3, 4));
    }
}

TEST_CASE("mstPrim throws on invalid input", "[prim]") {
    SECTION("Duplicate vertex data") {
        // Создаём граф с двумя вершинами, у которых одинаковые данные (оба = 1)
        UndirectedAbstractGraph<int, int> g;
        auto v1 = g.addVertex(1);
        auto v2 = g.addVertex(1); // одинаковое значение
        g.addEdge(v1, v2, 10);
        // Должно выбросить исключение при вызове mstPrim
        REQUIRE_THROWS_AS(mstPrim(g), std::invalid_argument);
        // Проверим сообщение (опционально)
        try {
            mstPrim(g);
        } catch (const std::invalid_argument& e) {
            REQUIRE(std::string(e.what()).find("unique") != std::string::npos);
        }
    }

    SECTION("Disconnected graph") {
        // Граф из двух компонент: 0-1 вес 5, и изолированная вершина 2
        int raw[] = {
            0, 5, 0,
            5, 0, 0,
            0, 0, 0
        };
        auto view = MatrixView<int>(raw, 3, 3);
        auto graph = make_graph_from_matrix<int>(view, 0);
        REQUIRE_THROWS_AS(mstPrim(graph), std::logic_error);
        try {
            mstPrim(graph);
        } catch (const std::logic_error& e) {
            REQUIRE(std::string(e.what()).find("disconnected") != std::string::npos);
        }
    }
}
