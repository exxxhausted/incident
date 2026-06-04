/*

================================================================================
           МИНИМАЛЬНОЕ ОСТОВНОЕ ДЕРЕВО (MST) – АЛГОРИТМ ПРИМА/КРУСКАЛА
         Сравнение API различных графовых библиотек C++ на одном примере
================================================================================

Исходный граф (неориентированный, взвешенный) задан матрицей смежности 5x5.
Вес 0 означает отсутствие ребра.

Матрица смежности:
    0   1   2   3   4
0   0   2   0   6   0
1   2   0   3   8   5
2   0   3   0   0   7
3   6   8   0   0   9
4   0   5   7   9   0

Ожидаемое минимальное остовное дерево (веса рёбер: 2, 3, 5, 6)
Матрица смежности МОД:
0 2 0 6 0
2 0 3 0 5
0 3 0 0 0
6 0 0 0 0
0 5 0 0 0

Ниже приведены компилируемые(?) примеры кода для каждой библиотеки.
-----------------------------------------------------------------------



1. BOOST GRAPH LIBRARY (BGL)
   – Алгоритм: prim_minimum_spanning_tree
   – API: шаблонные функции + property maps, граф adjacency_list

-----------------------------------------------------------------------
#include <iostream>
#include <vector>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/prim_minimum_spanning_tree.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/property_map/property_map.hpp>

int main() {
    const int num_vertices = 5;
    int adjacency_matrix[num_vertices][num_vertices] = {
        {0, 2, 0, 6, 0},
        {2, 0, 3, 8, 5},
        {0, 3, 0, 0, 7},
        {6, 8, 0, 0, 9},
        {0, 5, 7, 9, 0}
    };

    using namespace boost;
    typedef adjacency_list<vecS, vecS, undirectedS,
        no_property, property<edge_weight_t, int>> Graph;

    Graph g(num_vertices);

    for (int i = 0; i < num_vertices; ++i)
        for (int j = i + 1; j < num_vertices; ++j)
            if (adjacency_matrix[i][j] != 0)
                add_edge(i, j, adjacency_matrix[i][j], g);

    std::vector<graph_traits<Graph>::vertex_descriptor> predecessor(num_vertices);
    prim_minimum_spanning_tree(g, &predecessor[0]);

    int mst_adjacency_matrix[num_vertices][num_vertices] = {0};
    for (int i = 0; i < num_vertices; ++i) {
        if (predecessor[i] != i) {
            int parent = predecessor[i];
            auto [edge, exists] = edge(parent, i, g);
            if (exists) {
                int weight = get(edge_weight, g, edge);
                mst_adjacency_matrix[parent][i] = weight;
                mst_adjacency_matrix[i][parent] = weight;
            }
        }
    }

    std::cout << "Boost: MST adjacency matrix\n";
    for (int i = 0; i < num_vertices; ++i) {
        for (int j = 0; j < num_vertices; ++j)
            std::cout << mst_adjacency_matrix[i][j] << " ";
        std::cout << std::endl;
    }
    return 0;
}
-----------------------------------------------------------------------



2. LEMON (Library for Efficient Modeling and Optimization in Networks)
   – Алгоритм: lemon::prim
   – API: property maps (EdgeMap, NodeMap), алгоритм как функция

-----------------------------------------------------------------------
#include <iostream>
#include <vector>
#include <lemon/list_graph.h>
#include <lemon/prim.h>

int main() {
    const int n = 5;
    int adj_matrix[n][n] = {
        {0,2,0,6,0},{2,0,3,8,5},{0,3,0,0,7},{6,8,0,0,9},{0,5,7,9,0}
    };

    lemon::ListGraph g;
    std::vector<lemon::ListGraph::Node> nodes(n);
    for (int i = 0; i < n; ++i) nodes[i] = g.addNode();

    lemon::ListGraph::EdgeMap<int> weight(g);
    for (int i = 0; i < n; ++i)
        for (int j = i+1; j < n; ++j)
            if (adj_matrix[i][j] != 0)
                weight[g.addEdge(nodes[i], nodes[j])] = adj_matrix[i][j];

    lemon::ListGraph::NodeMap<lemon::ListGraph::Edge> tree_map(g);
    lemon::prim(g, weight, tree_map);

    std::vector<std::vector<int>> mst_matrix(n, std::vector<int>(n,0));
    for (lemon::ListGraph::NodeIt it(g); it != lemon::INVALID; ++it)
        if (tree_map[it] != lemon::INVALID) {
            int u = g.id(g.u(tree_map[it])), v = g.id(g.v(tree_map[it]));
            mst_matrix[u][v] = mst_matrix[v][u] = weight[tree_map[it]];
        }

    std::cout << "LEMON: MST adjacency matrix\n";
    for (auto& row : mst_matrix) {
        for (int w : row) std::cout << w << " ";
        std::cout << std::endl;
    }
    return 0;
}
-----------------------------------------------------------------------



3. CXXGraph (Header-only, modern C++17)
   – Алгоритм: g.prim() – метод класса графа
   – API: объектно-ориентированный, умные указатели, std::optional

-----------------------------------------------------------------------
#include <iostream>
#include <memory>
#include <vector>
#include "CXXGraph/Graph.h"
#include "CXXGraph/Edge.h"

int main() {
    const int n = 5;
    int adj_matrix[n][n] = {
        {0,2,0,6,0},{2,0,3,8,5},{0,3,0,0,7},{6,8,0,0,9},{0,5,7,9,0}
    };
    std::vector<std::shared_ptr<CXXGraph::Edge<int>>> edges;

    for (int i = 0; i < n; ++i)
        for (int j = i+1; j < n; ++j)
            if (adj_matrix[i][j] != 0)
                edges.push_back(std::make_shared<CXXGraph::UndirectedWeightedEdge<int>>(i, j, adj_matrix[i][j]));

    CXXGraph::Graph<int> g(edges);
    auto mst_result = g.prim();

    if (mst_result.success) {
        std::vector<std::vector<int>> mst_matrix(n, std::vector<int>(n,0));
        for (const auto& e : mst_result.mstEdges) {
            int u = e->getNodePair().first, v = e->getNodePair().second;
            mst_matrix[u][v] = mst_matrix[v][u] = e->getWeight().value();
        }
        std::cout << "CXXGraph: MST adjacency matrix\n";
        for (auto& row : mst_matrix) {
            for (int w : row) std::cout << w << " ";
            std::cout << std::endl;
        }
    }
    return 0;
}
-----------------------------------------------------------------------



4. OGDF (Open Graph Drawing Framework)
   – Алгоритм: makeMinimumSpanningTree (Kruskal, но результат тот же)
   – API: модульный, EdgeArray<bool>, граф ogdf::Graph

-----------------------------------------------------------------------
#include <iostream>
#include <vector>
#include <ogdf/basic/Graph.h>
#include <ogdf/basic/EdgeArray.h>
#include <ogdf/graphalg/MinimumSpanningTree.h>

int main() {
    const int n = 5;
    int adj_matrix[n][n] = {
        {0,2,0,6,0},{2,0,3,8,5},{0,3,0,0,7},{6,8,0,0,9},{0,5,7,9,0}
    };
    ogdf::Graph G;
    std::vector<ogdf::node> nodes(n);
    for (int i = 0; i < n; ++i) nodes[i] = G.newNode();

    ogdf::EdgeArray<double> weight(G);
    for (int i = 0; i < n; ++i)
        for (int j = i+1; j < n; ++j)
            if (adj_matrix[i][j] != 0)
                weight[G.newEdge(nodes[i], nodes[j])] = adj_matrix[i][j];

    ogdf::EdgeArray<bool> inMST(G);
    double totalWeight = ogdf::makeMinimumSpanningTree(G, weight, inMST);

    std::vector<std::vector<int>> mst_matrix(n, std::vector<int>(n,0));
    for (auto e : G.edges)
        if (inMST[e]) {
            int u = e->source()->index(), v = e->target()->index();
            mst_matrix[u][v] = mst_matrix[v][u] = weight[e];
        }

    std::cout << "OGDF: MST adjacency matrix\n";
    for (auto& row : mst_matrix) {
        for (int w : row) std::cout << w << " ";
        std::cout << std::endl;
    }
    return 0;
}
-----------------------------------------------------------------------



5. NetworKit (анализ больших сетей)
   – Алгоритм: PrimMSF
   – API: класс PrimMSF, методы run() / getEdges()

-----------------------------------------------------------------------
#include <iostream>
#include <vector>
#include <networkit/graph/Graph.hpp>
#include <networkit/graph/PrimMSF.hpp>
#include <networkit/auxiliary/Log.hpp>

int main() {
    Aux::Log::setLogLevel(Aux::LogLevel::LOG_OFF);
    const int n = 5;
    int adj_matrix[n][n] = {
        {0,2,0,6,0},{2,0,3,8,5},{0,3,0,0,7},{6,8,0,0,9},{0,5,7,9,0}
    };
    NetworKit::Graph G(n, true, false); // weighted, undirected
    for (int i = 0; i < n; ++i)
        for (int j = i+1; j < n; ++j)
            if (adj_matrix[i][j] != 0)
                G.addEdge(i, j, adj_matrix[i][j]);

    NetworKit::PrimMSF prim(G);
    prim.run();

    std::vector<std::vector<int>> mst_matrix(n, std::vector<int>(n,0));
    for (auto e : prim.getEdges()) {
        int u = e.first, v = e.second;
        mst_matrix[u][v] = mst_matrix[v][u] = G.weight(u, v);
    }

    std::cout << "NetworKit: MST adjacency matrix\n";
    for (auto& row : mst_matrix) {
        for (int w : row) std::cout << w << " ";
        std::cout << std::endl;
    }
    return 0;
}
-----------------------------------------------------------------------



6. igraph (библиотека на C, но с C++ обёрткой)
   – Алгоритм: igraph_minimum_spanning_tree_prim
   – API: структуры C, векторы, явное управление памятью

-----------------------------------------------------------------------
#include <iostream>
#include <vector>
#include <igraph/igraph.h>

int main() {
    const int n = 5;
    int adj_matrix[n][n] = {
        {0,2,0,6,0},{2,0,3,8,5},{0,3,0,0,7},{6,8,0,0,9},{0,5,7,9,0}
    };
    igraph_t g;
    igraph_vector_t edges, weights;

    igraph_vector_init(&edges, 0);
    igraph_vector_init(&weights, 0);
    for (int i = 0; i < n; ++i)
        for (int j = i+1; j < n; ++j)
            if (adj_matrix[i][j] != 0) {
                igraph_vector_push_back(&edges, i);
                igraph_vector_push_back(&edges, j);
                igraph_vector_push_back(&weights, adj_matrix[i][j]);
            }

    igraph_create(&g, &edges, n, IGRAPH_UNDIRECTED);
    igraph_vector_t result_edges;
    igraph_vector_init(&result_edges, 0);

    igraph_minimum_spanning_tree_prim(&g, &result_edges, &weights);

    std::vector<std::vector<int>> mst_matrix(n, std::vector<int>(n,0));
    for (int i = 0; i < igraph_vector_size(&result_edges) / 2; ++i) {
        int u = VECTOR(result_edges)[2*i], v = VECTOR(result_edges)[2*i+1];
        int w = adj_matrix[u][v];
        mst_matrix[u][v] = mst_matrix[v][u] = w;
    }

    std::cout << "igraph: MST adjacency matrix\n";
    for (auto& row : mst_matrix) {
        for (int w : row) std::cout << w << " ";
        std::cout << std::endl;
    }

    igraph_vector_destroy(&result_edges);
    igraph_vector_destroy(&weights);
    igraph_vector_destroy(&edges);
    igraph_destroy(&g);
    return 0;
}
-----------------------------------------------------------------------



7. LEDA (коммерческая библиотека)
   – Алгоритм: MIN_SPANNING_TREE
   – API: объектно-ориентированный, list<edge>, edge_array<int>

-----------------------------------------------------------------------
#include <iostream>
#include <vector>
#include <LEDA/graph/graph.h>
#include <LEDA/graph/min_span.h>

int main() {
    const int n = 5;
    int adj_matrix[n][n] = {
        {0,2,0,6,0},{2,0,3,8,5},{0,3,0,0,7},{6,8,0,0,9},{0,5,7,9,0}
    };
    leda::graph G;
    std::vector<leda::node> nodes(n);
    for (int i = 0; i < n; ++i) nodes[i] = G.new_node();

    leda::edge_array<int> cost(G);
    for (int i = 0; i < n; ++i)
        for (int j = i+1; j < n; ++j)
            if (adj_matrix[i][j] != 0)
                cost[G.new_edge(nodes[i], nodes[j])] = adj_matrix[i][j];

    leda::list<leda::edge> mst_edges = MIN_SPANNING_TREE(G, cost);

    std::vector<std::vector<int>> mst_matrix(n, std::vector<int>(n,0));
    for (leda::edge e : mst_edges) {
        int u = G.id(G.source(e)), v = G.id(G.target(e));
        mst_matrix[u][v] = mst_matrix[v][u] = cost[e];
    }

    std::cout << "LEDA: MST adjacency matrix\n";
    for (auto& row : mst_matrix) {
        for (int w : row) std::cout << w << " ";
        std::cout << std::endl;
    }
    return 0;
}
-----------------------------------------------------------------------

Конец файла.

*/

#include "incident/algorithms/mstPrim.hpp"
#include "incident/io/UndirectedGraphFromMatrix.hpp"
#include "incident/io/UndirectedGraphToMatrix.hpp"

#include <iostream>

using namespace exx::incident;

int main() {
    const int n = 5;
    int adj_matrix[n * n] = {
        0,2,0,6,0,
        2,0,3,8,5,
        0,3,0,0,7,
        6,8,0,0,9,
        0,5,7,9,0
    };

    auto G = buildUndirectedGraph(adj_matrix, n);

    if(!G) {
        std::cerr << to_string(G.error());
        return 1;
    }

    auto mstG = mstPrim(*G);

    if(!mstG) {
        std::cerr << to_string(mstG.error());
        return 1;
    }

    auto adjMatrix = toAdjacencyMatrix(*mstG);

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) std::cout << adjMatrix(i, j) << " ";
        std::cout << std::endl;
    }

    return 0;
}
