#ifndef GRAPHMATRIX_HPP
#define GRAPHMATRIX_HPP

#include <vector>
#include <stack>

// Graph class represented by an adjacency matrix
class GraphMatrix {
    int V; // Number of vertices
    std::vector<std::vector<int>> adj; // Adjacency matrix for the original graph
    std::vector<std::vector<int>> revAdj; // Adjacency matrix for the reversed graph

    void fillOrder(int v, std::vector<bool> &visited, std::stack<int> &Stack);
    void DFS(int v, std::vector<bool> &visited, std::vector<int> &component);

public:
    GraphMatrix(int V);
    void addEdge(int v, int w);
    std::vector<std::vector<int>> getSCCs();
};

#endif
