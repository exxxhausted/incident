#include <catch2/catch_test_macros.hpp>

#include "incident/directed/DirectedGraph.hpp"
#include "incident/utility/UniqueVertexIndexedView.hpp"

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

TEST_CASE("DirectedGraph unweighted", "[directed][graph][unweighted]") {
    using Graph = DirectedGraph<std::string, void>;

    SECTION("Simple graph specific behavior") {
        Graph g;

        auto a = g.addVertex("A");
        auto b = g.addVertex("B");

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

    SECTION("Unweighted graph (void) from matrix") {
        using Graph = DirectedGraph<int, void>;

        std::vector<std::vector<bool>> mat = {
            {0, 1, 1},
            {0, 0, 1},
            {0, 0, 0}
        };

        auto graphResult = Graph::fromAdjacencyMatrix(mat);
        REQUIRE(graphResult.has_value());
        auto g = UniqueVertexIndexedView(*graphResult);

        REQUIRE(g.graph().vertexCount() == 3);
        REQUIRE(g.graph().arcCount() == 3); // 0->1, 0->2, 1->2

        REQUIRE(g.graph().hasArc(*g.findVertex(0), *g.findVertex(1)));
        REQUIRE(g.graph().hasArc(*g.findVertex(0), *g.findVertex(2)));
        REQUIRE(g.graph().hasArc(*g.findVertex(1), *g.findVertex(2)));
        REQUIRE_FALSE(g.graph().hasArc(*g.findVertex(1), *g.findVertex(0)));
        REQUIRE_FALSE(g.graph().hasArc(*g.findVertex(2), *g.findVertex(0)));

        auto matOut = g.graph().toAdjacencyMatrix();
        REQUIRE(matOut.rows() == 3);
        REQUIRE(matOut.cols() == 3);
        for (size_t i = 0; i < 3; ++i) {
            for (size_t j = 0; j < 3; ++j) {
                bool expected = (i < j);
                REQUIRE(matOut(i, j) == expected);
            }
        }

        std::vector<std::vector<bool>> emptyMat;
        auto emptyResult = Graph::fromAdjacencyMatrix(emptyMat);
        REQUIRE_FALSE(emptyResult.has_value());
        REQUIRE(emptyResult.error() == GraphBuildingError::ZeroSize);

        std::vector<std::vector<bool>> nonSquare = {
            {0, 1},
            {0, 0},
            {0, 0}
        };
        auto nonSquareResult = Graph::fromAdjacencyMatrix(nonSquare);
        REQUIRE_FALSE(nonSquareResult.has_value());
        REQUIRE(nonSquareResult.error() == GraphBuildingError::NonSquareMatrix);
    }
}

TEST_CASE("DirectedGraph weighted", "[directed][graph][weighted]") {
    SECTION("Weighted graph (int) from vector<vector<int>>") {
        using Graph = DirectedGraph<int, int>;

        std::vector<std::vector<int>> mat = {
            {0, 5, 0},
            {0, 0, 2},
            {0, 0, 0}
        };

        auto graphResult = Graph::fromAdjacencyMatrix(mat);
        REQUIRE(graphResult.has_value());
        auto g = UniqueVertexIndexedView(*graphResult);

        REQUIRE(g.graph().vertexCount() == 3);
        REQUIRE(g.graph().arcCount() == 2);

        auto e01 = g.graph().findArc(*g.findVertex(0), *g.findVertex(1));
        REQUIRE(e01.has_value());
        REQUIRE(e01->data() == 5);

        auto e12 = g.graph().findArc(*g.findVertex(1), *g.findVertex(2));
        REQUIRE(e12.has_value());
        REQUIRE(e12->data() == 2);

        auto matOut = g.graph().toAdjacencyMatrix(0);
        REQUIRE(matOut.rows() == 3);
        REQUIRE(matOut.cols() == 3);
        for (size_t i = 0; i < 3; ++i)
            for (size_t j = 0; j < 3; ++j)
                REQUIRE(matOut(i, j) == mat[i][j]);
    }

    SECTION("Unweighted from flat vector<bool> and size") {
        using Graph = DirectedGraph<int, void>;

        std::vector<bool> flat = {
            0, 1, 1,
            0, 0, 1,
            0, 0, 0
        };
        auto result = Graph::fromAdjacencyMatrix(flat, 3);
        REQUIRE(result.has_value());
        REQUIRE(result->arcCount() == 3);

        auto nullResult = Graph::fromAdjacencyMatrix(std::vector<bool>{}, 5);
        REQUIRE_FALSE(nullResult.has_value());
        REQUIRE(nullResult.error() == GraphBuildingError::EmptyVector);

        auto zeroResult = Graph::fromAdjacencyMatrix(flat, 0);
        REQUIRE_FALSE(zeroResult.has_value());
        REQUIRE(zeroResult.error() == GraphBuildingError::ZeroSize);
    }

    SECTION("Weighted from raw pointer") {
        using Graph = DirectedGraph<int, double>;

        std::vector<double> flat = {
            0, 1.5, 0,
            0, 0, 2.5,
            0, 0, 0
        };
        auto result = Graph::fromAdjacencyMatrix(flat.data(), 3);
        REQUIRE(result.has_value());
        auto g = UniqueVertexIndexedView(*result);

        REQUIRE(g.graph().arcCount() == 2);
        REQUIRE(g.graph().findArc(*g.findVertex(0), *g.findVertex(1))->data() == 1.5);
        REQUIRE(g.graph().findArc(*g.findVertex(1), *g.findVertex(2))->data() == 2.5);
    }
}
