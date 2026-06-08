#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>

#include "incident/directed/DirectedPseudoGraph.hpp"

#include <string>
#include <unordered_set>

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

TEST_CASE("DirectedPseudoGraph unweighted", "[directed][pseudograph][unweighted]") {
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

        auto ab = g.addArc(a, b);   // A -> B
        g.addArc(a, c);             // A -> C

        REQUIRE(g.arcCount() == 2);

        REQUIRE(a.outDegree() == 2);
        REQUIRE(a.inDegree()  == 0);
        REQUIRE(b.outDegree() == 0);
        REQUIRE(b.inDegree()  == 1);
        REQUIRE(c.outDegree() == 0);
        REQUIRE(c.inDegree()  == 1);

        REQUIRE(g.hasArc(a, b));
        REQUIRE_FALSE(g.hasArc(b, a));   // обратной дуги нет
        REQUIRE_FALSE(g.hasArc(b, c));

        requireValidDirectedGraph(g);

        g.removeArc(ab);    // удаляем A->B

        REQUIRE(g.arcCount() == 1);
        REQUIRE(a.outDegree() == 1);
        REQUIRE(b.inDegree()  == 0);

        requireValidDirectedGraph(g);

        g.removeVertex(a);   // удаляем вершину A (и её дугу A->C)

        REQUIRE(g.vertexCount() == 2);
        REQUIRE(g.arcCount() == 0);
        requireValidDirectedGraph(g);
    }

    SECTION("Pseudo graph specific behavior (loops, multiarcs)") {
        Graph g;
        auto a = g.addVertex("A");
        auto b = g.addVertex("B");

        SECTION("Self-loops") {
            auto loop = g.addArc(a, a);   // петля
            REQUIRE(g.arcCount() == 1);
            REQUIRE(a.outDegree() == 1);
            REQUIRE(a.inDegree()  == 1);   // петля даёт +1 и к исходу, и к заходу
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
            g.addArc(a, a);      // петля
            g.addArc(a, b);      // A->B
            g.addArc(a, b);      // ещё одна A->B
            REQUIRE(g.arcCount() == 3);
            REQUIRE(a.outDegree() == 3); // петля + две дуги
            REQUIRE(a.inDegree()  == 1); // только петля
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
            REQUIRE(copy.hasVertex("A"));
            REQUIRE(copy.hasVertex("B"));
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
        REQUIRE(cg.hasVertex("Alpha"));
        REQUIRE(cg.hasVertex("Beta"));

        Graph::ConstVertexDescriptor ca, cb;
        for (auto v : cg.vertices()) if (v.data() == "Alpha") ca = v;
        for (auto v : cg.vertices()) if (v.data() == "Beta")  cb = v;

        REQUIRE(cg.hasArc(ca, cb));    // Alpha -> Beta
        REQUIRE_FALSE(cg.hasArc(cb, ca));
        requireValidDirectedGraph(cg);
    }
}

TEST_CASE("DirectedPseudoGraph weighted", "[directed][pseudograph][weighted]") {
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
}

