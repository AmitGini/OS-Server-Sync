#include "GraphMatrix.hpp"
#include <vector>
#include <stack>
#include <algorithm>
#include <iostream>


GraphMatrix::GraphMatrix(int V) : V(V), adj(V, std::vector<int>(V, 0)), revAdj(V, std::vector<int>(V, 0)) {}

int GraphMatrix::getSizeV() const
{
    return V;
}

bool GraphMatrix::addEdge(int v, int w) {
    if(V == 0 || v >= V || w >= V)
    {
        std::cout << "Invalid edge" << std::endl;
        return false;
    }
    adj[v][w] = 1;
    revAdj[w][v] = 1;
    return true;
}

bool GraphMatrix::removeEdge(int v, int w)
{
    if(V == 0 || v >= V || w >= V)
    {
        std::cout << "Invalid edge" << std::endl;
        return false;
    }
    adj[v][w] = 0;
    revAdj[w][v] = 0;
    return true;
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
    if(V == 0) return {};
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
