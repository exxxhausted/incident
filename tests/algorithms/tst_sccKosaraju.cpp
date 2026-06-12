#include <catch2/catch_test_macros.hpp>
#include <unordered_set>
#include <vector>

#include "incident/directed/DirectedGraph.hpp"
#include "incident/algorithms/sccKosaraju.hpp"

using namespace exx::incident;
using TestGraph = DirectedGraph<int, int>;
using Vertex = TestGraph::VertexDescriptor;
using VertexC = TestGraph::ConstVertexDescriptor;

TEST_CASE("Kosaraju SCC", "[kosaraju]") {
    SECTION("Empty graph") {
        TestGraph g;
        auto sccs = sccKosaraju(g);
        REQUIRE(sccs.componentCount() == 0);
        REQUIRE(sccs.components().empty());
    }

    SECTION("Single vertex") {
        TestGraph g;
        Vertex v = g.addVertex(0);
        auto sccs = sccKosaraju(g);
        REQUIRE(sccs.componentCount() == 1);
        const auto& comp = sccs.components().front();
        REQUIRE(comp.size() == 1);
        REQUIRE(comp.contains(v));
        REQUIRE(sccs.areStronglyConnected(v, v));
    }

    SECTION("Two vertices, no edge") {
        TestGraph g;
        Vertex v0 = g.addVertex(0);
        Vertex v1 = g.addVertex(1);
        auto sccs = sccKosaraju(g);
        REQUIRE(sccs.componentCount() == 2);
        REQUIRE(!sccs.areStronglyConnected(v0, v1));
        REQUIRE(!sccs.areStronglyConnected(v1, v0));
        REQUIRE(sccs.componentOf(v0).value().size() == 1);
        REQUIRE(sccs.componentOf(v1).value().size() == 1);
    }

    SECTION("Two vertices, single edge 0->1") {
        TestGraph g;
        Vertex v0 = g.addVertex(0);
        Vertex v1 = g.addVertex(1);
        g.addArc(v0, v1, 1);
        auto sccs = sccKosaraju(g);
        REQUIRE(sccs.componentCount() == 2);
        REQUIRE(!sccs.areStronglyConnected(v0, v1));
        REQUIRE(!sccs.areStronglyConnected(v1, v0));
    }

    SECTION("Two vertices, mutual edges") {
        TestGraph g;
        Vertex v0 = g.addVertex(0);
        Vertex v1 = g.addVertex(1);
        g.addArc(v0, v1, 1);
        g.addArc(v1, v0, 1);
        auto sccs = sccKosaraju(g);
        REQUIRE(sccs.componentCount() == 1);
        const auto& comp = sccs.components().front();
        REQUIRE(comp.size() == 2);
        REQUIRE(comp.contains(v0));
        REQUIRE(comp.contains(v1));
        REQUIRE(sccs.areStronglyConnected(v0, v1));
    }

    SECTION("Triangle 0->1, 1->2, 2->0") {
        TestGraph g;
        Vertex v0 = g.addVertex(0);
        Vertex v1 = g.addVertex(1);
        Vertex v2 = g.addVertex(2);
        g.addArc(v0, v1, 1);
        g.addArc(v1, v2, 1);
        g.addArc(v2, v0, 1);
        auto sccs = sccKosaraju(g);
        REQUIRE(sccs.componentCount() == 1);
        const auto& comp = sccs.components().front();
        REQUIRE(comp.size() == 3);
        REQUIRE(comp.contains(v0));
        REQUIRE(comp.contains(v2));
        REQUIRE(sccs.areStronglyConnected(v0, v2));
    }

    SECTION("Two separate cycles and isolated vertex") {
        TestGraph g;
        Vertex v0 = g.addVertex(0), v1 = g.addVertex(1);
        Vertex v2 = g.addVertex(2), v3 = g.addVertex(3);
        Vertex v4 = g.addVertex(4);
        g.addArc(v0, v1, 1); g.addArc(v1, v0, 1);
        g.addArc(v2, v3, 1); g.addArc(v3, v2, 1);
        auto sccs = sccKosaraju(g);
        REQUIRE(sccs.componentCount() == 3);
        const auto& comp01 = sccs.componentOf(v0);
        const auto& comp23 = sccs.componentOf(v2);
        const auto& comp4  = sccs.componentOf(v4);
        REQUIRE(comp01.value().size() == 2);
        REQUIRE(comp23.value().size() == 2);
        REQUIRE(comp4.value().size() == 1);
        REQUIRE(comp01.value().contains(v1));
        REQUIRE(comp23.value().contains(v3));
        REQUIRE_FALSE(sccs.areStronglyConnected(v0, v2));
    }

    SECTION("Chain 0->1->2 with back edge 2->1 (SCC {1,2})") {
        TestGraph g;
        Vertex v0 = g.addVertex(0), v1 = g.addVertex(1), v2 = g.addVertex(2);
        g.addArc(v0, v1, 1);
        g.addArc(v1, v2, 1);
        g.addArc(v2, v1, 1);
        auto sccs = sccKosaraju(g);
        REQUIRE(sccs.componentCount() == 2);
        REQUIRE(sccs.componentOf(v0).value().size() == 1);
        REQUIRE(sccs.componentOf(v1).value().size() == 2);
        REQUIRE(sccs.areStronglyConnected(v1, v2));
        REQUIRE_FALSE(sccs.areStronglyConnected(v0, v1));
    }

    SECTION("All vertices in one SCC (4 vertices)") {
        TestGraph g;
        Vertex v0 = g.addVertex(0), v1 = g.addVertex(1);
        Vertex v2 = g.addVertex(2), v3 = g.addVertex(3);
        g.addArc(v0, v1, 1); g.addArc(v1, v0, 1);
        g.addArc(v1, v2, 1);
        g.addArc(v2, v3, 1); g.addArc(v3, v2, 1);
        g.addArc(v3, v1, 1);
        auto sccs = sccKosaraju(g);
        REQUIRE(sccs.componentCount() == 1);
        REQUIRE(sccs.components().front().size() == 4);
        REQUIRE(sccs.areStronglyConnected(v0, v2));
    }

    SECTION("DAG linear 0->1->2->3 – each vertex separate") {
        TestGraph g;
        std::vector<Vertex> v;
        for (int i = 0; i < 4; ++i) v.push_back(g.addVertex(i));
        g.addArc(v[0], v[1], 1);
        g.addArc(v[1], v[2], 1);
        g.addArc(v[2], v[3], 1);
        auto sccs = sccKosaraju(g);
        REQUIRE(sccs.componentCount() == 4);
        for (size_t i = 0; i < v.size(); ++i) {
            REQUIRE(sccs.componentOf(v[i]).value().size() == 1);
            REQUIRE(sccs.componentOf(v[i]).value().contains(v[i]));
        }
        REQUIRE_FALSE(sccs.areStronglyConnected(v[0], v[1]));
    }

    SECTION("Partition property (vertices covered exactly once)") {
        TestGraph g;
        std::vector<Vertex> v;
        for (int i = 0; i < 5; ++i) v.push_back(g.addVertex(i));
        g.addArc(v[0], v[1], 1);
        g.addArc(v[0], v[2], 1);
        g.addArc(v[1], v[3], 1);
        g.addArc(v[2], v[3], 1);
        g.addArc(v[3], v[4], 1);
        auto sccs = sccKosaraju(g);
        std::unordered_set<VertexC> covered;
        for (const auto& comp : sccs.components()) {
            for (auto x : comp.vertices()) {
                REQUIRE(covered.find(x) == covered.end());
                covered.insert(x);
            }
        }
        REQUIRE(covered.size() == v.size());
    }
}
