#include <catch2/catch_test_macros.hpp>

#include "incident/undirected/UndirectedPseudoGraph.hpp"
#include "incident/utility/MultiIndexedGraphView.hpp"

using namespace exx::incident;
using Graph = UndirectedPseudoGraph<std::string, int>;

TEST_CASE("MultiIndexedGraphView", "[view][multi]") {
    Graph g;
    auto v1 = g.addVertex("X");
    auto v2 = g.addVertex("X"); // duplicate data
    auto v3 = g.addVertex("Y");

    MultiIndexedGraphView<Graph> view(g);

    SECTION("findVertices returns range of all matches") {
        auto range = view.findVertices("X");
        std::vector<typename Graph::ConstVertexDescriptor> descs(range.begin(), range.end());
        REQUIRE(descs.size() == 2);
        for (auto d : descs) REQUIRE(d.data() == "X");
    }

    SECTION("count returns correct multiplicity") {
        REQUIRE(view.count("X") == 2);
        REQUIRE(view.count("Y") == 1);
        REQUIRE(view.count("Z") == 0);
    }

    SECTION("containsVertex returns true if at least one exists") {
        REQUIRE(view.containsVertex("X"));
        REQUIRE(view.containsVertex("Y"));
        REQUIRE_FALSE(view.containsVertex("Z"));
    }

    SECTION("rebuild after data modification through graph") {
        v1.data() = "X1";
        REQUIRE(view.count("X") == 2);
        view.rebuild();
        REQUIRE(view.count("X") == 1);
        REQUIRE(view.count("X1") == 1);
    }
}
