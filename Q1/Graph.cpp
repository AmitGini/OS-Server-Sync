#include "Graph.hpp"

#include <vector>
#include <stack>
#include <algorithm>
#include <iostream>

Graph::Graph(int V) : V(V), adj(V), revAdj(V) {}

void Graph::addEdge(int v, int w) {
    adj[v].push_back(w);
    revAdj[w].push_back(v);
}

void Graph::fillOrder(int v, std::vector<bool> &visited, std::stack<int> &Stack) {
    visited[v] = true;

    for(auto i : adj[v])
        if(!visited[i])
            fillOrder(i, visited, Stack);

    Stack.push(v);
}

void Graph::DFS(int v, std::vector<bool> &visited, std::vector<int> &component) {
    visited[v] = true;
    component.push_back(v);

    for(auto i : revAdj[v])
        if(!visited[i])
            DFS(i, visited, component);
}

std::vector<std::vector<int>> Graph::getSCCs() {
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