#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

#include "incident/UndirectedAbstractGraph.hpp"
#include "incident/algorithms/creation_of_UndirectedAbstractGraph.hpp"

using namespace exx::incident;

template<typename Graph>
int getEdgeWeight(const Graph& g, int fromIdx, int toIdx) {
    for (auto edge : g.edges()) {
        int v1 = (*edge.v1()).data();
        int v2 = (*edge.v2()).data();
        if ((v1 == fromIdx && v2 == toIdx) || (v1 == toIdx && v2 == fromIdx)) {
            return edge.data();
        }
    }
    return -1;
}

TEST_CASE("Graph creation from matrix views", "[graph_factory]") {
    SECTION("MatrixView creation") {
        int raw[] = {
            0, 2, 0, 5,
            2, 0, 3, 0,
            0, 3, 0, 7,
            5, 0, 7, 0
        };
        auto graph = make_graph_from_matrix<int, int>({raw, 4, 4}, 0);

        REQUIRE(graph.vertexCount() == 4);
        std::vector<int> indices;
        for (auto v : graph.vertices()) indices.push_back(v.data());
        REQUIRE_THAT(indices, Catch::Matchers::UnorderedEquals(std::vector<int>{0,1,2,3}));

        REQUIRE(graph.edgeCount() == 4);
        REQUIRE(getEdgeWeight(graph, 0, 1) == 2);
        REQUIRE(getEdgeWeight(graph, 0, 3) == 5);
        REQUIRE(getEdgeWeight(graph, 1, 2) == 3);
        REQUIRE(getEdgeWeight(graph, 2, 3) == 7);
        REQUIRE(getEdgeWeight(graph, 0, 2) == -1);
        REQUIRE(getEdgeWeight(graph, 0, 1) != -1);
    }

    SECTION("MatrixOwningView creation") {
        int raw[] = {
            0, 2, 0, 5,
            2, 0, 3, 0,
            0, 3, 0, 7,
            5, 0, 7, 0
        };
        auto graph = make_graph_from_matrix<int, int>(MatrixOwningView<int>(raw, 4, 4), 0);
        REQUIRE(graph.vertexCount() == 4);
        REQUIRE(graph.edgeCount() == 4);
        REQUIRE(getEdgeWeight(graph, 1, 2) == 3);
    }

    SECTION("Custom noEdgeValue") {
        int raw[] = {
            -1, 2, -1,
            2, -1, 3,
            -1, 3, -1
        };
        MatrixView<int> view(raw, 3, 3);
        auto graph = make_graph_from_matrix<int>(view, -1);
        REQUIRE(graph.vertexCount() == 3);
        REQUIRE(graph.edgeCount() == 2);
        REQUIRE(getEdgeWeight(graph, 0, 1) == 2);
        REQUIRE(getEdgeWeight(graph, 1, 2) == 3);
        REQUIRE(getEdgeWeight(graph, 0, 2) == -1);
    }

    SECTION("Empty matrix (0x0)") {
        int* raw = nullptr;
        MatrixView<int> view(raw, 0, 0);
        auto graph = make_graph_from_matrix<int>(view, 0);
        REQUIRE(graph.vertexCount() == 0);
        REQUIRE(graph.edgeCount() == 0);
    }

    SECTION("Non-square matrix throws") {
        int raw[] = {1,2,3,4,5,6};
        MatrixView<int> view(raw, 2, 3);
        REQUIRE_THROWS_AS(make_graph_from_matrix<int>(view, 0), std::invalid_argument);
    }

    SECTION("MatrixOwningView from initializer_list") {
        auto graph = make_graph_from_matrix<int>({{0,1,0},{1,0,2},{0,2,0}}, 0);
        REQUIRE(graph.vertexCount() == 3);
        REQUIRE(graph.edgeCount() == 2);
        REQUIRE(getEdgeWeight(graph, 0, 1) == 1);
        REQUIRE(getEdgeWeight(graph, 1, 2) == 2);
    }

    SECTION("VertexData type other than int") {
        using Graph = UndirectedAbstractGraph<size_t, double>;
        double raw[] = {
            0.0, 1.5, 0.0,
            1.5, 0.0, 2.3,
            0.0, 2.3, 0.0
        };
        MatrixView<double> view(raw, 3, 3);
        Graph graph = make_graph_from_matrix<size_t, double>(view, 0.0);
        REQUIRE(graph.vertexCount() == 3);
        REQUIRE(graph.edgeCount() == 2);
        for (auto e : graph.edges()) {
            size_t v1 = (*e.v1()).data();
            size_t v2 = (*e.v2()).data();
            double w = e.data();
            if ((v1 == 0 && v2 == 1) || (v1 == 1 && v2 == 0)) REQUIRE(w == 1.5);
            if ((v1 == 1 && v2 == 2) || (v1 == 2 && v2 == 1)) REQUIRE(w == 2.3);
        }
    }
}
