#include <catch2/catch_test_macros.hpp>

#include "incident/directed/DirectedMultiGraph.hpp"

#include <string>
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

TEST_CASE("DirectedMultiGraph unweighted",
          "[directed][multigraph][unweighted]")
{
    using Graph = DirectedMultiGraph<std::string, void>;

    SECTION("Multi graph specific befavior") {
        Graph g;

        auto a = g.addVertex("A");
        auto b = g.addVertex("B");
        SECTION("No self-loops") {
            auto loop = g.addArc(a, a);

            REQUIRE(g.arcCount() == 0);
            REQUIRE(a.outDegree() == 0);
            REQUIRE(!loop.has_value());

            requireValidDirectedGraph(g);
        }

        SECTION("Multiple edges") {
            auto e1 = g.addArc(a, b);
            auto e2 = g.addArc(a, b);

            REQUIRE(g.arcCount() == 2);

            REQUIRE(a.outDegree() == 2);
            REQUIRE(b.inDegree() == 2);
            REQUIRE(g.hasArc(a, b));

            requireValidDirectedGraph(g);

            g.removeArc(*e1);

            REQUIRE(g.arcCount() == 1);
            REQUIRE(g.hasArc(a, b));

            requireValidDirectedGraph(g);

            g.removeArc(*e2);

            REQUIRE(g.arcCount() == 0);

            requireValidDirectedGraph(g);
        }
    }
}

