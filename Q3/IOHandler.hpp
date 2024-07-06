#ifndef IOHANDLER_HPP
#define IOHANDLER_HPP

#include <vector>
#include "GraphMatrix.hpp"

class IOHandler {
public:
    void readInput(GraphMatrix &graphMatrix, int m);
    void printOutput(const std::vector<std::vector<int>> &SCCs);
};

#endif 