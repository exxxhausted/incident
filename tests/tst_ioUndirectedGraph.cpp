#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <vector>

#include "incident/UndirectedGraph.hpp"
#include "incident/io/UndirectedGraphFromMatrix.hpp"
#include "incident/io/UndirectedGraphToMatrix.hpp"

using namespace exx::incident;

TEST_CASE("buildUndirectedGraph and toAdjacencyMatrix", "[build][matrix]") {

    SECTION("Raw array weighted") {
        int matrix[] = {
            0,5,0,
            5,0,7,
            0,7,0
        };
        auto result = buildUndirectedGraph<int>(matrix, 3);
        REQUIRE(result.has_value());
        auto& g = result.value();
        CHECK(g.vertexCount() == 3);
        CHECK(g.edgeCount() == 2);
        auto e01 = g.findEdge(0,1);
        REQUIRE(e01.has_value());
        CHECK(e01->data() == 5);
        auto e12 = g.findEdge(1,2);
        REQUIRE(e12.has_value());
        CHECK(e12->data() == 7);
    }

    SECTION("Raw array unweighted") {
        bool matrix[] = {
            0,1,0,
            1,0,1,
            0,1,0
        };
        auto result = buildUndirectedGraph(matrix, 3);
        REQUIRE(result.has_value());
        auto& g = result.value();
        CHECK(g.vertexCount() == 3);
        CHECK(g.edgeCount() == 2);
        CHECK(g.hasEdge(0,1));
        CHECK(g.hasEdge(1,2));
    }

    SECTION("Raw array errors") {
        auto r1 = buildUndirectedGraph<int>(nullptr, 5);
        REQUIRE(!r1.has_value());
        CHECK(r1.error() == GraphBuildingError::NullPointer);
        int dummy = 0;
        auto r2 = buildUndirectedGraph<int>(&dummy, 0);
        REQUIRE(!r2.has_value());
        CHECK(r2.error() == GraphBuildingError::ZeroSize);
    }

    SECTION("Vector weighted") {
        std::vector<std::vector<int>> m = {
            {0,5,0},
            {5,0,7},
            {0,7,0}
        };
        auto result = buildUndirectedGraph<int>(m);
        REQUIRE(result.has_value());
        auto& g = result.value();
        CHECK(g.vertexCount() == 3);
        CHECK(g.edgeCount() == 2);
        CHECK(g.findEdge(0,1)->data() == 5);
        CHECK(g.findEdge(1,2)->data() == 7);
    }

    SECTION("Vector unweighted") {
        std::vector<std::vector<bool>> m = {
            {0,1,0},
            {1,0,1},
            {0,1,0}
        };
        auto result = buildUndirectedGraph(m);
        REQUIRE(result.has_value());
        auto& g = result.value();
        CHECK(g.vertexCount() == 3);
        CHECK(g.edgeCount() == 2);
        CHECK(g.hasEdge(0,1));
        CHECK(g.hasEdge(1,2));
    }

    SECTION("Ignore lower triangle and loops") {
        std::vector<std::vector<int>> m = {
            { 99,  5,  0},
            {100, 99,  7},
            {  0,  8, 99}};
        auto result = buildUndirectedGraph<int>(m);
        REQUIRE(result.has_value());
        auto& g = result.value();
        CHECK(g.edgeCount() == 2);
        auto e = g.findEdge(1,0);
        REQUIRE(e.has_value());
        CHECK(e->data() == 5);
    }

    SECTION("Vector errors") {
        std::vector<std::vector<int>> nonSquare = {
            {1,2,3},
            {4,5,6}
        };
        auto r1 = buildUndirectedGraph<int>(nonSquare);
        REQUIRE(!r1.has_value());
        CHECK(r1.error() == GraphBuildingError::NonSquareMatrix);

        std::vector<std::vector<int>> inconsistent = {
            {1,2},
            {3,4,5}};
        auto r2 = buildUndirectedGraph<int>(inconsistent);
        REQUIRE(!r2.has_value());
        CHECK(r2.error() == GraphBuildingError::NonSquareMatrix);
    }

    SECTION("To adjacency unweighted") {
        UndirectedGraph<std::size_t, void> g;
        g.addVertex(0); g.addVertex(1); g.addVertex(2);
        g.addEdge(0,1); g.addEdge(1,2);
        auto mat = toAdjacencyMatrix(g);
        CHECK(mat.rows() == 3);
        CHECK(mat.cols() == 3);
        CHECK(mat(0,0) == false);
        CHECK(mat(0,1) == true);
        CHECK(mat(0,2) == false);
        CHECK(mat(1,0) == true);
        CHECK(mat(1,1) == false);
        CHECK(mat(1,2) == true);
        CHECK(mat(2,0) == false);
        CHECK(mat(2,1) == true);
        CHECK(mat(2,2) == false);
    }

    SECTION("To adjacency weighted with default") {
        UndirectedGraph<std::size_t, int> g;
        g.addVertex(0); g.addVertex(1); g.addVertex(2);
        g.addEdge(0,1,5); g.addEdge(1,2,7);
        auto mat = toAdjacencyMatrix(g, -1);
        CHECK(mat.rows() == 3);
        CHECK(mat(0,0) == -1);
        CHECK(mat(0,1) == 5);
        CHECK(mat(0,2) == -1);
        CHECK(mat(1,0) == 5);
        CHECK(mat(1,1) == -1);
        CHECK(mat(1,2) == 7);
        CHECK(mat(2,0) == -1);
        CHECK(mat(2,1) == 7);
        CHECK(mat(2,2) == -1);
    }

    SECTION("Empty graph to matrix") {
        UndirectedGraph<std::size_t, int> g;
        auto mat = toAdjacencyMatrix(g, 0);
        CHECK(mat.rows() == 0);
        CHECK(mat.cols() == 0);
    }
}
