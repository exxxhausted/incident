#include <catch2/catch_test_macros.hpp>
#include "incident/UndirectedAbstractGraph.hpp"

using namespace exx::incident;

TEST_CASE("UndirectedAbstractGraph works correctly", "[undirected]") {
    SECTION("Empty graph") {
        UndirectedAbstractGraph<int, int> g;
        REQUIRE(g.vertexCount() == 0);
        REQUIRE(g.edgeCount() == 0);
    }

    SECTION("Add vertices") {
        UndirectedAbstractGraph<std::string, int> g;
        auto v1 = g.addVertex("A");
        auto v2 = g.addVertex("B");
        auto v3 = g.addVertex("C");

        REQUIRE(g.vertexCount() == 3);
        REQUIRE(v1.data() == "A");
        REQUIRE(v2.data() == "B");
        REQUIRE(v3.data() == "C");
    }

    SECTION("Add edges") {
        UndirectedAbstractGraph<int, int> g;
        auto v1 = g.addVertex(1);
        auto v2 = g.addVertex(2);
        auto e = g.addEdge(v1, v2, 42);

        REQUIRE(g.edgeCount() == 1);
        REQUIRE(e.data() == 42);
        REQUIRE(e.v1().data() == 1);
        REQUIRE(e.v2().data() == 2);
    }

    SECTION("Incident edges iteration") {
        UndirectedAbstractGraph<int, int> g;
        auto v1 = g.addVertex(1);
        auto v2 = g.addVertex(2);
        auto v3 = g.addVertex(3);
        g.addEdge(v1, v2, 10);
        g.addEdge(v1, v3, 20);

        auto incident = v1.incidentEdges();
        auto it = incident.begin();
        REQUIRE(it != incident.end());
        // Порядок не определён, но оба ребра должны присутствовать
        int sum = 0;
        for (auto edge : incident) {
            sum += edge.data();
        }
        REQUIRE(sum == 30);
    }

    SECTION("otherEnd works") {
        UndirectedAbstractGraph<int, int> g;
        auto v1 = g.addVertex(1);
        auto v2 = g.addVertex(2);
        auto e = g.addEdge(v1, v2, 99);

        REQUIRE(e.otherEnd(v1)->data() == 2);
        REQUIRE(e.otherEnd(v2)->data() == 1);
    }

    SECTION("Remove edge") {
        UndirectedAbstractGraph<int, int> g;
        auto v1 = g.addVertex(1);
        auto v2 = g.addVertex(2);
        auto e = g.addEdge(v1, v2, 123);
        REQUIRE(g.edgeCount() == 1);

        g.removeEdge(e);
        REQUIRE(g.edgeCount() == 0);
        // Списки инцидентности вершин должны быть пусты
        REQUIRE(v1.incidentEdges().empty());
        REQUIRE(v2.incidentEdges().empty());
    }

    SECTION("Remove vertex also removes its edges") {
        UndirectedAbstractGraph<int, int> g;
        auto v1 = g.addVertex(1);
        auto v2 = g.addVertex(2);
        auto v3 = g.addVertex(3);
        g.addEdge(v1, v2, 5);
        g.addEdge(v2, v3, 6);
        g.addEdge(v1, v3, 7);

        REQUIRE(g.vertexCount() == 3);
        REQUIRE(g.edgeCount() == 3);

        g.removeVertex(v2);
        REQUIRE(g.vertexCount() == 2);
        REQUIRE(g.edgeCount() == 1); // остаётся ребро (v1, v3)

        // Проверяем, что у оставшихся вершин правильные рёбра
        auto incidentV1 = v1.incidentEdges();
        REQUIRE(std::distance(incidentV1.begin(), incidentV1.end()) == 1);
        REQUIRE((*incidentV1.begin()).otherEnd(v1)->data() == 3);
    }

    SECTION("Multiple edges are allowed") {
        UndirectedAbstractGraph<int, int> g;
        auto v1 = g.addVertex(1);
        auto v2 = g.addVertex(2);
        g.addEdge(v1, v2, 10);
        g.addEdge(v1, v2, 20);
        REQUIRE(g.edgeCount() == 2);
        auto incident = v1.incidentEdges();
        int sum = 0;
        for (auto e : incident) sum += e.data();
        REQUIRE(sum == 30);
    }
}
