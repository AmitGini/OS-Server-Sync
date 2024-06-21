#include "GraphList.hpp"

#include <vector>
#include <stack>
#include <algorithm>
#include <iostream>


GraphList::GraphList(int V) : V(V), adj(V), revAdj(V) {}

void GraphList::addEdge(int v, int w) {
    adj[v].push_back(w);
    revAdj[w].push_back(v);
}

void GraphList::fillOrder(int v, std::vector<bool> &visited, std::stack<int> &Stack) {
    visited[v] = true;

    for(auto i : adj[v])
        if(!visited[i])
            fillOrder(i, visited, Stack);

    Stack.push(v);
}

void GraphList::DFS(int v, std::vector<bool> &visited, std::vector<int> &component) {
    visited[v] = true;
    component.push_back(v);

    for(auto i : revAdj[v])
        if(!visited[i])
            DFS(i, visited, component);
}

// Function to return the strongly connected components of the graph using Kosaraju's algorithm
std::vector<std::vector<int>> GraphList::getSCCs() {
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