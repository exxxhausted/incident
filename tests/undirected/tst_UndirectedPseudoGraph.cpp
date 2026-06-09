#include <catch2/catch_test_macros.hpp>

#include "incident/undirected/UndirectedPseudoGraph.hpp"

#include <string>
#include <cstdlib>

using namespace exx::incident;

template<typename Graph>
void requireValidUndirectedGraph(const Graph& g) {
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

TEST_CASE("UndirectedPseudoGraph weighted vertices, unweighted edges",
          "[undirected][pseudograph][weighted vertices][unweighted edges]")
{
    using Graph = UndirectedPseudoGraph<std::string, void>;

    SECTION("Empty graph state") {
        Graph g;
        REQUIRE(g.empty());
        REQUIRE(g.vertexCount() == 0);
        REQUIRE(g.edgeCount() == 0);
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

    SECTION("Const correctness") {
        Graph g;
        auto a = g.addVertex("Alpha");
        auto b = g.addVertex("Beta");
        g.addEdge(a, b);
        g.addEdge(a, a);

        const Graph& cg = g;
        REQUIRE(cg.vertexCount() == 2);
        REQUIRE(cg.edgeCount() == 2);

        Graph::ConstVertexDescriptor ca, cb;
        for (auto v : cg.vertices()) if (v.data() == "Alpha") ca = v;
        for (auto v : cg.vertices()) if (v.data() == "Beta")  cb = v;
        REQUIRE(cg.hasEdge(ca, cb));
        requireValidUndirectedGraph(cg);
    }

    SECTION("Copy and move") {
        Graph g;
        auto a = g.addVertex("A");
        auto b = g.addVertex("B");
        g.addEdge(a, b);
        g.addEdge(a, a);

        Graph copy(g);
        REQUIRE(copy.vertexCount() == 2);
        REQUIRE(copy.edgeCount() == 2);
        requireValidUndirectedGraph(copy);

        Graph moved(std::move(g));
        REQUIRE(moved.vertexCount() == 2);
        REQUIRE(moved.edgeCount() == 2);
        requireValidUndirectedGraph(moved);
        REQUIRE(g.vertexCount() == 0);
    }
}

TEST_CASE("UndirectedPseudoGraph weighted vertices, weighted edges",
          "[undirected][pseudograph][weighted vertices][weighted edges]")
{
    using Graph = UndirectedPseudoGraph<int, double>;

    SECTION("Edge data") {
        Graph g;
        auto a = g.addVertex(1);
        auto b = g.addVertex(2);
        auto e = g.addEdge(a, b, 3.14);
        REQUIRE(e.data() == 3.14);
        e.data() = 2.71;
        REQUIRE(e.data() == 2.71);
        requireValidUndirectedGraph(g);
    }

    SECTION("Multiedges with data") {
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

    SECTION("Loops with data") {
        Graph g;
        auto a = g.addVertex(42);
        auto loop = g.addEdge(a, a, 9.9);
        REQUIRE(loop.data() == 9.9);
        REQUIRE(a.degree() == 2);
        requireValidUndirectedGraph(g);
    }
}

TEST_CASE("UndirectedPseudoGraph unweighted vertices, unweighted edges",
          "[undirected][pseudograph][unweighted vertices][unweighted edges]")
{
    using Graph = UndirectedPseudoGraph<void, void>;

    SECTION("Empty graph") {
        Graph g;
        REQUIRE(g.empty());
        REQUIRE(g.vertexCount() == 0);
        REQUIRE(g.edgeCount() == 0);
        requireValidUndirectedGraph(g);
    }

    SECTION("Add vertices and edges") {
        Graph g;
        auto a = g.addVertex();
        auto b = g.addVertex();
        auto c = g.emplaceVertex();

        REQUIRE(g.vertexCount() == 3);
        REQUIRE(a.degree() == 0);
        REQUIRE(b.degree() == 0);
        REQUIRE(c.degree() == 0);

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

    SECTION("Self-loops and multiedges") {
        Graph g;
        auto a = g.addVertex();
        auto b = g.addVertex();

        SECTION("Self-loop") {
            auto loop = g.addEdge(a, a);
            REQUIRE(g.edgeCount() == 1);
            REQUIRE(a.degree() == 2);
            REQUIRE(g.hasEdge(a, a));
            requireValidUndirectedGraph(g);
            g.removeEdge(loop);
            REQUIRE(g.edgeCount() == 0);
            REQUIRE(a.degree() == 0);
        }

        SECTION("Multiple edges") {
            g.addEdge(a, b);
            g.addEdge(a, b);
            REQUIRE(g.edgeCount() == 2);
            REQUIRE(a.degree() == 2);
            REQUIRE(b.degree() == 2);
            requireValidUndirectedGraph(g);
        }

        SECTION("Combined") {
            g.addEdge(a, a);
            g.addEdge(a, b);
            g.addEdge(a, b);
            REQUIRE(g.edgeCount() == 3);
            REQUIRE(a.degree() == 4);
            REQUIRE(b.degree() == 2);
            requireValidUndirectedGraph(g);
        }
    }

    SECTION("Iterators") {
        Graph g;
        auto a = g.addVertex();
        auto b = g.addVertex();
        auto c = g.addVertex();
        g.addEdge(a, b);
        g.addEdge(a, b);
        g.addEdge(a, a);
        g.addEdge(b, c);

        size_t vcnt = 0, ecnt = 0;
        for (auto v : g.vertices()) ++vcnt;
        for (auto e : g.edges()) ++ecnt;
        REQUIRE(vcnt == g.vertexCount());
        REQUIRE(ecnt == g.edgeCount());
        requireValidUndirectedGraph(g);
    }

    SECTION("Copy and move") {
        Graph g;
        auto a = g.addVertex();
        auto b = g.addVertex();
        g.addEdge(a, b);
        g.addEdge(a, a);

        Graph copy(g);
        REQUIRE(copy.vertexCount() == 2);
        REQUIRE(copy.edgeCount() == 2);
        requireValidUndirectedGraph(copy);

        Graph moved(std::move(g));
        REQUIRE(moved.vertexCount() == 2);
        REQUIRE(moved.edgeCount() == 2);
        requireValidUndirectedGraph(moved);
        REQUIRE(g.vertexCount() == 0);
    }

    SECTION("Const correctness") {
        Graph g;
        auto a = g.addVertex();
        auto b = g.addVertex();
        g.addEdge(a, b);
        g.addEdge(a, a);

        const Graph& cg = g;
        REQUIRE(cg.vertexCount() == 2);
        REQUIRE(cg.edgeCount() == 2);

        Graph::ConstVertexDescriptor ca, cb;

        auto it = cg.vertices().begin();
        ca = *it; ++it;
        cb = *it;

        REQUIRE(cg.hasEdge(ca, cb));
        requireValidUndirectedGraph(cg);
    }
}

TEST_CASE("UndirectedPseudoGraph unweighted vertices, weighted edges",
          "[undirected][pseudograph][unweighted vertices][weighted edges]")
{
    using Graph = UndirectedPseudoGraph<void, int>;

    SECTION("Edge data") {
        Graph g;
        auto a = g.addVertex();
        auto b = g.addVertex();
        auto e = g.addEdge(a, b, 42);
        REQUIRE(e.data() == 42);
        e.data() = 100;
        REQUIRE(e.data() == 100);
        requireValidUndirectedGraph(g);
    }

    SECTION("Multiedges with different data") {
        Graph g;
        auto a = g.addVertex();
        auto b = g.addVertex();
        auto e1 = g.addEdge(a, b, 10);
        auto e2 = g.addEdge(a, b, 20);
        REQUIRE(g.edgeCount() == 2);
        REQUIRE(e1.data() == 10);
        REQUIRE(e2.data() == 20);
        requireValidUndirectedGraph(g);
    }

    SECTION("Self-loop with data") {
        Graph g;
        auto a = g.addVertex();
        auto loop = g.addEdge(a, a, 999);
        REQUIRE(loop.data() == 999);
        REQUIRE(a.degree() == 2);
        requireValidUndirectedGraph(g);
    }

    SECTION("Data modification after copy") {
        Graph g;
        auto a = g.addVertex();
        auto b = g.addVertex();
        auto e = g.addEdge(a, b, 5);

        Graph copy = g;
        auto e_copy = *copy.findEdge(*copy.vertices().begin(), *(++copy.vertices().begin()));
        REQUIRE(e_copy.data() == 5);
        e_copy.data() = 7;
        REQUIRE(e_copy.data() == 7);

        REQUIRE(e.data() == 5);
        requireValidUndirectedGraph(copy);
        requireValidUndirectedGraph(g);
    }

    SECTION("Const access to edge data") {
        Graph g;
        auto a = g.addVertex();
        auto b = g.addVertex();
        g.addEdge(a, b, 123);

        const Graph& cg = g;
        auto ce = cg.findEdge(*cg.vertices().begin(), *(++cg.vertices().begin()));
        REQUIRE(ce.has_value());
        REQUIRE(ce->data() == 123);
        requireValidUndirectedGraph(cg);
    }
}
