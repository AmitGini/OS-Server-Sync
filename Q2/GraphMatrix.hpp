#ifndef GRAPHMATRIX_HPP
#define GRAPHMATRIX_HPP

#include <vector>
#include <stack>

// Graph class represented by an adjacency matrix
class GraphMatrix {
    int V; // Number of vertices
    std::vector<std::vector<int>> adj; // Adjacency matrix for the original graph
    std::vector<std::vector<int>> revAdj; // Adjacency matrix for the reversed graph

    // Fill the stack with vertices in increasing order of finishing times (post-order)
    void fillOrder(int v, std::vector<bool> &visited, std::stack<int> &Stack);

    // DFS traversal of the reversed graph
    void DFS(int v, std::vector<bool> &visited, std::vector<int> &component);

public:
    GraphMatrix(int V); // Constructor
    void addEdge(int v, int w); // Add an edge to the graph
    std::vector<std::vector<int>> getSCCs(); // Get the strongly connected components
};

#endif