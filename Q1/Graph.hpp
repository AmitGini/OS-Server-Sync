#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <list>
#include <vector>
#include <stack>


class Graph {
    int V; // Number of vertices
    std::vector<std::list<int>> adj; // Adjacency list for the original graph
    std::vector<std::list<int>> revAdj; // Adjacency list for the reversed graph

    void fillOrder(int v, std::vector<bool> &visited, std::stack<int> &Stack);
    void DFS(int v, std::vector<bool> &visited, std::vector<int> &component);

public:
    Graph(int V);
    void addEdge(int v, int w);
    std::vector<std::vector<int>> getSCCs();
};

#endif 
