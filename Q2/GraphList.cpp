#include "GraphList.hpp"

#include <vector>
#include <stack>
#include <algorithm>
#include <iostream>

GraphList::GraphList(int V) : V(V), adj(V), revAdj(V) {}

void GraphList::addEdge(int v, int w) {
    adj[v].push_back(w); // Add w to v's list (add an arc from v to w)
    revAdj[w].push_back(v); // Add v to w's list (add an arc from w to v)
}

// Fill the stack with vertices in increasing order of finishing times (post-order)
void GraphList::fillOrder(int v, std::vector<bool> &visited, std::stack<int> &Stack) {
    visited[v] = true;

    // Recur for all the vertices adjacent to this vertex
    for(auto i : adj[v])
        if(!visited[i])
            fillOrder(i, visited, Stack);

    Stack.push(v); // All vertices reachable from v are processed by now
}

// DFS traversal of the reversed graph and identification of the strongly connected components
void GraphList::DFS(int v, std::vector<bool> &visited, std::vector<int> &component) {
    visited[v] = true;
    component.push_back(v); // Add the vertex to the current component

    // Recur for all the vertices adjacent to this vertex
    for(auto i : revAdj[v])
        if(!visited[i])
            DFS(i, visited, component);
}

// Get the strongly connected components
std::vector<std::vector<int>> GraphList::getSCCs() {
    std::stack<int> Stack;
    std::vector<bool> visited(V, false); // Mark all the vertices as not visited

    // Fill vertices in stack according to their finishing times
    for(int i = 0; i < V; i++)
        if(!visited[i])
            fillOrder(i, visited, Stack); // Fill Stack with vertices in increasing order of finishing times

    std::fill(visited.begin(), visited.end(), false); // Mark all the vertices as not visited
    std::vector<std::vector<int>> SCCs; // Strongly connected components

    // Process all vertices in order defined by Stack
    while(!Stack.empty()) {
        int v = Stack.top();
        Stack.pop();

        // If v is not visited yet, then it forms a new SCC
        if(!visited[v]) {
            std::vector<int> component;
            DFS(v, visited, component);
            std::sort(component.begin(), component.end()); // Sort the component in ascending order
            SCCs.push_back(component);
        }
    }

    return SCCs; // Return the strongly connected components
}