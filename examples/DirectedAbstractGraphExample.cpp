#include "incident/DirectedAbstractGraph.hpp"

#include <iostream>
#include <string>

using namespace exx::incident;

using Orgraph = DirectedAbstractGraph<std::string, int>;

void printV(const Orgraph& G) {
    std::cout << "Vertices: ";
    for (auto v : G.vertices()) std::cout << v.data() << " ";
    std::cout << std::endl;
}

void printA(const Orgraph& G) {
    std::cout << "Arcs:" << std::endl;
    for (auto a : G.arcs()) {
        std::cout << a.from().data() << " -> " << a.to().data()
                  << " weight = " << a.data() << std::endl;
    }
    std::cout << std::endl;
}

void print(const Orgraph& G) {
    printV(G);
    printA(G);
}

int main() {
    Orgraph graph;

    // Добавляем вершины – получаем дескрипторы
    auto vA = graph.addVertex("A");
    auto vB = graph.emplaceVertex("B");
    auto vC = graph.addVertex("C");
    auto vD = graph.addVertex("D");

    // Добавляем дуги
    graph.addArc(vA, vB, 5);
    auto arc2 = graph.emplaceArc(vB, vC, 10);
    graph.addArc(vC, vD, 3);
    graph.addArc(vD, vA, 7);

    print(graph);

    // Перебираем дуги, исходящие из B
    std::cout << "Outgoing arcs from B:" << std::endl;
    for (auto a : vB.adjacentArcs()) {
        // a.from() — дескриптор вершины-начала, a.to() — дескриптор конца
        std::cout << "  " << a.from().data() << " -> " << a.to().data()
                  << " weight " << a.data() << std::endl;
    }
    std::cout << std::endl;

    // Удаление дуги
    graph.removeArc(arc2);
    std::cout << "After removing B->C:" << std::endl;
    printA(graph);

    // Удаление вершины (автоматически удаляет все инцидентные дуги)
    graph.removeVertex(vC);
    std::cout << "After removing vertex C:" << std::endl;
    print(graph);

    return 0;
}
