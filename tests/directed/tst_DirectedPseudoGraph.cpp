#include <catch2/catch_test_macros.hpp>

#include "incident/directed/DirectedPseudoGraph.hpp"

#include <string>

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

TEST_CASE("DirectedPseudoGraph weighted vertices, unweighted arcs",
          "[directed][pseudograph][weighted vertices][unweighted edges]")
{
    using Graph = DirectedPseudoGraph<std::string, void>;

    SECTION("Empty graph state") {
        Graph g;
        REQUIRE(g.empty());
        REQUIRE(g.vertexCount() == 0);
        REQUIRE(g.arcCount() == 0);
        REQUIRE(g.beginVertices() == g.endVertices());
        REQUIRE(g.beginArcs() == g.endArcs());
        requireValidDirectedGraph(g);
    }

    SECTION("Basic graph operations") {
        Graph g;
        auto a = g.addVertex("A");
        auto b = g.addVertex("B");
        auto c = g.emplaceVertex("C");

        REQUIRE(g.vertexCount() == 3);

        auto ab = g.addArc(a, b);
        g.addArc(a, c);

        REQUIRE(g.arcCount() == 2);

        REQUIRE(a.outDegree() == 2);
        REQUIRE(a.inDegree()  == 0);
        REQUIRE(b.outDegree() == 0);
        REQUIRE(b.inDegree()  == 1);
        REQUIRE(c.outDegree() == 0);
        REQUIRE(c.inDegree()  == 1);

        REQUIRE(g.hasArc(a, b));
        REQUIRE_FALSE(g.hasArc(b, a));
        REQUIRE_FALSE(g.hasArc(b, c));

        requireValidDirectedGraph(g);

        g.removeArc(ab);

        REQUIRE(g.arcCount() == 1);
        REQUIRE(a.outDegree() == 1);
        REQUIRE(b.inDegree()  == 0);

        requireValidDirectedGraph(g);

        g.removeVertex(a);

        REQUIRE(g.vertexCount() == 2);
        REQUIRE(g.arcCount() == 0);
        requireValidDirectedGraph(g);
    }

    SECTION("Pseudo graph specific behavior (loops, multiarcs)") {
        Graph g;
        auto a = g.addVertex("A");
        auto b = g.addVertex("B");

        SECTION("Self-loops") {
            auto loop = g.addArc(a, a);
            REQUIRE(g.arcCount() == 1);
            REQUIRE(a.outDegree() == 1);
            REQUIRE(a.inDegree()  == 1);
            REQUIRE(g.hasArc(a, a));
            requireValidDirectedGraph(g);

            g.removeArc(loop);
            REQUIRE(g.arcCount() == 0);
            REQUIRE(a.outDegree() == 0);
            REQUIRE(a.inDegree()  == 0);
            requireValidDirectedGraph(g);
        }

        SECTION("Multiple arcs (parallel arcs)") {
            auto a1 = g.addArc(a, b);
            auto a2 = g.addArc(a, b);
            REQUIRE(g.arcCount() == 2);
            REQUIRE(a.outDegree() == 2);
            REQUIRE(b.inDegree()  == 2);
            REQUIRE(g.hasArc(a, b));
            requireValidDirectedGraph(g);

            g.removeArc(a1);
            REQUIRE(g.arcCount() == 1);
            REQUIRE(a.outDegree() == 1);
            REQUIRE(b.inDegree()  == 1);
            requireValidDirectedGraph(g);

            g.removeArc(a2);
            REQUIRE(g.arcCount() == 0);
            requireValidDirectedGraph(g);
        }

        SECTION("Loops and multiarcs together") {
            g.addArc(a, a);
            g.addArc(a, b);
            g.addArc(a, b);
            REQUIRE(g.arcCount() == 3);
            REQUIRE(a.outDegree() == 3);
            REQUIRE(a.inDegree()  == 1);
            REQUIRE(b.outDegree() == 0);
            REQUIRE(b.inDegree()  == 2);
            requireValidDirectedGraph(g);
        }
    }

    SECTION("Iterators traverse all elements") {
        Graph g;
        auto a = g.addVertex("A");
        auto b = g.addVertex("B");
        auto c = g.addVertex("C");
        g.addArc(a, b);
        g.addArc(a, b);
        g.addArc(a, a);
        g.addArc(b, c);

        size_t vcount = 0;
        for (auto v : g.vertices()) ++vcount;
        size_t acount = 0;
        for (auto a : g.arcs()) ++acount;

        REQUIRE(vcount == g.vertexCount());
        REQUIRE(acount == g.arcCount());
        requireValidDirectedGraph(g);
    }

    SECTION("Copy and move semantics") {
        Graph g;
        auto a = g.addVertex("A");
        auto b = g.addVertex("B");
        g.addArc(a, b);
        g.addArc(a, a);

        SECTION("Copy constructor") {
            Graph copy(g);
            REQUIRE(copy.vertexCount() == 2);
            REQUIRE(copy.arcCount() == 2);

            bool hasA = false, hasB = false;
            for (auto v : copy.vertices()) {
                if (v.data() == "A") hasA = true;
                if (v.data() == "B") hasB = true;
            }
            REQUIRE(hasA);
            REQUIRE(hasB);
            requireValidDirectedGraph(copy);
        }

        SECTION("Move constructor") {
            Graph moved(std::move(g));
            REQUIRE(moved.vertexCount() == 2);
            REQUIRE(moved.arcCount() == 2);
            requireValidDirectedGraph(moved);
            REQUIRE(g.vertexCount() == 0);
            REQUIRE(g.arcCount() == 0);
        }

        SECTION("Clear") {
            g.clear();
            REQUIRE(g.empty());
            REQUIRE(g.vertexCount() == 0);
            REQUIRE(g.arcCount() == 0);
            requireValidDirectedGraph(g);
        }
    }

    SECTION("Const correctness") {
        Graph g;
        auto a = g.addVertex("Alpha");
        auto b = g.addVertex("Beta");
        g.addArc(a, b);
        g.addArc(a, a);

        const Graph& cg = g;
        REQUIRE(cg.vertexCount() == 2);
        REQUIRE(cg.arcCount() == 2);

        Graph::ConstVertexDescriptor ca, cb;
        for (auto v : cg.vertices()) if (v.data() == "Alpha") ca = v;
        for (auto v : cg.vertices()) if (v.data() == "Beta")  cb = v;

        REQUIRE(cg.hasArc(ca, cb));
        REQUIRE_FALSE(cg.hasArc(cb, ca));
        requireValidDirectedGraph(cg);
    }

    SECTION("rotateArc works") {
        Graph g;
        auto a = g.addVertex("A");
        auto b = g.addVertex("B");
        auto arc = g.addArc(a, b);

        REQUIRE(g.rotateArc(arc));
        REQUIRE(arc.from() == b);
        REQUIRE(arc.to() == a);
        REQUIRE(g.hasArc(b, a));
        REQUIRE_FALSE(g.hasArc(a, b));

        auto loop = g.addArc(a, a);
        REQUIRE_FALSE(g.rotateArc(loop));
        REQUIRE(loop.from() == a);
        REQUIRE(loop.to() == a);
    }
}

