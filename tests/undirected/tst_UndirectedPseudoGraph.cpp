#include <catch2/catch_test_macros.hpp>

#include "incident/undirected/UndirectedPseudoGraph.hpp"

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

TEST_CASE("UndirectedPseudoGraph unweighted",
          "[undirected][pseudograph][unweighted]")
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
            REQUIRE(copy.hasVertex("A"));
            REQUIRE(copy.hasVertex("B"));

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

    SECTION("Const correctness") {
        Graph g;

        auto a = g.addVertex("Alpha");
        auto b = g.addVertex("Beta");

        g.addEdge(a, b);
        g.addEdge(a, a);

        const Graph& cg = g;

        REQUIRE(cg.vertexCount() == 2);
        REQUIRE(cg.edgeCount() == 2);
        REQUIRE(cg.hasVertex("Alpha"));
        REQUIRE(cg.hasVertex("Beta"));

        Graph::ConstVertexDescriptor ca, cb;
        for(auto v : cg.vertices()) if (v.data() == "Alpha") ca = v;
        for(auto v : cg.vertices()) if (v.data() == "Beta")  cb = v;

        REQUIRE(cg.hasEdge(ca, cb));

        requireValidUndirectedGraph(cg);
    }
}

TEST_CASE("UndirectedPseudoGraph weighted",
          "[undirected][pseudograph][weighted]")
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

