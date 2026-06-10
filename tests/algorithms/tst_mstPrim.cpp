#include <catch2/catch_test_macros.hpp>

#include "incident/algorithms/mstPrim.hpp"

#include "incident/undirected/UndirectedGraph.hpp"
#include "incident/undirected/UndirectedMultiGraph.hpp"
#include "incident/undirected/UndirectedPseudoGraph.hpp"

using namespace exx::incident;

template<typename G>
auto totalWeight(const G& g)
{
    typename G::EdgeValueType sum{};

    for (auto e : g.edges())
        sum += e.data();

    return sum;
}

template<typename VertexData, typename EdgeData>
static auto makeGraphFromMatrix(const EdgeData* matrix, std::size_t n)
{
    using Graph = UndirectedGraph<VertexData, EdgeData>;
    Graph g;
    std::vector<typename Graph::VertexDescriptor> vertices;
    vertices.reserve(n);
    for (std::size_t i = 0; i < n; ++i)
        vertices.push_back(g.addVertex(static_cast<VertexData>(i)));

    for (std::size_t i = 0; i < n; ++i) {
        for (std::size_t j = i + 1; j < n; ++j) {
            auto val = matrix[i * n + j];
            if (val != EdgeData{}) {
                g.addEdge(vertices[i], vertices[j], val);
            }
        }
    }
    return g;
}

TEST_CASE("Prim", "[prim][undirected]") {
    SECTION("Prim builds MST on simple graph") {
        /*
            1
        0 ----- 1
        | \     |
      4 |  \2   | 3
        |   \   |
        2 ----- 3
            5

        Expected MST:
            0-1 (1)
            0-3 (2)
            1-2 (3)

        Total:
            6
        */

        int matrix[] = {0,1,4,2, 1,0,3,0, 4,3,0,5, 2,0,5,0};
        auto g = makeGraphFromMatrix<int, int>(matrix, 4);

        auto mstExpected = mstPrim(g);
        REQUIRE(mstExpected.has_value());

        auto& mst = mstExpected.value();
        REQUIRE(mst.vertexCount() == 4);
        REQUIRE(mst.edgeCount() == 3);
        REQUIRE(totalWeight(mst) == 6);
    }

    SECTION("Prim handles single vertex graph") {
        int matrix[] = {0};
        auto g = makeGraphFromMatrix<int, int>(matrix, 1);

        auto mstExpected = mstPrim(g);
        REQUIRE(mstExpected.has_value());

        auto& mst = mstExpected.value();
        REQUIRE(mst.vertexCount() == 1);
        REQUIRE(mst.edgeCount() == 0);
    }

    SECTION("Prim handles disconnected graph") {
        /*
        0 -- 1

        2 -- 3
        */
        int matrix[] = {0,1,0,0, 1,0,0,0, 0,0,0,1, 0,0,1,0};
        auto g = makeGraphFromMatrix<int, int>(matrix, 4);

        auto mstExpected = mstPrim(g);
        REQUIRE_FALSE(mstExpected.has_value());
        REQUIRE(mstExpected.error() == PrimError::DisconnectedGraph);
    }

    SECTION("Prim ignores self-loops") {
        /*
        Base graph:
                  _100__
                  \    /
                   \  /
            0 --1-- 1 --2-- 2

        Loop must NOT appear in MST.

        Expected total:
            3
        */
        int matrix[] = {0,1,0, 1,0,2, 0,2,0};
        auto simple = makeGraphFromMatrix<int, int>(matrix, 3);

        UndirectedPseudoGraph<int, int> g(simple.baseMultiGraph().basePseudoGraph());

        // Build local index map from vertex data to descriptor
        std::unordered_map<int, UndirectedPseudoGraph<int, int>::VertexDescriptor> desc;
        for (auto v : g.vertices())
            desc[v.data()] = v;

        auto v1 = desc[1];
        g.addEdge(v1, v1, 100);

        auto mstExpected = mstPrim(g);
        REQUIRE(mstExpected.has_value());

        auto& mst = mstExpected.value();
        REQUIRE(mst.vertexCount() == 3);
        REQUIRE(mst.edgeCount() == 2);
        REQUIRE(totalWeight(mst) == 3);
    }

    SECTION("Prim selects minimal multiedge") {
        /*
        Base graph:
            0 --10-- 1
            1 --5--- 2

        Add cheaper multiedge:
            0 --1--- 1

        Expected MST:
            0 --1--- 1
            1 --5--- 2

        Total:
            6
        */
        int matrix[] = {0,10,0, 10,0,5, 0,5,0};
        auto simple = makeGraphFromMatrix<int, int>(matrix, 3);
        UndirectedMultiGraph<int, int> g(simple.baseMultiGraph());

        // Build local index map
        std::unordered_map<int, UndirectedMultiGraph<int, int>::VertexDescriptor> desc;
        for (auto v : g.vertices())
            desc[v.data()] = v;

        auto v0 = desc[0];
        auto v1 = desc[1];
        g.addEdge(v0, v1, 1);

        auto mstExpected = mstPrim(g);
        REQUIRE(mstExpected.has_value());

        auto& mst = mstExpected.value();
        REQUIRE(mst.vertexCount() == 3);
        REQUIRE(mst.edgeCount() == 2);
        REQUIRE(totalWeight(mst) == 6);
    }

    SECTION("Prim handles equal edge weights") {
        /*
            1
        0 ----- 1
        |       |
      1 |       | 1
        |       |
        2 ----- 3
            1
        */
        int matrix[] = {0,1,1,0, 1,0,0,1, 1,0,0,1, 0,1,1,0};
        auto g = makeGraphFromMatrix<int, int>(matrix, 4);

        auto mstExpected = mstPrim(g);
        REQUIRE(mstExpected.has_value());

        auto& mst = mstExpected.value();
        REQUIRE(mst.vertexCount() == 4);
        REQUIRE(mst.edgeCount() == 3);
        REQUIRE(totalWeight(mst) == 3);
    }

    SECTION("Prim handles negative weights") {
        /*
            -5
        0 ------ 1
         \      /
       2  \    / 1
           \  /
            2
        */
        int matrix[] = {0,-5,2, -5,0,1, 2,1,0};
        auto g = makeGraphFromMatrix<int, int>(matrix, 3);

        auto mstExpected = mstPrim(g);
        REQUIRE(mstExpected.has_value());

        auto& mst = mstExpected.value();
        REQUIRE(mst.vertexCount() == 3);
        REQUIRE(mst.edgeCount() == 2);
        REQUIRE(totalWeight(mst) == -4);
    }
}