TEST_CASE("DirectedPseudoGraph weighted vertices, weighted arcs",
          "[directed][pseudograph][weighted vertices][weighted edges]")
{
    using Graph = DirectedPseudoGraph<int, double>;

    SECTION("Weighted arc data") {
        Graph g;
        auto a = g.addVertex(1);
        auto b = g.addVertex(2);
        auto e = g.addArc(a, b, 3.14);
        REQUIRE(e.data() == 3.14);
        e.data() = 2.71;
        REQUIRE(e.data() == 2.71);
        requireValidDirectedGraph(g);
    }

    SECTION("Weighted multiarcs") {
        Graph g;
        auto a = g.addVertex(10);
        auto b = g.addVertex(20);
        auto e1 = g.addArc(a, b, 1.0);
        auto e2 = g.addArc(a, b, 2.0);
        REQUIRE(g.arcCount() == 2);
        REQUIRE(e1.data() == 1.0);
        REQUIRE(e2.data() == 2.0);
        requireValidDirectedGraph(g);
    }

    SECTION("Weighted loops") {
        Graph g;
        auto a = g.addVertex(42);
        auto loop = g.addArc(a, a, 9.9);
        REQUIRE(loop.data() == 9.9);
        REQUIRE(a.outDegree() == 1);
        REQUIRE(a.inDegree()  == 1);
        requireValidDirectedGraph(g);
    }

    SECTION("Weighted copy and move") {
        Graph g;
        auto a = g.addVertex(1);
        auto b = g.addVertex(2);
        g.addArc(a, b, 5.5);
        g.addArc(a, a, 7.7);

        SECTION("Copy") {
            Graph copy = g;
            REQUIRE(copy.vertexCount() == 2);
            REQUIRE(copy.arcCount() == 2);
            requireValidDirectedGraph(copy);
        }

        SECTION("Move") {
            Graph moved = std::move(g);
            REQUIRE(moved.vertexCount() == 2);
            REQUIRE(moved.arcCount() == 2);
            requireValidDirectedGraph(moved);
        }
    }

    SECTION("rotateArc preserves data") {
        Graph g;
        auto a = g.addVertex(1);
        auto b = g.addVertex(2);
        auto arc = g.addArc(a, b, 99.9);
        g.rotateArc(arc);
        REQUIRE(arc.data() == 99.9);
        REQUIRE(arc.from().data() == 2);
        REQUIRE(arc.to().data() == 1);
    }
}

