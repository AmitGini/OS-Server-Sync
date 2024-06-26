#include "GraphList.hpp"
#include "GraphMatrix.hpp"
#include "IOHandler.hpp"
#include <iostream>
#include <chrono>

int main() {
    IOHandler ioHandler;
    
    // Measure time for GraphList implementation
    auto startList = std::chrono::high_resolution_clock::now();
    
    GraphList graphList(0);
    ioHandler.readInput(graphList);
    std::vector<std::vector<int>> SCCsList = graphList.getSCCs();
    ioHandler.printOutput(SCCsList);
    
    auto endList = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> durationList = endList - startList;
    std::cout << "\nGraphList implementation took " << durationList.count() << " seconds.\n" << std::endl;
    
    // Measure time for GraphMatrix implementation
    auto startMatrix = std::chrono::high_resolution_clock::now();
    
    GraphMatrix graphMatrix(0);
    ioHandler.readInput(graphMatrix);
    std::vector<std::vector<int>> SCCsMatrix = graphMatrix.getSCCs();
    ioHandler.printOutput(SCCsMatrix);
    
    auto endMatrix = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> durationMatrix = endMatrix - startMatrix;
    std::cout << "\nGraphMatrix implementation took " << durationMatrix.count() << " seconds.\n" << std::endl;

    if(durationMatrix.count() < durationList.count())
        std::cout << "GraphMatrix implementation is faster." << std::endl;
    else
        std::cout << "GraphList implementation is faster." << std::endl;
    return 0;
}