TEST_CASE("DirectedPseudoGraph descriptors", "[directed][pseudograph][descriptors]") {
    using Graph = DirectedPseudoGraph<std::string, int>;
    using VertexDesc = Graph::VertexDescriptor;
    using ConstVertexDesc = Graph::ConstVertexDescriptor;
    using ArcDesc = Graph::ArcDescriptor;
    using ConstArcDesc = Graph::ConstArcDescriptor;

    Graph g;
    auto vA = g.addVertex("A");
    auto vB = g.addVertex("B");
    auto vC = g.addVertex("C");
    auto vD = g.addVertex("D");

    // Дуги: A->B (10), A->B (20) – кратные, A->A (30) – петля, B->C (40), C->D (50)
    auto aAB1 = g.addArc(vA, vB, 10);
    auto aAB2 = g.addArc(vA, vB, 20);
    auto aLoop = g.addArc(vA, vA, 30);
    auto aBC   = g.addArc(vB, vC, 40);
    auto aCD   = g.addArc(vC, vD, 50);

    requireValidDirectedGraph(g);

    SECTION("VertexDescriptor basics") {
        // Проверка полустепеней
        REQUIRE(vA.outDegree() == 3); // две A->B + петля
        REQUIRE(vA.inDegree()  == 1); // только петля
        REQUIRE(vB.outDegree() == 1); // B->C
        REQUIRE(vB.inDegree()  == 2); // две A->B
        REQUIRE(vC.outDegree() == 1); // C->D
        REQUIRE(vC.inDegree()  == 1); // B->C
        REQUIRE(vD.outDegree() == 0);
        REQUIRE(vD.inDegree()  == 1); // C->D

        // Изменение данных вершины
        REQUIRE(vA.data() == "A");
        vA.data() = "A1";
        REQUIRE(vA.data() == "A1");
        vA.data() = "A";

        // Исходящие дуги
        auto outA = vA.outgoingArcs();
        REQUIRE(outA.size() == 3);
        bool hasLoop = false, hasAB1 = false, hasAB2 = false;
        for (auto a : outA) {
            if (a.from() == vA && a.to() == vA) hasLoop = true;
            if (a.from() == vA && a.to() == vB) {
                if (a.data() == 10) hasAB1 = true;
                if (a.data() == 20) hasAB2 = true;
            }
        }
        REQUIRE(hasLoop);
        REQUIRE(hasAB1);
        REQUIRE(hasAB2);

        // Входящие дуги в A
        auto inA = vA.incomingArcs();
        REQUIRE(inA.size() == 1);
        REQUIRE((*inA.begin()).from() == vA);
        REQUIRE((*inA.begin()).to()   == vA);

        // ===== ИСХОДЯЩИЕ СОСЕДИ (adjacentVertices) =====
        {
            auto adjA = vA.adjacentVertices();
            REQUIRE(adjA.size() == 2);
            std::unordered_set<std::string> outNames;
            for (auto v : adjA) outNames.insert(v.data());
            REQUIRE(outNames.contains("A"));
            REQUIRE(outNames.contains("B"));

            // Неупорядоченная версия
            auto unordA = vA.unorderedOutV();
            REQUIRE(unordA.size() == 2);
            std::unordered_set<std::string> unordSet;
            for (auto v : unordA) unordSet.insert(v.data());
            REQUIRE(unordSet == outNames);

            // Сортировка с компаратором (по убыванию)
            auto adjADesc = vA.adjacentVertices<std::greater<std::string>>();
            REQUIRE(adjADesc.size() == 2);
            REQUIRE(adjADesc[0].data() == "B");
            REQUIRE(adjADesc[1].data() == "A");

            // Для B: исходящие соседи только C
            auto adjB = vB.adjacentVertices();
            REQUIRE(adjB.size() == 1);
            REQUIRE(adjB[0].data() == "C");
            REQUIRE(vB.unorderedOutV()[0].data() == "C");

            // Для D: нет исходящих соседей
            REQUIRE(vD.adjacentVertices().empty());
            REQUIRE(vD.unorderedOutV().empty());
        }

        // ===== ВХОДЯЩИЕ СОСЕДИ (incomingVertices) =====
        {
            auto inA = vA.incomingVertices();
            REQUIRE(inA.size() == 1);
            REQUIRE(inA[0].data() == "A");   // петля

            auto inB = vB.incomingVertices();
            REQUIRE(inB.size() == 1);
            REQUIRE(inB[0].data() == "A");

            auto inC = vC.incomingVertices();
            REQUIRE(inC.size() == 1);
            REQUIRE(inC[0].data() == "B");

            auto inD = vD.incomingVertices();
            REQUIRE(inD.size() == 1);
            REQUIRE(inD[0].data() == "C");

            // Неупорядоченная версия
            auto unordInD = vD.unorderedInV();
            REQUIRE(unordInD.size() == 1);
            REQUIRE(unordInD[0].data() == "C");

            // Сортировка (хотя бы для разнообразия – по возрастанию, по умолчанию)
            auto inBSorted = vB.incomingVertices<std::less<std::string>>();
            REQUIRE(inBSorted.size() == 1);
            REQUIRE(inBSorted[0].data() == "A");
        }
    }

    SECTION("ArcDescriptor basics") {
        REQUIRE(aAB1.data() == 10);
        aAB1.data() = 100;
        REQUIRE(aAB1.data() == 100);
        aAB1.data() = 10;

        REQUIRE(aAB1.from() == vA);
        REQUIRE(aAB1.to()   == vB);
        REQUIRE(aLoop.from() == vA);
        REQUIRE(aLoop.to()   == vA);

        // followArcDirection – только от начала
        auto follow = aAB1.followArcDirection(vA);
        REQUIRE(follow.has_value());
        REQUIRE(*follow == vB);
        follow = aAB1.followArcDirection(vB);
        REQUIRE_FALSE(follow.has_value());

        // otherEnd – неориентированный другой конец
        auto other = aAB1.otherEnd(vA);
        REQUIRE(other.has_value());
        REQUIRE(*other == vB);
        other = aAB1.otherEnd(vB);
        REQUIRE(other.has_value());
        REQUIRE(*other == vA);
        other = aAB1.otherEnd(vC);
        REQUIRE_FALSE(other.has_value());

        auto loopOther = aLoop.otherEnd(vA);
        REQUIRE(loopOther.has_value());
        REQUIRE(*loopOther == vA);

        ArcDesc copy = aAB1;
        REQUIRE(copy == aAB1);
        REQUIRE(copy != aAB2);
    }

    SECTION("Const descriptors") {
        const Graph& cg = g;
        ConstVertexDesc cvA, cvB;
        for (auto v : cg.vertices()) if (v.data() == "A") cvA = v;
        for (auto v : cg.vertices()) if (v.data() == "B") cvB = v;

        auto found = cg.findArc(cvA, cvB);
        REQUIRE(found.has_value());
        REQUIRE(found->from() == cvA);
        REQUIRE(found->to()   == cvB);
        REQUIRE(found->data() == 10);   // первая найденная дуга

        // Константные версии соседей
        auto constOut = cvA.adjacentVertices();
        REQUIRE(constOut.size() == 2);
        auto constIn = cvA.incomingVertices();
        REQUIRE(constIn.size() == 1);
    }

    SECTION("findArc and hasArc") {
        auto found = g.findArc(vA, vB);
        REQUIRE(found.has_value());
        REQUIRE(found->from() == vA);
        REQUIRE(found->to()   == vB);
        REQUIRE(g.hasArc(vA, vB));
        REQUIRE_FALSE(g.hasArc(vB, vA));

        auto foundLoop = g.findArc(vA, vA);
        REQUIRE(foundLoop.has_value());
        REQUIRE(foundLoop->data() == 30);

        auto notFound = g.findArc(vB, vD);
        REQUIRE_FALSE(notFound.has_value());
        REQUIRE_FALSE(g.hasArc(vB, vD));
    }

    SECTION("incomingVertices handles multiarcs and loops correctly") {
        Graph h;
        auto x = h.addVertex("X");
        auto y = h.addVertex("Y");
        auto z = h.addVertex("Z");

        h.addArc(x, y, 1);
        h.addArc(x, y, 2);   // кратные дуги X->Y
        h.addArc(y, x, 3);   // обратная дуга Y->X
        h.addArc(z, z, 4);   // петля на Z
        h.addArc(y, z, 5);

        // Входящие соседи для Y: только X (уникально)
        auto inY = y.incomingVertices();
        REQUIRE(inY.size() == 1);
        REQUIRE(inY[0].data() == "X");

        // Входящие соседи для X: Y (одна дуга)
        auto inX = x.incomingVertices();
        REQUIRE(inX.size() == 1);
        REQUIRE(inX[0].data() == "Y");

        // Входящие соседи для Z: Y (дуга) и Z (петля)
        auto inZ = z.incomingVertices();
        REQUIRE(inZ.size() == 2);
        std::unordered_set<std::string> expected = {"Y", "Z"};
        for (auto v : inZ) {
            REQUIRE(expected.contains(v.data()));
        }

        // Неупорядоченная версия для Z
        auto unordInZ = z.unorderedInV();
        REQUIRE(unordInZ.size() == 2);
        std::unordered_set<std::string> unordSet;
        for (auto v : unordInZ) unordSet.insert(v.data());
        REQUIRE(unordSet == expected);

        // Исходящие соседи не должны измениться
        auto outX = x.adjacentVertices();
        REQUIRE(outX.size() == 1);
        REQUIRE(outX[0].data() == "Y");
    }
}