TEST_CASE("UndirectedPseudoGraph descriptors",
          "[undirecteds][pseudograph][descriptors]")
{
    using Graph = UndirectedPseudoGraph<std::string, int>;
    using VertexDesc = Graph::VertexDescriptor;
    using ConstVertexDesc = Graph::ConstVertexDescriptor;
    using EdgeDesc = Graph::EdgeDescriptor;
    using ConstEdgeDesc = Graph::ConstEdgeDescriptor;

    Graph g;

    auto vA = g.addVertex("A");
    auto vB = g.addVertex("B");
    auto vC = g.addVertex("C");
    auto vD = g.addVertex("D");

    // edges: A-B (10), A-B (20) – multiple, A-A (30) – loop, B-C (40), C-D (50)
    auto eAB1 = g.addEdge(vA, vB, 10);
    auto eAB2 = g.addEdge(vA, vB, 20);
    auto eLoop = g.addEdge(vA, vA, 30);
    auto eBC  = g.addEdge(vB, vC, 40);
    auto eCD  = g.addEdge(vC, vD, 50);

    requireValidUndirectedGraph(g);

    SECTION("VertexDescriptor basics") {
        REQUIRE(vA.degree() == 4);
        REQUIRE(vB.degree() == 3);
        REQUIRE(vC.degree() == 2);
        REQUIRE(vD.degree() == 1);

        REQUIRE(vA.data() == "A");
        vA.data() = "A1";
        REQUIRE(vA.data() == "A1");
        vA.data() = "A";

        REQUIRE(vA.incidentEdges().size() == 4);

        bool hasLoop = false, hasAB1 = false, hasAB2 = false;
        for (auto e : vA.incidentEdges()) {
            if (e.u() == vA && e.v() == vA) hasLoop = true;

            if ((e.u() == vA && e.v() == vB) || (e.u() == vB && e.v() == vA)) {
                if (e.data() == 10) hasAB1 = true;
                if (e.data() == 20) hasAB2 = true;
            }
        }
        REQUIRE(hasLoop);
        REQUIRE(hasAB1);
        REQUIRE(hasAB2);
\
        auto adjA = vA.adjacentVertices();
        REQUIRE(adjA.size() == 2);
        REQUIRE(adjA[0].data() == "A");
        REQUIRE(adjA[1].data() == "B");

        auto unordAdjA = vA.unorderedAdjV();
        REQUIRE(unordAdjA.size() == 2);

        std::unordered_set<std::string> names;
        for (auto v : unordAdjA) names.insert(v.data());
        REQUIRE(names.contains("A"));
        REQUIRE(names.contains("B"));

        auto adjADesc = vA.adjacentVertices<std::greater<std::string>>();
        REQUIRE(adjADesc.size() == 2);
        REQUIRE(adjADesc[0].data() == "B");
        REQUIRE(adjADesc[1].data() == "A");

        auto adjB = vB.adjacentVertices();
        REQUIRE(adjB.size() == 2);
        REQUIRE(adjB[0].data() == "A");
        REQUIRE(adjB[1].data() == "C");

        auto adjD = vD.adjacentVertices();
        REQUIRE(adjD.size() == 1);
        REQUIRE(adjD[0].data() == "C");
    }

    SECTION("EdgeDescriptor basics") {
        REQUIRE(eAB1.data() == 10);
        eAB1.data() = 100;
        REQUIRE(eAB1.data() == 100);
        eAB1.data() = 10;


        auto AB1containsValidLinks = (eAB1.v() == vA && eAB1.u() == vB) ||
                                     (eAB1.u() == vA && eAB1.v() == vB);
        REQUIRE(AB1containsValidLinks);
        REQUIRE(eLoop.u() == vA);
        REQUIRE(eLoop.v() == vA);

        auto other = eAB1.otherEnd(vA);
        REQUIRE(other.has_value());
        REQUIRE(*other == vB);
        other = eAB1.otherEnd(vB);
        REQUIRE(other.has_value());
        REQUIRE(*other == vA);
        other = eAB1.otherEnd(vC);
        REQUIRE_FALSE(other.has_value());

        auto loopOther = eLoop.otherEnd(vA);
        REQUIRE(loopOther.has_value());
        REQUIRE(*loopOther == vA);

        EdgeDesc eAB1_copy = eAB1;
        REQUIRE(eAB1 == eAB1_copy);
        REQUIRE(eAB1 != eAB2);
    }

    SECTION("Const descriptors") {
        const Graph& cg = g;

        Graph::ConstVertexDescriptor ca, cb;
        for(auto v : cg.vertices()) if (v.data() == "A") ca = v;
        for(auto v : cg.vertices()) if (v.data() == "B") cb = v;
        auto ceAB = cg.findEdge(ca, cb);
        REQUIRE(ceAB.has_value());

        REQUIRE(ca.data() == "A");

        REQUIRE(ceAB->data() == 10);
        auto other = ceAB->otherEnd(ca);
        REQUIRE(other.has_value());
        REQUIRE(other->data() == "B");
    }

    SECTION("findEdge and hasEdge") {
        auto found = g.findEdge(vA, vB);
        REQUIRE(found.has_value());

        auto uv = found->u();
        auto vv = found->v();
        auto f = ((uv == vA && vv == vB) || (uv == vB && vv == vA));
        REQUIRE(f);

        auto found2 = g.findEdge(vA, vB);
        REQUIRE(found2.has_value());
        REQUIRE(g.hasEdge(vA, vB));
        REQUIRE(g.hasEdge(vB, vA));

        auto foundLoop = g.findEdge(vA, vA);
        REQUIRE(foundLoop.has_value());
        REQUIRE(foundLoop->data() == 30);

        auto notFound = g.findEdge(vB, vD);
        REQUIRE_FALSE(notFound.has_value());
        REQUIRE_FALSE(g.hasEdge(vB, vD));
    }

    SECTION("adjacentVertices handles multiedges and loops correctly") {
        Graph h;
        auto x = h.addVertex("X");
        auto y = h.addVertex("Y");

        h.addEdge(x, y, 1);
        h.addEdge(x, y, 2);
        h.addEdge(x, x, 3);

        auto adjX = x.adjacentVertices();
        REQUIRE(adjX.size() == 2);
        REQUIRE(adjX[0].data() == "X");
        REQUIRE(adjX[1].data() == "Y");

        auto adjY = y.adjacentVertices();
        REQUIRE(adjY.size() == 1);
        REQUIRE(adjY[0].data() == "X");
    }
}
