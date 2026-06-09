#include <catch2/catch_test_macros.hpp>

#include "incident/directed/DirectedGraph.hpp"

#include <cstdlib>

using namespace exx::incident;

template<typename Graph>
void requireValidDirectedGraph(const Graph& g) {
    size_t outSum = 0;
    size_t inSum = 0;
    size_t arcCountFromIter = 0;

    for (auto v : g.vertices()) {
        outSum += v.outDegree();
        inSum  += v.inDegree();

        for (auto a : v.outgoingArcs()) {
            REQUIRE(a.from() == v);
            REQUIRE(g.hasArc(v, a.to()));
        }

        for (auto a : v.incomingArcs()) {
            REQUIRE(a.to() == v);
            REQUIRE(g.hasArc(a.from(), v));
        }
    }

    for (auto a : g.arcs()) {
        ++arcCountFromIter;
        REQUIRE(g.hasArc(a.from(), a.to()));
    }

    REQUIRE(arcCountFromIter == g.arcCount());
    REQUIRE(outSum == g.arcCount());
    REQUIRE(inSum == g.arcCount());
}

TEST_CASE("DirectedGraph unweighted",
          "[directed][graph][unweighted vertices][unweighted edges]")
{
    using Graph = DirectedGraph<void, void>;

    SECTION("Simple graph specific behavior") {
        Graph g;

        auto a = g.addVertex();
        auto b = g.addVertex();

        SECTION("No self-loops") {
            auto loop = g.addArc(a, a);
            REQUIRE(g.arcCount() == 0);
            REQUIRE(a.outDegree() == 0);
            REQUIRE(a.inDegree() == 0);
            REQUIRE(!loop.has_value());
            requireValidDirectedGraph(g);
        }

        SECTION("No multiple arcs") {
            g.addArc(a, b);
            auto second = g.addArc(a, b);
            REQUIRE(g.arcCount() == 1);
            REQUIRE(a.outDegree() == 1);
            REQUIRE(b.inDegree() == 1);
            REQUIRE(!second.has_value());
            requireValidDirectedGraph(g);
        }
    }
}
