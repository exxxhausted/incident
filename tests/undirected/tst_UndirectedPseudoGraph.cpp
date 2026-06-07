#include <catch2/catch_test_macros.hpp>

#include "incident/undirected/UndirectedPseudoGraph.hpp"
#include "incident/algorithms/basic_algorithms.hpp"

#include <string>
#include <vector>
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

TEST_CASE("UndirectedPseudoGraph unweighted",
          "[undirecteds][pseudograph][unweighted]")
{
    using Graph = UndirectedPseudoGraph<std::string, void>;

    SECTION("Empty graph state") {
        Graph g;

        REQUIRE(g.empty());
        REQUIRE(g.vertexCount() == 0);
        REQUIRE(g.edgeCount() == 0);
        REQUIRE(g.beginVertices() == g.endVertices());
        REQUIRE(g.beginEdges() == g.endEdges());

        requireValidUndirectedGraph(g);
    }

    SECTION("Basic graph operations") {
        Graph g;

        auto a = g.addVertex("A");
        auto b = g.addVertex("B");
        auto c = g.emplaceVertex("C");

        REQUIRE(g.vertexCount() == 3);

        auto ab = g.addEdge(a, b);
        g.addEdge(a, c);

        REQUIRE(g.edgeCount() == 2);

        REQUIRE(a.degree() == 2);
        REQUIRE(b.degree() == 1);
        REQUIRE(c.degree() == 1);
        REQUIRE(g.hasEdge(a, b));
        REQUIRE(g.hasEdge(b, a));
        REQUIRE_FALSE(g.hasEdge(b, c));

        requireValidUndirectedGraph(g);

        g.removeEdge(ab);

        REQUIRE(g.edgeCount() == 1);
        REQUIRE(a.degree() == 1);
        REQUIRE(b.degree() == 0);

        requireValidUndirectedGraph(g);

        g.removeVertex(a);

        REQUIRE(g.vertexCount() == 2);
        REQUIRE(g.edgeCount() == 0);

        requireValidUndirectedGraph(g);
    }

    SECTION("Pseudo graph specific behavior") {
        Graph g;

        auto a = g.addVertex("A");
        auto b = g.addVertex("B");

        SECTION("Self-loops") {
            auto loop = g.addEdge(a, a);

            REQUIRE(g.edgeCount() == 1);
            REQUIRE(a.degree() == 2);
            REQUIRE(g.hasEdge(a, a));

            requireValidUndirectedGraph(g);

            g.removeEdge(loop);

            REQUIRE(g.edgeCount() == 0);
            REQUIRE(a.degree() == 0);

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

            g.removeEdge(e1);

            REQUIRE(g.edgeCount() == 1);
            REQUIRE(g.hasEdge(a, b));

            requireValidUndirectedGraph(g);

            g.removeEdge(e2);

            REQUIRE(g.edgeCount() == 0);

            requireValidUndirectedGraph(g);
        }

        SECTION("Loops and multiedges together") {
            g.addEdge(a, a);
            g.addEdge(a, b);
            g.addEdge(a, b);

            REQUIRE(g.edgeCount() == 3);

            REQUIRE(a.degree() == 4);
            REQUIRE(b.degree() == 2);

            requireValidUndirectedGraph(g);
        }
    }

    SECTION("Iterators traverse all elements") {
        Graph g;

        auto a = g.addVertex("A");
        auto b = g.addVertex("B");
        auto c = g.addVertex("C");

        g.addEdge(a, b);
        g.addEdge(a, b);
        g.addEdge(a, a);
        g.addEdge(b, c);

        size_t vertexCount = 0;
        for (auto v : g.vertices()) ++vertexCount;

        size_t edgeCount = 0;
        for (auto e : g.edges()) ++edgeCount;

        REQUIRE(vertexCount == g.vertexCount());
        REQUIRE(edgeCount == g.edgeCount());

        requireValidUndirectedGraph(g);
    }

    SECTION("Copy and move semantics") {
        Graph g;

        auto a = g.addVertex("A");
        auto b = g.addVertex("B");

        g.addEdge(a, b);
        g.addEdge(a, a);

        SECTION("Copy constructor") {
            Graph copy(g);

            REQUIRE(copy.vertexCount() == 2);
            REQUIRE(copy.edgeCount() == 2);
            REQUIRE(containsVertex(copy, "A"));
            REQUIRE(containsVertex(copy, "B"));

            requireValidUndirectedGraph(copy);
        }

        SECTION("Move constructor") {
            Graph moved(std::move(g));

            REQUIRE(moved.vertexCount() == 2);
            REQUIRE(moved.edgeCount() == 2);

            requireValidUndirectedGraph(moved);

            REQUIRE(g.vertexCount() == 0);
            REQUIRE(g.edgeCount() == 0);
        }

        SECTION("Clear") {
            g.clear();

            REQUIRE(g.empty());
            REQUIRE(g.vertexCount() == 0);
            REQUIRE(g.edgeCount() == 0);

            requireValidUndirectedGraph(g);
        }
    }

    SECTION("Helper algorithms") {
        Graph g;

        g.addVertex("100");
        g.addVertex("200");
        g.addVertex("300");

        REQUIRE(containsVertex(g, "100"));
        REQUIRE(containsVertex(g, "200"));
        REQUIRE_FALSE(containsVertex(g, "999"));

        auto found = findVertex(g, "200");
        REQUIRE(found.has_value());
        REQUIRE(found->data() == "200");

        auto notFound = findVertex(g, "999");
        REQUIRE_FALSE(notFound.has_value());
    }

    SECTION("Const correctness") {
        Graph g;

        auto a = g.addVertex("Alpha");
        auto b = g.addVertex("Beta");

        g.addEdge(a, b);
        g.addEdge(a, a);

        const Graph& cg = g;

        REQUIRE(cg.vertexCount() == 2);
        REQUIRE(cg.edgeCount() == 2);
        REQUIRE(containsVertex(cg, "Alpha"));
        REQUIRE(containsVertex(cg, "Beta"));
        REQUIRE(cg.hasEdge(
            *findVertex(cg, "Alpha"),
            *findVertex(cg, "Beta")
            ));

        requireValidUndirectedGraph(cg);
    }
}