TEST_CASE("DirectedPseudoGraph unweighted vertices, unweighted arcs",
          "[directed][pseudograph][unweighted vertices][unweighted edges]")
{
    using Graph = DirectedPseudoGraph<void, void>;

    SECTION("Empty graph") {
        Graph g;
        REQUIRE(g.empty());
        REQUIRE(g.vertexCount() == 0);
        REQUIRE(g.arcCount() == 0);
        requireValidDirectedGraph(g);
    }

    SECTION("Add vertices and arcs") {
        Graph g;
        auto a = g.addVertex();
        auto b = g.addVertex();
        auto c = g.emplaceVertex();

        REQUIRE(g.vertexCount() == 3);
        REQUIRE(a.outDegree() == 0);
        REQUIRE(a.inDegree() == 0);

        auto ab = g.addArc(a, b);
        g.addArc(a, c);
        REQUIRE(g.arcCount() == 2);

        REQUIRE(a.outDegree() == 2);
        REQUIRE(a.inDegree()  == 0);
        REQUIRE(b.outDegree() == 0);
        REQUIRE(b.inDegree()  == 1);
        REQUIRE(c.outDegree() == 0);
        REQUIRE(c.inDegree()  == 1);

        REQUIRE(g.hasArc(a, b));
        REQUIRE_FALSE(g.hasArc(b, a));
        REQUIRE_FALSE(g.hasArc(b, c));

        requireValidDirectedGraph(g);

        g.removeArc(ab);
        REQUIRE(g.arcCount() == 1);
        REQUIRE(a.outDegree() == 1);
        REQUIRE(b.inDegree()  == 0);
        requireValidDirectedGraph(g);

        g.removeVertex(a);
        REQUIRE(g.vertexCount() == 2);
        REQUIRE(g.arcCount() == 0);
        requireValidDirectedGraph(g);
    }

    SECTION("Self-loops and multiarcs") {
        Graph g;
        auto a = g.addVertex();
        auto b = g.addVertex();

        SECTION("Self-loop") {
            auto loop = g.addArc(a, a);
            REQUIRE(g.arcCount() == 1);
            REQUIRE(a.outDegree() == 1);
            REQUIRE(a.inDegree() == 1);
            REQUIRE(g.hasArc(a, a));
            requireValidDirectedGraph(g);
            g.removeArc(loop);
            REQUIRE(g.arcCount() == 0);
        }

        SECTION("Multiple arcs") {
            g.addArc(a, b);
            g.addArc(a, b);
            REQUIRE(g.arcCount() == 2);
            REQUIRE(a.outDegree() == 2);
            REQUIRE(b.inDegree() == 2);
            requireValidDirectedGraph(g);
        }

        SECTION("Combined") {
            g.addArc(a, a);
            g.addArc(a, b);
            g.addArc(a, b);
            REQUIRE(g.arcCount() == 3);
            REQUIRE(a.outDegree() == 3);
            REQUIRE(a.inDegree() == 1);
            REQUIRE(b.inDegree() == 2);
            requireValidDirectedGraph(g);
        }
    }

    SECTION("Iterators") {
        Graph g;
        auto a = g.addVertex();
        auto b = g.addVertex();
        auto c = g.addVertex();
        g.addArc(a, b);
        g.addArc(a, b);
        g.addArc(a, a);
        g.addArc(b, c);

        size_t vcnt = 0, acnt = 0;
        for (auto v : g.vertices()) ++vcnt;
        for (auto a : g.arcs()) ++acnt;
        REQUIRE(vcnt == g.vertexCount());
        REQUIRE(acnt == g.arcCount());
        requireValidDirectedGraph(g);
    }

    SECTION("Copy and move") {
        Graph g;
        auto a = g.addVertex();
        auto b = g.addVertex();
        g.addArc(a, b);
        g.addArc(a, a);

        Graph copy(g);
        REQUIRE(copy.vertexCount() == 2);
        REQUIRE(copy.arcCount() == 2);
        requireValidDirectedGraph(copy);

        Graph moved(std::move(g));
        REQUIRE(moved.vertexCount() == 2);
        REQUIRE(moved.arcCount() == 2);
        requireValidDirectedGraph(moved);
        REQUIRE(g.vertexCount() == 0);
    }

    SECTION("Const correctness") {
        Graph g;
        auto a = g.addVertex();
        auto b = g.addVertex();
        g.addArc(a, b);
        g.addArc(a, a);

        const Graph& cg = g;
        REQUIRE(cg.vertexCount() == 2);
        REQUIRE(cg.arcCount() == 2);

        auto it = cg.vertices().begin();
        auto ca = *it; ++it;
        auto cb = *it;
        REQUIRE(cg.hasArc(ca, cb));
        REQUIRE_FALSE(cg.hasArc(cb, ca));
        requireValidDirectedGraph(cg);
    }

    SECTION("rotateArc works") {
        Graph g;
        auto a = g.addVertex();
        auto b = g.addVertex();
        auto arc = g.addArc(a, b);

        REQUIRE(g.rotateArc(arc));
        REQUIRE(arc.from() == b);
        REQUIRE(arc.to() == a);
        REQUIRE(g.hasArc(b, a));
        REQUIRE_FALSE(g.hasArc(a, b));
    }
}

