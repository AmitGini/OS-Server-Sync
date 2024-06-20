#ifndef IOHANDLER_HPP
#define IOHANDLER_HPP

#include <vector>
#include "Graph.hpp"

class IOHandler {
public:
    void readInput(Graph &graph);
    void printOutput(const std::vector<std::vector<int>> &SCCs);
};

#endif 
