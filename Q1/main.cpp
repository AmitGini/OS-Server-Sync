#include "Graph.hpp"
#include "IOHandler.hpp"
#include <iostream>

int main() {
    IOHandler ioHandler;
    Graph graph(0);
    ioHandler.readInput(graph);
    std::vector<std::vector<int>> SCCs = graph.getSCCs();
    ioHandler.printOutput(SCCs);
    return 0;
}