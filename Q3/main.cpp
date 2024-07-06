#include "GraphMatrix.hpp"
#include "IOHandler.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <limits>
#include <exception>
#include <stdexcept>
#include <sstream>

int main() {
    IOHandler ioHandler;
    GraphMatrix* ptrGraph = nullptr;

    for (;;) {

        std::cout << "Commands:" << std::endl;
        std::cout << "Newgraph n m" << std::endl;
        std::cout << "Kosaraju" << std::endl;
        std::cout << "Newedge i j" << std::endl;
        std::cout << "Removeedge i j" << std::endl;
        std::cout << "Exit\n" << std::endl;
        std::cout << "Your choice: ";
        std::string input;
        std::getline(std::cin, input); // Get the entire line of input

        if (input.size() >= 12 && input.substr(0, 8) == "Newgraph") {
            int n, m;

            try 
            {
                std::istringstream iss(input.substr(9)); // Skip the "Newgraph" part
                iss >> n >> m; // Extract the integers
                if (n <= 0 || m <= 0) throw std::exception();
            } 
            catch (std::exception &e) 
            {
                std::cout << "Invalid number - positive integers required" << std::endl;
                continue;
            }

            delete ptrGraph;
            ptrGraph = new GraphMatrix(n);
            ioHandler.readInput(*ptrGraph, m);
            // Clear the buffer to handle any leftover input
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); 
        } 

        else if (input == "Kosaraju") 
        {
            if (ptrGraph) 
            {
                std::vector<std::vector<int>> SCCs = ptrGraph->getSCCs();
                ioHandler.printOutput(SCCs);
            } 
            else 
            {
                std::cout << "Graph not initialized." << std::endl;
            }
        } 

        else if (input.substr(0, 7) == "Newedge") 
        {
            int i, j;
            std::istringstream iss(input.substr(8));
            try 
            {
                iss >> i >> j; // Extract the integers from the input
                if (i <= 0 || j <= 0) throw std::exception();
            } 
            catch (std::exception &e) 
            {
                std::cout << "Invalid number - positive integers required" << std::endl;
                continue;
            }

            if (ptrGraph) 
            {
                ptrGraph->addEdge(i - 1, j - 1); // Adjust for zero-based indexing
            } 
            else 
            {
                std::cout << "Graph not initialized." << std::endl;
            }
            // Clear the buffer to handle any leftover input
        } 
        
        else if (input.substr(0, 10) == "Removeedge") 
        {
            int i, j;
            std::istringstream iss(input.substr(11));
            
            try 
            {
                iss >> i >> j;
                if (i <= 0 || j <= 0) throw std::exception();
            } 
            catch (std::exception &e) 
            {
                std::cout << "Invalid number - positive integers required" << std::endl;
                continue;
            }
            
            if (ptrGraph) 
            {
                ptrGraph->removeEdge(i - 1, j - 1);  // Adjust for zero-based indexing
            } 
            else 
            {
                std::cout << "Graph not initialized." << std::endl;
            }
            // Clear the buffer to handle any leftover input
        } 

        else if (input == "Exit") 
        {
            delete ptrGraph;
            return 0;
        } 
        else 
        {
            std::cout << "Invalid command" << std::endl;
        }
        

    }
    return 0;
}