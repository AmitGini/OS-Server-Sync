#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <list>
#include <vector>
#include <stack>


class GraphList {
    int V; // Number of vertices
    std::vector<std::list<int>> adj; // Adjacency list for the original graph
    std::vector<std::list<int>> revAdj; // Adjacency list for the reversed graph

    // Fill the stack with vertices in increasing order of finishing times (post-order)
    void fillOrder(int v, std::vector<bool> &visited, std::stack<int> &Stack);

    // DFS traversal of the reversed graph
    void DFS(int v, std::vector<bool> &visited, std::vector<int> &component);

public:
    GraphList(int V);
    void addEdge(int v, int w); // Add an edge to the graph
    std::vector<std::vector<int>> getSCCs(); // Get the strongly connected components
};

#endif 