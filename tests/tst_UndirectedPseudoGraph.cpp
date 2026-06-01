#include <catch2/catch_test_macros.hpp>

#include "incident/UndirectedPseudoGraph.hpp"

using namespace exx::incident;

// Вспомогательная функция для проверки наличия вершины в графе через линейный поиск
template<typename Graph>
bool hasVertex(const Graph& g, const typename Graph::VertexData& data) {
    for (auto v : g.vertices())
        if (v.data() == data) return true;
    return false;
}

// Вспомогательная функция для проверки наличия ребра через линейный поиск
template<typename Graph>
bool hasEdge(const Graph& g, const typename Graph::EdgeData& data) {
    for (auto e : g.edges())
        if (e.data() == data) return true;
    return false;
}

TEST_CASE("UndirectedPseudoGraph comprehensive tests", "[graph][pseudograph]") {
    SECTION("Unique vertices with hash (int vertices, void edges)") {
        UndirectedPseudoGraph<int, void, std::hash<int>, void> g;

        // Добавление уникальных вершин
        auto v1 = g.addVertex(10);
        REQUIRE(v1.has_value());
        auto v2 = g.addVertex(20);
        REQUIRE(v2.has_value());
        auto v3 = g.addVertex(10); // дубликат
        REQUIRE_FALSE(v3.has_value()); // не добавилась

        REQUIRE(g.vertexCount() == 2);
        REQUIRE(g.findVertex(10).has_value());
        REQUIRE(g.findVertex(20).has_value());
        REQUIRE_FALSE(g.findVertex(30).has_value());

        // Добавление ребра между v1 и v2
        auto e1 = g.addEdge(*v1, *v2);
        REQUIRE(g.edgeCount() == 1);
        REQUIRE(e1.v1().data() == 10);
        REQUIRE(e1.v2().data() == 20);

        // Поиск рёбер через findEdge (EdgeHash == void, но метод findEdge доступен только для EdgeData не void)
        // Здесь EdgeData == void, поэтому findEdge не существует – пропускаем.

        // Удаление вершины – должно удалить и инцидентное ребро
        g.removeVertex(*v1);
        REQUIRE(g.vertexCount() == 1);
        REQUIRE(g.edgeCount() == 0);
        REQUIRE_FALSE(g.findVertex(10).has_value());
        REQUIRE(g.findVertex(20).has_value());
    }

    SECTION("Unique vertices and edges with hash (int vertices, int edges)") {
        UndirectedPseudoGraph<int, int, std::hash<int>, std::hash<int>> g;

        // Вершины
        auto v1 = g.addVertex(1);
        auto v2 = g.addVertex(2);
        auto v3 = g.addVertex(3);
        REQUIRE(v1.has_value());
        REQUIRE(v2.has_value());
        REQUIRE(v3.has_value());

        // Добавление рёбер с данными (уникальность рёбер НЕ контролируется, только вершин)
        auto e1 = g.addEdge(*v1, *v2, 100);
        auto e2 = g.addEdge(*v2, *v3, 200);
        auto e3 = g.addEdge(*v1, *v3, 100); // такие же данные, как у e1 – допустимо
        REQUIRE(g.edgeCount() == 3);

        // findEdge возвращает диапазон (через ranges)
        auto edges100 = g.findEdge(100);
        auto it = edges100.begin();
        REQUIRE(std::distance(edges100.begin(), edges100.end()) == 2);

        // containsEdge
        REQUIRE(g.containsEdge(100));
        REQUIRE(g.containsEdge(200));
        REQUIRE_FALSE(g.containsEdge(999));

        // Удаление одного ребра по дескриптору (должно удалить только e1, не e3)
        g.removeEdge(e1);
        REQUIRE(g.edgeCount() == 2);
        auto edges100_after = g.findEdge(100);
        REQUIRE(std::distance(edges100_after.begin(), edges100_after.end()) == 1);

        // Проверка findVertex
        auto found = g.findVertex(2);
        REQUIRE(found.has_value());
        REQUIRE(found->data() == 2);

        // Удаление вершины – удаляет все её инцидентные рёбра (e2 и e3)
        g.removeVertex(*v2);
        REQUIRE(g.vertexCount() == 2);
        REQUIRE(g.edgeCount() == 1);
        REQUIRE_FALSE(g.findVertex(2).has_value());
    }

    SECTION("No hash (linear search) – VertexHash = void, EdgeHash = void") {
        UndirectedPseudoGraph<std::string, double, void, void> g;

        auto vA = g.addVertex("A");
        auto vB = g.addVertex("B");
        auto vC = g.addVertex("C");
        REQUIRE(vA.has_value());
        REQUIRE(vB.has_value());
        REQUIRE(vC.has_value());

        // Дубликат вершины – не добавится (линейный поиск)
        REQUIRE(g.vertexCount() == 3);
        auto vA2 = g.addVertex("A");
        REQUIRE_FALSE(vA2.has_value());
        REQUIRE(g.vertexCount() == 3);

        // Рёбра с данными
        auto eAB = g.addEdge(*vA, *vB, 1.5);
        g.addEdge(*vB, *vC, 2.5);
        REQUIRE(g.edgeCount() == 2);

        // findEdge использует линейный поиск (возвращает range)
        auto edges_15 = g.findEdge(1.5);
        REQUIRE(std::distance(edges_15.begin(), edges_15.end()) == 1);
        auto edges_99 = g.findEdge(99.0);
        REQUIRE(edges_99.begin() == edges_99.end());

        // containsEdge
        REQUIRE(g.containsEdge(1.5));
        REQUIRE_FALSE(g.containsEdge(0.0));

        // Удаление ребра
        g.removeEdge(eAB);
        REQUIRE(g.edgeCount() == 1);
        REQUIRE_FALSE(g.containsEdge(1.5));

        // Удаление вершины
        g.removeVertex(*vB);
        REQUIRE(g.vertexCount() == 2);
        REQUIRE(g.edgeCount() == 0);
    }

    SECTION("EdgeData = void (edges without data)") {
        UndirectedPseudoGraph<char, void, std::hash<char>, void> g;

        auto vX = g.addVertex('X');
        auto vY = g.addVertex('Y');
        REQUIRE(vX.has_value());
        REQUIRE(vY.has_value());

        // Добавление ребра без данных
        auto e = g.addEdge(*vX, *vY);
        REQUIRE(g.edgeCount() == 1);

        // Проверка инцидентности
        auto incident = vX->incidentEdges();
        auto it = incident.begin();
        REQUIRE(it != incident.end());
        auto other = (*it).otherEnd(*vX);
        REQUIRE(other->data() == 'Y');

        // Удаление ребра
        g.removeEdge(e);
        REQUIRE(g.edgeCount() == 0);
    }

    SECTION("Copy and move semantics") {
        using Graph = UndirectedPseudoGraph<int, std::string, std::hash<int>, std::hash<std::string>>;
        Graph original;
        auto v1 = original.addVertex(1);
        auto v2 = original.addVertex(2);
        original.addEdge(*v1, *v2, "edge12");
        REQUIRE(original.vertexCount() == 2);
        REQUIRE(original.edgeCount() == 1);

        // Копирование
        Graph copy(original);
        REQUIRE(copy.vertexCount() == 2);
        REQUIRE(copy.edgeCount() == 1);
        auto found = copy.findVertex(1);
        REQUIRE(found.has_value());
        // Данные вершины совпадают
        REQUIRE(found->data() == 1);
        // Поиск ребра по данным
        auto edges = copy.findEdge("edge12");
        REQUIRE(std::distance(edges.begin(), edges.end()) == 1);

        // Изменение копии не влияет на оригинал
        copy.removeVertex(*found);
        REQUIRE(copy.vertexCount() == 1);
        REQUIRE(original.vertexCount() == 2);

        // Перемещение
        Graph moved(std::move(original));
        REQUIRE(moved.vertexCount() == 2);
        REQUIRE(moved.edgeCount() == 1);
        REQUIRE(original.vertexCount() == 0); // unspecified but valid state
        // original можно присвоить новый
        original = std::move(moved);
        REQUIRE(original.vertexCount() == 2);
        REQUIRE(moved.vertexCount() == 0);
    }

    SECTION("Constructor from base graph") {
        using BaseGraph = UndirectedAbstractGraph<int, double>;
        BaseGraph base;
        auto v1 = base.addVertex(100);
        auto v2 = base.addVertex(200);
        base.addEdge(v1, v2, 3.14);

        // Создаём псевдограф из базового графа
        UndirectedPseudoGraph<int, double, std::hash<int>, std::hash<double>> g(base);
        REQUIRE(g.vertexCount() == 2);
        REQUIRE(g.edgeCount() == 1);
        REQUIRE(g.findVertex(100).has_value());
        auto edges = g.findEdge(3.14);
        REQUIRE(std::distance(edges.begin(), edges.end()) == 1);
    }

    SECTION("Iterators and ranges") {
        UndirectedPseudoGraph<int, void, std::hash<int>, void> g;
        for (int i = 0; i < 5; ++i)
            g.addVertex(i);
        // Добавим несколько рёбер (без данных)
        auto v0 = *g.findVertex(0);
        auto v1 = *g.findVertex(1);
        auto v2 = *g.findVertex(2);
        g.addEdge(v0, v1);
        g.addEdge(v1, v2);
        g.addEdge(v0, v2);

        // Проверка vertex-итераторов
        int sum = 0;
        for (auto v : g.vertices())
            sum += v.data();
        REQUIRE(sum == 0+1+2+3+4);

        // Проверка edge-итераторов (по числу рёбер)
        std::size_t edgeCount = 0;
        for (auto e : g.edges())
            ++edgeCount;
        REQUIRE(edgeCount == 3);

        // const итераторы (через const ссылку)
        const auto& cg = g;
        sum = 0;
        for (auto v : cg.vertices())
            sum += v.data();
        REQUIRE(sum == 10);
    }
}
