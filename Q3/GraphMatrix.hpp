#ifndef GRAPHMATRIX_HPP
#define GRAPHMATRIX_HPP

#include <vector>
#include <stack>

// Graph class represented by an adjacency matrix
class GraphMatrix {
    int V; // Number of vertices
    std::vector<std::vector<int>> adj; // Adjacency matrix for the original graph
    std::vector<std::vector<int>> revAdj; // Adjacency matrix for the reversed graph

    void fillOrder(int v, std::vector<bool> &visited, std::stack<int> &Stack); // Fill the stack with vertices in increasing order of finishing times
    void DFS(int v, std::vector<bool> &visited, std::vector<int> &component); // Depth-first search traversal of the graph

public:
    GraphMatrix(int V); // Constructor
    int getSizeV() const; // Return the number of vertices
    void addEdge(int v, int w); // Add an edge to the graph
    void removeEdge(int v, int w); // Remove an edge from the graph
    std::vector<std::vector<int>> getSCCs(); // Return the strongly connected components of the graph
};

#endif