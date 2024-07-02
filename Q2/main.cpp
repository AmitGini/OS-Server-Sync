/*
make all  // compile the code
./SCC  // run exe
gprof SCC gmon.out > analysis.txt  // run time coverage and save the data in analysis text file
ls  //check analysis.txt has been created
cat analysis.txt  // file might be to large to review in the terminal
lines in the analysis, generating the graph:
void generateRandomGraph<GraphMatrix>(GraphMatrix&, int, int)
void generateRandomGraph<GraphList>(GraphList&, int, int)
line in the analysis, algorithm calculation time:
GraphMatrix::DFS(int, std::vector<bool, std::allocator<bool> >&, std::vector<int, std::allocator<int> >&)
GraphMatrix::getSCCs()
GraphMatrix::fillOrder(int, std::vector<bool, std::allocator<bool> >&, std::stack<int, std::deque<int, std::allocator<int> > >&)
GraphList::getSCCs()
GraphList::fillOrder(int, std::vector<bool, std::allocator<bool> >&, std::stack<int, std::deque<int, std::allocator<int> > >&)
and 

*/

#include "GraphList.hpp"
#include "GraphMatrix.hpp"


#include <iostream>
#include <cstdlib> // for srand and rand
#include <ctime> // for time
#include <chrono> // for high_resolution_clock

//Template from generation random graph, Matrix or List
template <typename GraphType>
void generateRandomGraph(GraphType &graph, int numVertices, int numEdges) {
    for (int i = 0; i < numVertices; ++i) {
        for (int w = 0; w < numVertices; ++w) {
            if (i != w) { // Avoid self-loops: graph is directed
                graph.addEdge(i, w);
            }
        }
    }
}


int main() {
    srand(static_cast<unsigned>(time(0))); // Seed the random number generator
    int numVertices = 1000;
    int numEdges = numVertices * (numVertices - 1); // Maximum number of edges in a directed graph without self-loops
    std::cout << "Generating a random graph with " << numVertices << " vertices and " << numEdges << " edges..." << std::endl;
    GraphList graphList(numVertices); // Create a graph with 0 vertices
    generateRandomGraph(graphList, numVertices, numEdges); // Generate a random graph

    // Measure time for GraphList implementation
    auto startList = std::chrono::high_resolution_clock::now();
    std::vector<std::vector<int>> SCCsList = graphList.getSCCs(); 
    auto endList = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> durationList = endList - startList; // Calculate the duration in seconds
    std::cout << "\nGraphList implementation took " << durationList.count() << " seconds.\n" << std::endl;
    
    GraphMatrix graphMatrix(numVertices); // Create a graph with 0 vertices
    generateRandomGraph(graphMatrix, numVertices, numEdges); // Generate a random graph

    // Measure time for GraphMatrix implementation
    auto startMatrix = std::chrono::high_resolution_clock::now();
    std::vector<std::vector<int>> SCCsMatrix = graphMatrix.getSCCs();
    auto endMatrix = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> durationMatrix = endMatrix - startMatrix; // Calculate the duration in seconds
    std::cout << "\nGraphMatrix implementation took " << durationMatrix.count() << " seconds.\n" << std::endl;

    // Compare the durations
    if(durationMatrix.count() < durationList.count())
        std::cout << "GraphMatrix implementation is faster." << std::endl;
    else
        std::cout << "GraphList implementation is faster." << std::endl;
    return 0;
}