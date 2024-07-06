#include "IOHandler.hpp"

#include <vector>
#include <iostream>

void IOHandler::readInput(GraphMatrix &graphMatrix, int m) {
    
    for (int size = 0; size < m; size++) {
        int ver1, ver2;
        std::cout << "Enter the edge vertices <ver ver>: ";
        try {
            std::cin >> ver1 >> ver2;
            if (ver1 <= 0 || ver2 <= 0) throw std::exception();
        } catch (std::exception &e) {
            std::cout << "Invalid number - positive numbers required" << std::endl;
            exit(1);
        }
        graphMatrix.addEdge(ver1 - 1, ver2 - 1); // Adjust for zero-based indexing
    }
}


void IOHandler::printOutput(const std::vector<std::vector<int>> &SCCs) {
    if (SCCs.empty()) {
        std::cout << "No strongly connected components found." << std::endl;
        return;
    }

    for (const auto &component : SCCs) {
        for (size_t compIndex = 0; compIndex < component.size(); compIndex++) {
            std::cout << component[compIndex] + 1; // Adjust back to one-based indexing
            // Print a space after all but the last element
            if (compIndex != component.size() - 1) std::cout << " ";
        }
        std::cout << std::endl;
    }
}