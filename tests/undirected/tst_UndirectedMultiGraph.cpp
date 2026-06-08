#include <catch2/catch_test_macros.hpp>

#include "incident/undirected/UndirectedMultiGraph.hpp"

#include <string>
#include <cstdlib>

using namespace exx::incident;

template<typename Graph>
void requireValidUndirectedGraph(const Graph& g){
    size_t degreeSum = 0;
    size_t iteratedEdgeCount = 0;

    for (auto w : g.vertices()) {
        degreeSum += w.degree();

        for (auto e : w.incidentEdges()) {
            auto u = e.u();
            auto v = e.v();

            REQUIRE(g.hasEdge(u, v));
            REQUIRE(g.hasEdge(v, u));
        }
    }

    for (auto e : g.edges()) {
        ++iteratedEdgeCount;

        REQUIRE(g.hasEdge(e.u(), e.v()));
        REQUIRE(g.hasEdge(e.v(), e.u()));
    }

    REQUIRE(iteratedEdgeCount == g.edgeCount());
    REQUIRE(degreeSum == g.edgeCount() * 2);
}

TEST_CASE("UndirectedMultiGraph unweighted",
          "[undirected][multigraph][unweighted]")
{
    using Graph = UndirectedMultiGraph<std::string, void>;

    SECTION("Multi graph specific befavior") {
        Graph g;

        auto a = g.addVertex("A");
        auto b = g.addVertex("B");
        SECTION("No self-loops") {
            auto loop = g.addEdge(a, a);

            REQUIRE(g.edgeCount() == 0);
            REQUIRE(a.degree() == 0);
            REQUIRE(!loop.has_value());

            requireValidUndirectedGraph(g);
        }

        SECTION("Multiple edges") {
            auto e1 = g.addEdge(a, b);
            auto e2 = g.addEdge(a, b);

            REQUIRE(g.edgeCount() == 2);

            REQUIRE(a.degree() == 2);
            REQUIRE(b.degree() == 2);
            REQUIRE(g.hasEdge(a, b));

            requireValidUndirectedGraph(g);

            g.removeEdge(*e1);

            REQUIRE(g.edgeCount() == 1);
            REQUIRE(g.hasEdge(a, b));

            requireValidUndirectedGraph(g);

            g.removeEdge(*e2);

            REQUIRE(g.edgeCount() == 0);

            requireValidUndirectedGraph(g);
        }
    }
}

