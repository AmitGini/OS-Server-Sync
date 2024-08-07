#include "GraphMatrix.hpp"
#include "Reactor.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h> // for sockaddr_in
#include <arpa/inet.h> // for inet_ntop
#include <unistd.h> // for close
#include <netdb.h> // for gethostbyname

#define PORT "9034"

GraphMatrix* ptrGraph = nullptr; // Graph object to store the graph

// get sockaddr IPv4 or IPv6
void* get_in_addr(struct sockaddr* sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr); // IPv4
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr); // IPv6
}

int get_listener_socket(void) {
    int listener;
    int yes = 1;
    int rv;

    struct addrinfo hints, *ai, *p;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; //  AF_INET6 to force IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
    hints.ai_flags = AI_PASSIVE; // fill in my IP for me
    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) { // NULL means local host
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv)); // getaddrinfo error
        exit(1);
    }

    // loop through all the results and bind to the first we can
    for (p = ai; p != NULL; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0) {
            continue;
        }
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)); // SOL_SOCKET is the socket layer itself, SO_REUSEADDR allows other sockets to bind() to this port, unless there is an active listening socket bound to the port already

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close(listener);
            continue;
        }
        break;
    }

    if (p == NULL) {
        fprintf(stderr, "selectserver: failed to bind\n");
        return -1;
    }

    freeaddrinfo(ai);

    if (listen(listener, 10) == -1) {
        perror("listen");
        return -1;
    }

    return listener;
}

