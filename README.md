# EX3 Server and Synchronization with Kosaraju Algorithm

This project implements a server that can handle multiple clients to manipulate graphs and compute the Strongly Connected Components (SCCs) using the Kosaraju-Sharir algorithm. The server allows clients to create new graphs, add or remove edges, and run the Kosaraju algorithm to get SCCs. The server is implemented using Beej chat based on poll, also implemented Proactor and Reactor design patterns to manage multiple client connections and manage the the clients requests efficiently.
The clients use shared memory, it means there is only 1 graph that can be edit or replaced, therefore the clients requests is processed using mutex.
the project implemented in parts, therefore **Q10** is the final implemntations.

## Features

- **Graph Manipulation**: Clients can create new graphs, add edges, remove edges, print the SCC.
- **Kosaraju Algorithm**: Computes and returns the SCCs of the graph.
- **Multi-client Support**: Handles multiple clients simultaneously using Proactor and Reactor patterns.
- **SCC Notification**: Notifies when at least 50% of the graph belongs to the same SCC.

## Getting Started

### Prerequisites

- Linux-based OS
- GCC compiler
- POSIX Threads (pthread) library

### Installation

1. Clone the repository:

    ```sh
    git clone https://github.com/yourusername/EX3-Server-Sync-Kosaraju.git
    cd EX3-Server-Sync-Kosaraju/Q10
    ```

2. Compile the project:

    ```sh
    make all
    ```

### Running the Server

1. Start the server:

    ```sh
    ./server
    ```

2. The server listens on port 9034 by default. Clients can connect to this port to send commands.

### Client Commands

- **Newgraph n m**: Create a new graph with `n` vertices and `m` edges. Followed by `m` lines of edges (e.g., `1 2`).

    ```sh
    Newgraph 5 5
    1 2
    2 3
    3 1
    3 4
    4 5
    ```

- **Kosaraju**: Compute and return the SCCs of the current graph.

    ```sh
    Kosaraju
    ```

- **Newedge i j**: Add a new edge from vertex `i` to vertex `j`.

    ```sh
    Newedge 1 3
    ```

- **Removeedge i j**: Remove the edge from vertex `i` to vertex `j`.

    ```sh
    Removeedge 1 2
    ```

- **Exit**: Disconnect the client from the server.

    ```sh
    Exit
    ```

## Code Overview

- **main.cpp**: Contains the main function, signal handlers, and server loop.
- **Proactor.hpp / Proactor.cpp**: Implements the Proactor pattern to accept client connections.
- **Reactor.hpp / Reactor.cpp**: Implements the Reactor pattern to handle client requests.
- **GraphMatrix.hpp / GraphMatrix.cpp**: Implements the graph and Kosaraju algorithm.

## Example

1. Start the server:

    ```sh
    ./server
    ```

2. Connect a client (e.g., using `telnet`):

    ```sh
    telnet localhost 9034
    ```

3. Send commands to the server through the client:

    ```sh
    Newgraph 5 5
    1 2
    2 3
    3 1
    3 4
    4 5

    Kosaraju

    Newedge 5 1

    Removeedge 3 4

    Exit
    ```

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Inspired by the Kosaraju-Sharir algorithm for finding SCCs.
- Uses POSIX Threads for multi-threading support.
- Reactor and Proactor design patterns for efficient I/O handling.
