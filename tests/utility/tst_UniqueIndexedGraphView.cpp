#include <catch2/catch_test_macros.hpp>

#include "incident/undirected/UndirectedPseudoGraph.hpp"
#include "incident/utility/UniqueVertexIndexedView.hpp"

using namespace exx::incident;
using Graph = UndirectedPseudoGraph<std::string, int>;

TEST_CASE("UniqueVertexIndexedView", "[view][unique]") {
    Graph g;
    auto vA = g.addVertex("A");
    auto vB = g.addVertex("B");
    auto vC = g.addVertex("C");

    UniqueVertexIndexedView<Graph> view(g);

    SECTION("findVertex returns correct descriptor") {
        auto found = view.findVertex("B");
        REQUIRE(found.has_value());
        REQUIRE(found->data() == "B");

        auto notFound = view.findVertex("X");
        REQUIRE_FALSE(notFound.has_value());
    }

    SECTION("containsVertex") {
        REQUIRE(view.containsVertex("A"));
        REQUIRE_FALSE(view.containsVertex("Z"));
    }

    SECTION("rebuild after deletion") {
        g.removeVertex(vB);
        REQUIRE(view.containsVertex("B"));
        view.rebuild();
        REQUIRE_FALSE(view.containsVertex("B"));
    }

    SECTION("iteration over keys") {
        std::vector<std::string> keys;
        for (auto& [key, desc] : view) keys.push_back(key);
        std::ranges::sort(keys);
        REQUIRE(keys == std::vector<std::string>({"A","B","C"}));
    }
}

