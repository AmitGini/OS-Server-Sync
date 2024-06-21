#ifndef GRAPHLIST_HPP
#define GRAPHLIST_HPP

#include <list>
#include <vector>
#include <stack>


//Graph class represented by adjacency list
class GraphList {
    int V; // Number of vertices
    std::vector<std::list<int>> adj; // Adjacency list for the original graph
    std::vector<std::list<int>> revAdj; // Adjacency list for the reversed graph

    void fillOrder(int v, std::vector<bool> &visited, std::stack<int> &Stack);
    void DFS(int v, std::vector<bool> &visited, std::vector<int> &component);

public:
    GraphList(int V);
    void addEdge(int v, int w);
    std::vector<std::vector<int>> getSCCs();
};

#endif 