TEST_CASE("DirectedPseudoGraph unweighted vertices, weighted arcs",
          "[directed][pseudograph][unweighted vertices][weighted edges]")
{
    using Graph = DirectedPseudoGraph<void, int>;

    SECTION("Arc data") {
        Graph g;
        auto a = g.addVertex();
        auto b = g.addVertex();
        auto e = g.addArc(a, b, 42);
        REQUIRE(e.data() == 42);
        e.data() = 100;
        REQUIRE(e.data() == 100);
        requireValidDirectedGraph(g);
    }

    SECTION("Multiarcs with different data") {
        Graph g;
        auto a = g.addVertex();
        auto b = g.addVertex();
        auto e1 = g.addArc(a, b, 10);
        auto e2 = g.addArc(a, b, 20);
        REQUIRE(g.arcCount() == 2);
        REQUIRE(e1.data() == 10);
        REQUIRE(e2.data() == 20);
        requireValidDirectedGraph(g);
    }

    SECTION("Self-loop with data") {
        Graph g;
        auto a = g.addVertex();
        auto loop = g.addArc(a, a, 999);
        REQUIRE(loop.data() == 999);
        REQUIRE(a.outDegree() == 1);
        REQUIRE(a.inDegree() == 1);
        requireValidDirectedGraph(g);
    }

    SECTION("Data modification after copy") {
        Graph g;
        auto a = g.addVertex();
        auto b = g.addVertex();
        auto e = g.addArc(a, b, 5);

        Graph copy = g;
        auto it = copy.vertices().begin();
        auto ca = *it; ++it;
        auto cb = *it;
        auto e_copy = *copy.findArc(ca, cb);
        REQUIRE(e_copy.data() == 5);
        e_copy.data() = 7;
        REQUIRE(e_copy.data() == 7);
        REQUIRE(e.data() == 5);
        requireValidDirectedGraph(copy);
        requireValidDirectedGraph(g);
    }

    SECTION("Const access to arc data") {
        Graph g;
        auto a = g.addVertex();
        auto b = g.addVertex();
        g.addArc(a, b, 123);

        const Graph& cg = g;
        auto it = cg.vertices().begin();
        auto ca = *it; ++it;
        auto cb = *it;
        auto ce = cg.findArc(ca, cb);
        REQUIRE(ce.has_value());
        REQUIRE(ce->data() == 123);
        requireValidDirectedGraph(cg);
    }

    SECTION("rotateArc preserves data") {
        Graph g;
        auto a = g.addVertex();
        auto b = g.addVertex();
        auto arc = g.addArc(a, b, 77);
        g.rotateArc(arc);
        REQUIRE(arc.data() == 77);
        REQUIRE(arc.from() == b);
        REQUIRE(arc.to() == a);
    }
}
