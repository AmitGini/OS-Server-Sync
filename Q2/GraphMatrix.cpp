#include "GraphMatrix.hpp"
#include <vector>
#include <stack>
#include <algorithm>
#include <iostream>


GraphMatrix::GraphMatrix(int V) : V(V), adj(V, std::vector<int>(V, 0)), revAdj(V, std::vector<int>(V, 0)) {}

void GraphMatrix::addEdge(int v, int w) {
    adj[v][w] = 1;
    revAdj[w][v] = 1;
}

void GraphMatrix::fillOrder(int v, std::vector<bool> &visited, std::stack<int> &Stack) {
    visited[v] = true;

    for(int i = 0; i < V; ++i)
        if(adj[v][i] && !visited[i])
            fillOrder(i, visited, Stack);

    Stack.push(v);
}

void GraphMatrix::DFS(int v, std::vector<bool> &visited, std::vector<int> &component) {
    visited[v] = true;
    component.push_back(v);

    for(int i = 0; i < V; ++i)
        if(revAdj[v][i] && !visited[i])
            DFS(i, visited, component);
}

// Function to return the strongly connected components of the graph using Kosaraju's algorithm
std::vector<std::vector<int>> GraphMatrix::getSCCs() {
    std::stack<int> Stack;
    std::vector<bool> visited(V, false);

    for(int i = 0; i < V; i++)
        if(!visited[i])
            fillOrder(i, visited, Stack);

    std::fill(visited.begin(), visited.end(), false);
    std::vector<std::vector<int>> SCCs;

    while(!Stack.empty()) {
        int v = Stack.top();
        Stack.pop();

        if(!visited[v]) {
            std::vector<int> component;
            DFS(v, visited, component);
            std::sort(component.begin(), component.end());
            SCCs.push_back(component);
        }
    }

    return SCCs;
}
