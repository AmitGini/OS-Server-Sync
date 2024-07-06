#include "GraphMatrix.hpp"

#include <iostream> 
#include <vector>
#include <string> 
#include <sstream> // std::istringstream
#include <cstring> // memset
#include <cstdio> // perror
#include <cstdlib> // exit
#include <unistd.h> // close
#include <sys/types.h> // socket
#include <sys/socket.h> // socket
#include <netinet/in.h> // socket
#include <arpa/inet.h> // inet_ntop
#include <netdb.h> // getaddrinfo
#include <poll.h> // poll


#define PORT "9034"

bool shutdown_flag = false; // flag to indicate shutdown


// get pointer to sockaddr, and return pointer to IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr); // IPv4
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr); // IPv6
}

// create a listener socket
int get_listener_socket(void) {
    int listener;
    int yes = 1; // for setsockopt() SO_REUSEADDR
    int rv; // value

    struct addrinfo hints, *ai, *p; // ai = address info, p = pointer to address info

    memset(&hints, 0, sizeof hints); // clear the struct
    hints.ai_family = AF_UNSPEC; // use IPv4 or IPv6, whichever
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
    hints.ai_flags = AI_PASSIVE; // fill in the IP of the local host
    
    // get address info
    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) { // get address info for the server: NULL = local host, PORT = port number
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }
    // loop through all the results (ai = address info) and bind to the first we can use for the server
    for (p = ai; p != NULL; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol); // create socket
        if (listener < 0) {
            continue;
        }

        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)); // SOL_SOCKET = socket level, SO_REUSEADDR = reuse port, yes = true

        // bind socket to port
        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close(listener);
            continue;
        }
        break;
    }

    if (p == NULL) {
        return -1;
    }

    freeaddrinfo(ai); // free the address info

    // listen to the port for 10 incoming connections
    if (listen(listener, 10) == -1) {
        return -1;
    }
    return listener; // return the listener
}

// add a new file descriptor to the pollfd structure
void add_to_pfds(struct pollfd *pfds[], int newfd, int *fd_count, int *fd_size) {
    // if the array is full, double its size
    if (*fd_count == *fd_size) { 
        *fd_size *= 2;
        *pfds = (pollfd*)realloc(*pfds, sizeof(**pfds) * (*fd_size)); // reallocate memory for the array of pollfd structures 
    }

    (*pfds)[*fd_count].fd = newfd; // add the new file descriptor
    (*pfds)[*fd_count].events = POLLIN; // check for incoming data on the new file descriptor to read
    (*fd_count)++; // increment the number of file descriptors
}

// delete a file descriptor from the pollfd structure
void del_from_pfds(struct pollfd pfds[], int i, int *fd_count) {
    pfds[i] = pfds[*fd_count - 1];
    (*fd_count)--; // decrement the number of file descriptors
}

