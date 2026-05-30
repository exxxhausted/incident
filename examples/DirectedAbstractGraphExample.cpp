#include "incident/DirectedAbstractGraph.hpp"

#include <iostream>
#include <string>

using namespace exx::incident;

using Orgraph = DirectedAbstractGraph<std::string, int>;

void printV(const Orgraph& G) {
    std::cout << "Vertices: ";

    for(auto vPr : G.constVertices()) std::cout << vPr.data() << " ";

    std::cout << std::endl;
}

void printA(const Orgraph& G) {
    std::cout << "Arcs:" << std::endl;

    for(auto aPr : G.constArcs()) {
        std::cout << (*aPr.from()).data() << " -> " << (*aPr.to()).data();
        std::cout << " weight = " << aPr.data() << std::endl;
    }

    std::cout << std::endl;
}

void print(const Orgraph& G) { printV(G); printA(G); }

int main() {

    Orgraph graph;

    // Методы возвращают итераторы - универсальный механизм доступа к внутреннему миру графа.
    Orgraph::VertexIterator vA = graph.addVertex("A");

    auto vB = graph.emplaceVertex("B");
    auto vC = graph.addVertex("C");
    auto vD = graph.addVertex("D");

    graph.addArc(vA, vB, 5);

    // Да, есть итераторы и на дуги
    auto arc2 = graph.emplaceArc(vB, vC, 10);

    graph.addArc(vC, vD, 3);
    graph.addArc(vD, vA, 7);

    print(graph);

    // Переберем дуги, исходящие из B
    std::cout << "Outgoing arcs from B:" << std::endl;

    // Разыменование итератора создает фейковый легковесный объект, методы которого обеспечивают доступ
    // только к тем данным, изменение которых разрешено(ну или константные ссылки на что-то еще интересное)

    // Реализация "operator ->", к сожалению, затруднена

    for (auto aProxy : (*vB).adjacentArcs()) {
        std::cout << "  " << (*(aProxy.from())).data() << " -> " << (*aProxy.to()).data()
        << " weight " << aProxy.data() << std::endl;
    }
    std::cout << std::endl;

    // Удаление дуги
    graph.removeArc(arc2);

    std::cout << "After removing B->C:" << std::endl;
    printA(graph);

    // Удаление вершины
    graph.removeVertex(vC);

    std::cout << "After removing vertex C:" << std::endl;
    print(graph);

    return 0;
}