void handle_client_message(int fd) {
    try {
        char buf[256];
        int nbytes = recv(fd, buf, sizeof(buf), 0); // Wait for the client to send data
        if (nbytes <= 0) {
            if (nbytes == 0) {
                printf("pollserver: socket %d hung up\n", fd);
            } else {
                perror("recv");
            }
            close(fd); // Close the client socket
            removeFdFromReactor(fd); // Remove the client socket from the reactor 
        } else {
            std::istringstream iss(std::string(buf, nbytes));
            std::string command;
            iss >> command;

            if (command == "Newgraph") {
                int n, m;
                if ( iss >> n >> m && n > 0 && m > 0) {
                    if(ptrGraph) {
                       delete ptrGraph;
                    }
                    ptrGraph = new GraphMatrix(n);

                    for (int size = 0; size < m; size++) {
                        ssize_t bytes_sent = send(fd, "Enter edge: ", 13, 0);
                        if (bytes_sent == -1) {
                            throw std::runtime_error("Error sending message to client");
                        }

                        memset(buf, 0, sizeof(buf)); // Clear the buffer before receiving new data
                        nbytes = recv(fd, buf, sizeof(buf), 0); // Wait for the client to send edge data
                        if (nbytes > 0) {
                            std::istringstream edgeStream(std::string(buf, nbytes));
                            int ver1, ver2;
                            edgeStream >> ver1 >> ver2; // Parse the vertices from the received data

                            if (ver1 > 0 && ver2 > 0 && ver1 <= n && ver2 <= n) {
                                ptrGraph->addEdge(ver1 - 1, ver2 - 1);
                            } else {
                                std::string msg = "Invalid edge\n";
                                send(fd, msg.c_str(), msg.size(), 0);
                                return;
                            }
                        } else {
                            // Handle error or disconnection
                            std::string msg = "Error receiving edge or client disconnected\n";
                            send(fd, msg.c_str(), msg.size(), 0);
                            return;
                        }
                    }

                    std::string msg = "Graph created with " + std::to_string(n) + " vertices and " + std::to_string(m) + " edges\n";
                    send(fd, msg.c_str(), msg.size(), 0);
                } else {
                    std::string msg = "Invalid command for Newgraph\n";
                    send(fd, msg.c_str(), msg.size(), 0);
                }
            } else if (command == "Kosaraju") {
                if (ptrGraph) {
                    std::vector<std::vector<int>> SCCs = ptrGraph->getSCCs();
                    std::ostringstream oss;
                    for (const auto& component : SCCs) {
                        for (size_t i = 0; i < component.size(); i++) {
                            oss << component[i] + 1;
                            if (i != component.size() - 1) oss << " ";
                        }
                        oss << "\n";
                    }
                    std::string msg = oss.str();
                    send(fd, msg.c_str(), msg.size(), 0);
                } else {
                    std::string msg = "Graph not initialized.\n";
                    send(fd, msg.c_str(), msg.size(), 0);
                }
            } else if (command == "Newedge") {
                int i, j;
                if (iss >> i >> j && i > 0 && j > 0) {
                    if (ptrGraph) {
                        ptrGraph->addEdge(i - 1, j - 1);
                        std::string msg = "Edge added between " + std::to_string(i) + " and " + std::to_string(j) + "\n";
                        send(fd, msg.c_str(), msg.size(), 0);
                    } else {
                        std::string msg = "Graph not initialized.\n";
                        send(fd, msg.c_str(), msg.size(), 0);
                    }
                } else {
                    std::string msg = "Invalid command for Newedge\n";
                    send(fd, msg.c_str(), msg.size(), 0);
                }
            } else if (command == "Removeedge") {
                int i, j;
                if (iss >> i >> j && i > 0 && j > 0) {
                    if (ptrGraph) {
                        ptrGraph->removeEdge(i - 1, j - 1);
                        std::string msg = "Edge removed between " + std::to_string(i) + " and " + std::to_string(j) + "\n";
                        send(fd, msg.c_str(), msg.size(), 0);
                    } else {
                        std::string msg = "Graph not initialized.\n";
                        send(fd, msg.c_str(), msg.size(), 0);
                    }
                } else {
                    std::string msg = "Invalid command for Removeedge\n";
                    send(fd, msg.c_str(), msg.size(), 0);
                }
            } else if (command == "Exit") {
                std::string msg = "Goodbye\n";
                send(fd, msg.c_str(), msg.size(), 0);
                close(fd);
                removeFdFromReactor(fd);

            } else if (command == "shutdown") {
                std::cerr << "Shutdown command received, stopping server" << std::endl;
                removeFdFromReactor(fd);
                close(fd);
                stopReactor();
                return;
            
            } else {
                std::string msg = "Invalid command\n";
                send(fd, msg.c_str(), msg.size(), 0);
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception in handle_client_message: " << e.what() << std::endl;
        close(fd);
        removeFdFromReactor(fd);
    }
}

void start_chat(int client_fd) {
    // First message to the client
    std::string startConversation = "**** Start chat ****:\n";
    ssize_t bytes_sent = send(client_fd, startConversation.c_str(), startConversation.size(), 0);
    if (bytes_sent == -1) {
        perror("send");
        close(client_fd);
    } else {
        // Test check that message has arrived
        printf("Sent start message to client on socket %d\n", client_fd);
        // In the main function, where client sockets are added to the reactor
        if (addFdToReactor(client_fd, handle_client_message) == -1) {
            fprintf(stderr, "Failed to add client socket to reactor\n");
            close(client_fd);
        }
    }
}

int main(void) {
    int listener = get_listener_socket();
    if (listener == -1) {
        fprintf(stderr, "error getting listening socket\n");
        exit(1);
    }
    printf("Listener socket created: %d\n", listener);

    startReactor();
    printf("Reactor started\n");

    if (addFdToReactor(listener, [](int fd) {
        struct sockaddr_storage remoteaddr;
        socklen_t addrlen = sizeof(remoteaddr);
        int listener = fd;
        int client_fd = accept(listener, (struct sockaddr*)&remoteaddr, &addrlen);
        if (client_fd == -1) {
            perror("accept");
        } else {
            // Test check printing
            char remoteIP[INET6_ADDRSTRLEN];
            printf("pollserver: new connection from %s on socket %d\n",
                   inet_ntop(remoteaddr.ss_family, get_in_addr((struct sockaddr*)&remoteaddr), remoteIP, INET6_ADDRSTRLEN), client_fd);

            start_chat(client_fd);
        }
    }) == -1) {
        fprintf(stderr, "Failed to add listener to reactor\n");
        exit(1);
    }
    printf("Listener socket added to reactor\n");

    try {
        std::string input;
        while (std::getline(std::cin, input)) {
            if (input == "Exit") {
                break;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    // Main loop to keep the program running
    while (running) {
        sleep(1); // Sleep for 1 second
    }

    close(listener);
    std::cout << "Server has been shut down." << std::endl;

    return 0;
}