TEST_CASE("UndirectedPseudoGraph weighted",
          "[undirecteds][pseudograph][weighted]")
{
    using Graph = UndirectedPseudoGraph<int, double>;

    SECTION("Weighted edge data") {
        Graph g;

        auto a = g.addVertex(1);
        auto b = g.addVertex(2);

        auto e = g.addEdge(a, b, 3.14);

        REQUIRE(e.data() == 3.14);

        e.data() = 2.71;

        REQUIRE(e.data() == 2.71);

        requireValidUndirectedGraph(g);
    }

    SECTION("Weighted multiedges") {
        Graph g;

        auto a = g.addVertex(10);
        auto b = g.addVertex(20);

        auto e1 = g.addEdge(a, b, 1.0);
        auto e2 = g.addEdge(a, b, 2.0);

        REQUIRE(g.edgeCount() == 2);

        REQUIRE(e1.data() == 1.0);
        REQUIRE(e2.data() == 2.0);

        requireValidUndirectedGraph(g);
    }

    SECTION("Weighted loops") {
        Graph g;

        auto a = g.addVertex(42);

        auto loop = g.addEdge(a, a, 9.9);

        REQUIRE(loop.data() == 9.9);
        REQUIRE(a.degree() == 2);

        requireValidUndirectedGraph(g);
    }

    SECTION("Weighted copy and move") {
        Graph g;

        auto a = g.addVertex(1);
        auto b = g.addVertex(2);

        g.addEdge(a, b, 5.5);
        g.addEdge(a, a, 7.7);

        SECTION("Copy") {
            Graph copy = g;

            REQUIRE(copy.vertexCount() == 2);
            REQUIRE(copy.edgeCount() == 2);

            requireValidUndirectedGraph(copy);
        }

        SECTION("Move") {
            Graph moved = std::move(g);

            REQUIRE(moved.vertexCount() == 2);
            REQUIRE(moved.edgeCount() == 2);

            requireValidUndirectedGraph(moved);
        }
    }
}
