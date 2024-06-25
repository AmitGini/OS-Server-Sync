#include "GraphMatrix.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <pthread.h>

#define PORT "9034"

pthread_mutex_t graph_mutex = PTHREAD_MUTEX_INITIALIZER;
GraphMatrix* ptrGraph = nullptr;

void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int get_listener_socket(void) {
    int listener;
    int yes = 1;
    int rv;

    struct addrinfo hints, *ai, *p;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }

    for (p = ai; p != NULL; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0) {
            continue;
        }
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close(listener);
            continue;
        }
        break;
    }

    if (p == NULL) {
        return -1;
    }

    freeaddrinfo(ai);

    if (listen(listener, 10) == -1) {
        return -1;
    }

    return listener;
}

void add_to_pfds(struct pollfd *pfds[], int newfd, int *fd_count, int *fd_size) {
    if (*fd_count == *fd_size) {
        *fd_size *= 2;
        *pfds = (pollfd*)realloc(*pfds, sizeof(**pfds) * (*fd_size));
    }

    (*pfds)[*fd_count].fd = newfd;
    (*pfds)[*fd_count].events = POLLIN;
    (*fd_count)++;
}

void del_from_pfds(struct pollfd pfds[], int i, int *fd_count) {
    pfds[i] = pfds[*fd_count - 1];
    (*fd_count)--;
}

void handle_new_graph(int sender_fd, GraphMatrix* &ptrGraph, int n, int m){
    char buf[256];
    int numBytes;
    int count = m;

    while(count > 0) {
        std::string msg = "Enter edge: ";
        send(sender_fd, msg.c_str(), msg.size(), 0);

        memset(buf, 0, sizeof(buf)); // clear the buffer
        numBytes = recv(sender_fd, buf, sizeof(buf), 0);
        if(numBytes <= 0) {
            std::string msg = "Failed to receive data for edges\n";
            send(sender_fd, msg.c_str(), msg.size(), 0);
            return;
        }
        
        std::istringstream iss(std::string(buf, numBytes));
        int ver1, ver2;
        iss >> ver1 >> ver2;

        if (ver1 > 0 && ver2 > 0 && ver1 <= n && ver2 <= n) {
            
            ptrGraph->addEdge(ver1 - 1, ver2 - 1);
            count--;
        } else {
            std::string msg = "Invalid edge\n";
            send(sender_fd, msg.c_str(), msg.size(), 0);
            continue;
        }
    }
}

void handle_kosaraju(int sender_fd, GraphMatrix* &ptrGraph) {
    pthread_mutex_lock(&graph_mutex);
    std::vector<std::vector<int>> SCCs = ptrGraph->getSCCs();
    pthread_mutex_unlock(&graph_mutex);
    std::ostringstream oss;
    
    for (const auto &component : SCCs) {
        for (size_t i = 0; i < component.size(); i++) {
            oss << component[i] + 1;
            if (i != component.size() - 1) oss << " ";
        }
        oss << "\n";
    }
    std::string msg = oss.str();
    send(sender_fd, msg.c_str(), msg.size(), 0);
}

