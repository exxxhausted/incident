#include <catch2/catch_test_macros.hpp>
#include "incident/DirectedAbstractGraph.hpp"

using namespace exx::incident;
/*
TEST_CASE("DirectedAbstractGraph works correctly", "[directed]") {
    SECTION("Empty graph") {
        DirectedAbstractGraph<int, int> g;
        REQUIRE(g.vertexCount() == 0);
        REQUIRE(g.arcCount() == 0);
    }

    SECTION("Add vertices") {
        DirectedAbstractGraph<std::string, int> g;
        auto v1 = g.addVertex("A");
        auto v2 = g.addVertex("B");
        auto v3 = g.addVertex("C");

        REQUIRE(g.vertexCount() == 3);
        REQUIRE((*v1).data() == "A");
        REQUIRE((*v2).data() == "B");
        REQUIRE((*v3).data() == "C");
    }

    SECTION("Add arcs") {
        DirectedAbstractGraph<int, int> g;
        auto v1 = g.addVertex(1);
        auto v2 = g.addVertex(2);
        auto arc = g.addArc(v1, v2, 42);

        REQUIRE(g.arcCount() == 1);
        REQUIRE((*arc).data() == 42);
        REQUIRE((*(*arc).from()).data() == 1);
        REQUIRE((*(*arc).to()).data() == 2);
    }

    SECTION("Adjacent arcs iteration from vertex") {
        DirectedAbstractGraph<int, int> g;
        auto v1 = g.addVertex(1);
        auto v2 = g.addVertex(2);
        auto v3 = g.addVertex(3);
        g.addArc(v1, v2, 10);
        g.addArc(v1, v3, 20);
        g.addArc(v2, v1, 30); // обратная дуга не должна влиять на исходящие из v1

        auto arcsFromV1 = (*v1).adjacentArcs();
        int sum = 0;
        for (auto arc : arcsFromV1) {
            sum += arc.data();
        }
        REQUIRE(sum == 30); // 10 + 20
    }

    SECTION("Remove arc") {
        DirectedAbstractGraph<int, int> g;
        auto v1 = g.addVertex(1);
        auto v2 = g.addVertex(2);
        auto arc = g.addArc(v1, v2, 123);
        REQUIRE(g.arcCount() == 1);

        g.removeArc(arc);
        REQUIRE(g.arcCount() == 0);
        // Список смежности v1 должен быть пуст
        REQUIRE((*v1).adjacentArcs().empty());
    }

    SECTION("Remove vertex also removes incident arcs") {
        DirectedAbstractGraph<int, int> g;
        auto v1 = g.addVertex(1);
        auto v2 = g.addVertex(2);
        auto v3 = g.addVertex(3);
        g.addArc(v1, v2, 5);
        g.addArc(v2, v1, 6);
        g.addArc(v2, v3, 7);
        g.addArc(v3, v1, 8);

        REQUIRE(g.vertexCount() == 3);
        REQUIRE(g.arcCount() == 4);

        g.removeVertex(v2);
        REQUIRE(g.vertexCount() == 2);
        REQUIRE(g.arcCount() == 1);
        // Проверяем, что у v1 нет исходящих дуг (была только в v2)
        CHECK((*v1).adjacentArcs().empty());
        // У v3 есть исходящая дуга в v1
        auto arcsFromV3 = (*v3).adjacentArcs();
        REQUIRE(std::distance(arcsFromV3.begin(), arcsFromV3.end()) == 1);
        REQUIRE((*(*arcsFromV3.begin()).to()).data() == 1);
    }

    SECTION("Multiple arcs are allowed (parallel arcs)") {
        DirectedAbstractGraph<int, int> g;
        auto v1 = g.addVertex(1);
        auto v2 = g.addVertex(2);
        g.addArc(v1, v2, 10);
        g.addArc(v1, v2, 20);
        g.addArc(v1, v2, 30);

        REQUIRE(g.arcCount() == 3);
        auto arcs = (*v1).adjacentArcs();
        int sum = 0;
        for (auto a : arcs) sum += a.data();
        REQUIRE(sum == 60);
    }

    SECTION("Iteration over arcs") {
        DirectedAbstractGraph<int, int> g;
        auto v1 = g.addVertex(1);
        auto v2 = g.addVertex(2);
        auto v3 = g.addVertex(3);
        g.addArc(v1, v2, 100);
        g.addArc(v2, v3, 200);
        g.addArc(v3, v1, 300);

        std::vector<int> data;
        for (auto arc : g.arcs()) data.push_back(arc.data());

        std::sort(data.begin(), data.end());
        REQUIRE(data == std::vector<int>{100, 200, 300});
    }

}
*/
