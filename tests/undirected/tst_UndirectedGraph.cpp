#include <catch2/catch_test_macros.hpp>

#include "incident/undirected/UndirectedGraph.hpp"
#include "incident/utility/UniqueVertexIndexedView.hpp"

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

TEST_CASE("UndirectedGraph unweighted",
          "[undirected][graph][unweighted]")
{
    using Graph = UndirectedGraph<std::string, void>;

    SECTION("Simple graph specific befavior") {
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

        SECTION("No multiple edges") {
            g.addEdge(a, b);
            auto e = g.addEdge(a, b);

            REQUIRE(g.edgeCount() == 1);
            REQUIRE(a.degree() == 1);
            REQUIRE(!e.has_value());

            requireValidUndirectedGraph(g);
        }
    }

    SECTION("Unweighted graph (void)") {
        using Graph = UndirectedGraph<int, void>;
        std::vector<std::vector<bool>> mat = {
            {0, 1, 1},
            {1, 0, 1},
            {1, 1, 0}
        };

        auto graphResult = Graph::fromAdjacencyMatrix(mat);
        REQUIRE(graphResult.has_value());
        auto g = UniqueVertexIndexedView(*graphResult);

        REQUIRE(g.graph().vertexCount() == 3);
        REQUIRE(g.graph().edgeCount() == 3);
        REQUIRE(g.graph().hasEdge(*g.findVertex(0), *g.findVertex(1)));
        REQUIRE(g.graph().hasEdge(*g.findVertex(0), *g.findVertex(2)));
        REQUIRE(g.graph().hasEdge(*g.findVertex(1), *g.findVertex(2)));

        auto matOut = g.graph().toAdjacencyMatrix();
        REQUIRE(matOut.rows() == 3);
        REQUIRE(matOut.cols() == 3);
        for (size_t i = 0; i < 3; ++i) {
            for (size_t j = 0; j < 3; ++j) {
                bool expected = (i != j);
                REQUIRE(matOut(i, j) == expected);
            }
        }

        std::vector<std::vector<bool>> emptyMat;
        auto emptyResult = Graph::fromAdjacencyMatrix(emptyMat);
        REQUIRE_FALSE(emptyResult.has_value());
        REQUIRE(emptyResult.error() == GraphBuildingError::ZeroSize);

        std::vector<std::vector<bool>> nonSquare = {{false, true}, {true, false}, {false, false}};
        auto nonSquareResult = Graph::fromAdjacencyMatrix(nonSquare);
        REQUIRE_FALSE(nonSquareResult.has_value());
        REQUIRE(nonSquareResult.error() == GraphBuildingError::NonSquareMatrix);
    }
}

TEST_CASE("UndirectedGraph weighted",
          "[undirected][graph][weighted]")
{
    SECTION("Weighted graph (int)") {
        using Graph = UndirectedGraph<int, int>;

        std::vector<std::vector<int>> mat = {
            {0, 5, 0},
            {5, 0, 2},
            {0, 2, 0}
        };

        auto graphResult = Graph::fromAdjacencyMatrix(mat);
        REQUIRE(graphResult.has_value());
        auto g = UniqueVertexIndexedView(*graphResult);

        REQUIRE(g.graph().vertexCount() == 3);
        REQUIRE(g.graph().edgeCount() == 2);

        auto e01 = g.graph().findEdge(*g.findVertex(0), *g.findVertex(1));
        REQUIRE(e01.has_value());
        REQUIRE(e01->data() == 5);

        auto e12 = g.graph().findEdge(*g.findVertex(1), *g.findVertex(2));
        REQUIRE(e12.has_value());
        REQUIRE(e12->data() == 2);

        auto matOut = g.graph().toAdjacencyMatrix(0);
        REQUIRE(matOut.rows() == 3);
        REQUIRE(matOut.cols() == 3);
        for (size_t i = 0; i < 3; ++i)
            for (size_t j = 0; j < 3; ++j)
                REQUIRE(matOut(i, j) == mat[i][j]);
    }

    SECTION("From raw pointer and vector<bool>") {
        using Graph = UndirectedGraph<int, void>;

        std::vector<bool> flat = {
            0, 1, 1,
            1, 0, 1,
            1, 1, 1
        };
        auto result = Graph::fromAdjacencyMatrix(flat, 3);
        REQUIRE(result.has_value());
        REQUIRE(result->edgeCount() == 3);

        auto nullResult = Graph::fromAdjacencyMatrix(std::vector<bool>{}, 5);
        REQUIRE_FALSE(nullResult.has_value());
        REQUIRE(nullResult.error() == GraphBuildingError::EmptyVector);

        auto zeroResult = Graph::fromAdjacencyMatrix(flat, 0);
        REQUIRE_FALSE(zeroResult.has_value());
        REQUIRE(zeroResult.error() == GraphBuildingError::ZeroSize);
    }

    SECTION("Weighted from raw pointer") {
        using Graph = UndirectedGraph<int, double>;

        std::vector<double> flat = {
            0, 1.5, 0,
            1.5, 0, 2.5,
            0, 2.5, 0
        };
        auto result = Graph::fromAdjacencyMatrix(flat.data(), 3);
        REQUIRE(result.has_value());
        auto g = UniqueVertexIndexedView(*result);

        REQUIRE(g.graph().edgeCount() == 2);
        REQUIRE(g.graph().findEdge(*g.findVertex(0), *g.findVertex(1))->data() == 1.5);
        REQUIRE(g.graph().findEdge(*g.findVertex(1), *g.findVertex(2))->data() == 2.5);
    }
}

