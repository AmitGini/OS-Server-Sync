#include "GraphMatrix.hpp"
#include <vector>
#include <stack>
#include <algorithm>
#include <iostream>

// Constructor
GraphMatrix::GraphMatrix(int V) : V(V), adj(V, std::vector<int>(V, 0)), revAdj(V, std::vector<int>(V, 0)) {}

// Function to add an edge to the graph
void GraphMatrix::addEdge(int v, int w) {
    adj[v][w] = 1;
    revAdj[w][v] = 1;
}

// Function to fill the stack with vertices in increasing order of finishing times (post-order)
void GraphMatrix::fillOrder(int v, std::vector<bool> &visited, std::stack<int> &Stack) {
    visited[v] = true;

    for(int i = 0; i < V; ++i)
        if(adj[v][i] && !visited[i])
            fillOrder(i, visited, Stack);

    Stack.push(v);
}

// Function to perform DFS traversal of the reversed graph
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
    std::vector<std::vector<int>> SCCs; // Strongly connected components 

    // Process all vertices in order defined by Stack
    while(!Stack.empty()) {
        int v = Stack.top();
        Stack.pop(); 

        // If v is not visited yet, then it forms a new SCC
        if(!visited[v]) {
            std::vector<int> component;
            DFS(v, visited, component);
            std::sort(component.begin(), component.end());
            SCCs.push_back(component); // Add the component to the list of strongly connected components
        }
    }

    return SCCs;
}