void* handle_client_message(void* arg) {
    int sender_fd = *(int*)arg;
    delete (int*)arg;
    char buf[256];
    int nbytes;

    while ((nbytes = recv(sender_fd, buf, sizeof(buf), 0)) > 0) {
        std::istringstream iss(std::string(buf, nbytes));
        std::string command;
        iss >> command;

        if (command == "Newgraph") {
            int n, m;
            if (iss >> n >> m && n > 0 && m > 0) {
                pthread_mutex_lock(&graph_mutex);
                delete ptrGraph;
                ptrGraph = new GraphMatrix(n);
                handle_new_graph(sender_fd, ptrGraph, n, m);
                pthread_mutex_unlock(&graph_mutex);
                
                std::string msg = "Graph created with " + std::to_string(n) + " vertices and " + std::to_string(m) + " edges\n";
                send(sender_fd, msg.c_str(), msg.size(), 0);
            } else {
                std::string msg = "Invalid command for Newgraph\n";
                send(sender_fd, msg.c_str(), msg.size(), 0);
            }

        } else if (command == "Kosaraju") {
            if (ptrGraph) {
                handle_kosaraju(sender_fd, ptrGraph);
            } else {
                std::string msg = "Graph not initialized.\n";
                send(sender_fd, msg.c_str(), msg.size(), 0);
            }

        } else if (command == "Newedge") {
            int i, j;
            if (iss >> i >> j && i > 0 && j > 0) {
                if (ptrGraph) {
                    pthread_mutex_lock(&graph_mutex);
                    ptrGraph->addEdge(i - 1, j - 1);
                    pthread_mutex_unlock(&graph_mutex);
                    std::string msg = "Edge added between " + std::to_string(i) + " and " + std::to_string(j) + "\n";
                    send(sender_fd, msg.c_str(), msg.size(), 0);
                } else {
                    std::string msg = "Graph not initialized.\n";
                    send(sender_fd, msg.c_str(), msg.size(), 0);
                }
            } else {
                std::string msg = "Invalid command for Newedge\n";
                send(sender_fd, msg.c_str(), msg.size(), 0);
            }

        } else if (command == "Removeedge") {
            int i, j;
            if (iss >> i >> j && i > 0 && j > 0) {
                if (ptrGraph) {
                    pthread_mutex_lock(&graph_mutex);
                    ptrGraph->removeEdge(i - 1, j - 1);
                    pthread_mutex_unlock(&graph_mutex);
                    std::string msg = "Edge removed between " + std::to_string(i) + " and " + std::to_string(j) + "\n";
                    send(sender_fd, msg.c_str(), msg.size(), 0);
                } else {
                    std::string msg = "Graph not initialized.\n";
                    send(sender_fd, msg.c_str(), msg.size(), 0);
                }
            } else {
                std::string msg = "Invalid command for Removeedge\n";
                send(sender_fd, msg.c_str(), msg.size(), 0);
            }

        } else if (command == "Exit") {
            std::string msg = "Goodbye\n";
            send(sender_fd, msg.c_str(), msg.size(), 0);
            break;

        } else {
            std::string msg = "Invalid command\n";
            send(sender_fd, msg.c_str(), msg.size(), 0);
        }
    }

    if (nbytes == 0) {
        printf("pollserver: socket %d hung up\n", sender_fd);
    } else {
        perror("recv");
    }

    close(sender_fd);
    return nullptr;
}

void* poll_thread_func(void* arg) {
    struct pollfd* pfds = (struct pollfd*)arg;
    int fd_count = 1;
    int fd_size = 5;

    for (;;) {
        int poll_count = poll(pfds, fd_count, -1);

        if (poll_count == -1) {
            perror("poll");
            exit(1);
        }

        for (int i = 0; i < fd_count; i++) {
            if (pfds[i].revents & POLLIN) {
                if (pfds[i].fd == pfds[0].fd) {  // New connection
                    struct sockaddr_storage remoteaddr;
                    socklen_t addrlen = sizeof remoteaddr;
                    int newfd = accept(pfds[0].fd, (struct sockaddr *)&remoteaddr, &addrlen);

                    if (newfd == -1) {
                        perror("accept");
                    } else {
                        add_to_pfds(&pfds, newfd, &fd_count, &fd_size);
                        char remoteIP[INET6_ADDRSTRLEN];
                        printf("pollserver: new connection from %s on socket %d\n",
                               inet_ntop(remoteaddr.ss_family, get_in_addr((struct sockaddr*)&remoteaddr), remoteIP, INET6_ADDRSTRLEN), newfd);
                        std::string startConversation = "**** Start chat ****:\n";
                        send(newfd, startConversation.c_str(), startConversation.size(), 0);
                    }
                } else {  // Existing connection
                    int sender_fd = pfds[i].fd;
                    int* client_fd = new int(sender_fd);
                    pthread_t client_thread;
                    pthread_create(&client_thread, nullptr, handle_client_message, client_fd);
                    pthread_detach(client_thread);

                    del_from_pfds(pfds, i, &fd_count);
                }
            }
        }
    }

    return nullptr;
}

int main(void) {
    int listener = get_listener_socket();
    if (listener == -1) {
        fprintf(stderr, "error getting listening socket\n");
        exit(1);
    }

    int fd_size = 5;
    struct pollfd* pfds = (pollfd*)malloc(sizeof *pfds * fd_size);
    pfds[0].fd = listener;
    pfds[0].events = POLLIN;

    pthread_t poll_thread;
    pthread_create(&poll_thread, nullptr, poll_thread_func, pfds);
    pthread_join(poll_thread, nullptr);

    free(pfds);
    pthread_mutex_destroy(&graph_mutex);

    return 0;
}
