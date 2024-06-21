#ifndef IOHANDLER_HPP
#define IOHANDLER_HPP

#include <vector>
#include "GraphList.hpp"
#include "GraphMatrix.hpp"

class IOHandler {
public:
    void readInput(GraphList &graphList);
    void readInput(GraphMatrix &graphMatrix);
    void printOutput(const std::vector<std::vector<int>> &SCCs);

};

#endif 
