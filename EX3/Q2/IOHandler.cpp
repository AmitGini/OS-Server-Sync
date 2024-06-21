#include "IOHandler.hpp"
#include "GraphList.hpp"

#include <vector>
#include <iostream>

void IOHandler::readInput(GraphList &graphList) {
    int V, E;
    std::cout<<"Enter the number of <vertices edges>:";
    try
    {
        std::cin >> V >> E;
        if(V <= 0 || E <= 0) throw std::exception();
    }catch (std::exception &e){
        std::cout<<"Invalid number - positive numbers required"<< std::endl;
        exit(1);
    }

    graphList = GraphList(V);

    for(int size = 0; size < E; size++) {
        int ver1, ver2;
        std::cout<<"Enter the edge vertics <ver ver>: ";
        try{
            std::cin >> ver1 >> ver2;
            if(ver1 <= 0 || ver2 <= 0) throw std::exception();
        }catch (std::exception &e){
            std::cout<<"Invalid number - positive numbers required"<< std::endl;
            exit(1);
        }
        graphList.addEdge(ver1 - 1, ver2 - 1); // Adjust for zero-based indexing
    }
}




void IOHandler::readInput(GraphMatrix &graphMatrix) {
    int V, E;
    std::cout << "Enter the number of <vertices edges>:";
    try {
        std::cin >> V >> E;
        if(V <= 0 || E <= 0) throw std::exception();
    } catch (std::exception &e) {
        std::cout << "Invalid number - positive numbers required" << std::endl;
        exit(1);
    }

    graphMatrix = GraphMatrix(V);

    for(int size = 0; size < E; size++) {
        int ver1, ver2;
        std::cout << "Enter the edge vertics <ver ver>: ";
        try {
            std::cin >> ver1 >> ver2;
            if(ver1 <= 0 || ver2 <= 0) throw std::exception();
        } catch (std::exception &e) {
            std::cout << "Invalid number - positive numbers required" << std::endl;
            exit(1);
        }
        graphMatrix.addEdge(ver1 - 1, ver2 - 1); // Adjust for zero-based indexing
    }
}

void IOHandler::printOutput(const std::vector<std::vector<int>> &SCCs) {
    if(SCCs.empty()) {
        std::cout << "No strongly connected components found." << std::endl;
        return;
    }

    for(const auto &component : SCCs) {
        for(size_t compIndex = 0; compIndex < component.size(); compIndex++) {
            std::cout << component[compIndex] + 1; // Adjust back to one-based indexing
            if(compIndex != component.size() - 1) std::cout << " ";
        }
        std::cout << std::endl;
    }
}