// handle the creation of a new graph 
void handle_new_graph(int sender_fd, struct pollfd& pfds,GraphMatrix* &ptrGraph, int n, int m){
    char buf[256]; // buffer for incoming data
    int numBytes; // number of bytes received
    int count = m; // number of edges to be added


    while(count > 0)
    {
        std::string msg = "Enter edge: ";
        send(sender_fd, msg.c_str(), msg.size(), 0); // send the message to the client

        memset(buf, 0, sizeof(buf)); // clear the buffer
        numBytes = recv(pfds.fd, buf, sizeof(buf), 0); // receive data from the client to the buffer
        if(numBytes <= 0){
            std::string msg = "Failed to receive data for edges\n";
            send(sender_fd, msg.c_str(), msg.size(), 0); 
            return;
        }
        
        std::istringstream iss(std::string(buf, numBytes)); // create an input string stream from the buffer
        int ver1, ver2;
        iss >> ver1 >> ver2; // extract the vertices from the input string stream

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

// handle the Kosaraju command, output the strongly connected components of the graph
void handle_kosaraju(int sender_fd, GraphMatrix* &ptrGraph) {

    std::vector<std::vector<int>> SCCs = ptrGraph->getSCCs(); // get the strongly connected components of the graph
    std::ostringstream oss; // create an output string stream
    
    for (const auto &component : SCCs) {
        for (size_t i = 0; i < component.size(); i++) {
            oss << component[i] + 1; // print the vertex
            if (i != component.size() - 1) oss << " "; // print a space if it is not the last vertex
        }
        oss << "\n";
    }
    std::string msg = oss.str(); // convert the output string stream to a string
    send(sender_fd, msg.c_str(), msg.size(), 0); // send the message to the client
}

// handle the client message and execute the command
void handle_client_message(int sender_fd, char *buf, int nbytes, struct pollfd& pfds, int fd_count, int listener, GraphMatrix* &ptrGraph, bool& shutdown_flag) {
    std::istringstream iss(std::string(buf, nbytes)); // create an input string stream from the buffer

    std::string command; // command to be executed
    
    iss >> command; // extract the command from the input string stream

    if (command == "Newgraph") {
        int n, m;
        
        if (iss >> n >> m && n > 0 && m > 0) {
            if(ptrGraph){
                delete ptrGraph;
            }
            ptrGraph = new GraphMatrix(n); // create a new graph with n vertices
            handle_new_graph(sender_fd, pfds, ptrGraph, n, m);
            
            std::string msg = "Graph created with " + std::to_string(n) + " vertices and " + std::to_string(m) + " edges\n";
            send(sender_fd, msg.c_str(), msg.size(), 0); 
        
        } else {
            std::string msg = "Invalid command for Newgraph\n";
            send(sender_fd, msg.c_str(), msg.size(), 0); 
        }

    } else if (command == "Kosaraju") {
        if(ptrGraph){
            handle_kosaraju(sender_fd, ptrGraph);
        } else {
            std::string msg = "Graph not initialized.\n";
            send(sender_fd, msg.c_str(), msg.size(), 0);
        }

    } else if (command == "Newedge") {
        int i, j;

        if (iss >> i >> j && i > 0 && j > 0) {
        
            if(ptrGraph){
                ptrGraph->addEdge(i - 1, j - 1);
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
                ptrGraph->removeEdge(i - 1, j - 1);
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
        close(sender_fd);
    
    } else if (command == "Shutdown") {
        std::string msg = "Server shutting down\n";
        send(sender_fd, msg.c_str(), msg.size(), 0);
        shutdown_flag = true; // set the flag to indicate shutdown
    
    } else {
        std::string msg = "Invalid command\n";
        send(sender_fd, msg.c_str(), msg.size(), 0);
    }
}


int main(void) {
    int listener; // listener socket
    int newfd; // new file descriptor
    struct sockaddr_storage remoteaddr; // client address (struct sockaddr_storage can hold both IPv4 and IPv6 addresses)
    socklen_t addrlen; // client address length
    char buf[256]; // buffer for incoming data
    char remoteIP[INET6_ADDRSTRLEN]; // client IP address (IPv6 address length = 46 bytes)
    int fd_count = 0; // number of file descriptors
    int fd_size = 5; // size of the file descriptor array
    struct pollfd *pfds = (pollfd*)malloc(sizeof *pfds * fd_size); // array of pollfd structures
    GraphMatrix* ptrGraph = nullptr; // pointer to the graph

    std::cout << "Server started.\n"<< std::endl;
    
    listener = get_listener_socket();
    if (listener == -1) {
        fprintf(stderr, "error getting listening socket\n");
        exit(1);
    }

    pfds[0].fd = listener; // add the listener to the pollfd structure
    pfds[0].events = POLLIN; // check for incoming data on the listener to  read
    fd_count = 1; // increment the number of file descriptors

    for (;;) {
        if (shutdown_flag) break; // check if the server is shutting down

        int poll_count = poll(pfds, fd_count, -1); // wait indefinitely for incoming data 

        if (poll_count == -1) {
            perror("poll");
            exit(1);
        }

        for (int i = 0; i < fd_count; i++) {
            if (pfds[i].revents & POLLIN) { // check if there is incoming data
                if (pfds[i].fd == listener) { // check if the incoming data is from the listener
                    addrlen = sizeof remoteaddr; // get the client address
                    newfd = accept(listener, (struct sockaddr *)&remoteaddr, &addrlen); // accept the incoming connection

                    if (newfd == -1) {
                        perror("accept");
                    } else {
                        add_to_pfds(&pfds, newfd, &fd_count, &fd_size);
                        printf("pollserver: new connection from %s on socket %d\n",
                        // inet_ntop: convert address from binary to text form
                        inet_ntop(remoteaddr.ss_family,
                                get_in_addr((struct sockaddr*)&remoteaddr),
                                remoteIP, INET6_ADDRSTRLEN),
                                newfd);
                        std::string startConversation = "**** Start chat ****:\n";
                        send(newfd, startConversation.c_str(), startConversation.size(), 0);
                    }
                } else { // incoming data is on other sockets
                    int nbytes = recv(pfds[i].fd, buf, sizeof(buf) , 0); 

                    int sender_fd = pfds[i].fd; 
                    if (nbytes <= 0) {
                        if (nbytes == 0) {
                            printf("pollserver: socket %d hung up\n", sender_fd);
                        } else {
                            perror("recv");
                        }

                        close(pfds[i].fd);
                        del_from_pfds(pfds, i, &fd_count);
                    } else {
                        handle_client_message(sender_fd, buf, nbytes, pfds[i], fd_count, listener, ptrGraph, shutdown_flag);
                        if (shutdown_flag) break; // check if the server is shutting down
                    }
                }
            }
        }
    }

    // clean up
    for (int i = 0; i < fd_count; i++) {
        close(pfds[i].fd);
    }

    if(ptrGraph){
        delete ptrGraph;
    }

    free(pfds);
    
    printf("Server shut down.\n");
    return 0;

}