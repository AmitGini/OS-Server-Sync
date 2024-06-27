#include "Reactor.hpp"
#include "Proactor.hpp"
#include "GraphMatrix.hpp"
#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>
#include <csignal>
#include <pthread.h>
#include <sstream>


GraphMatrix* ptrGraph = nullptr;

constexpr size_t PORT = 9034;
int stop_server = 0; // Define stop_server

pthread_mutex_t graph_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t graph_cond = PTHREAD_COND_INITIALIZER;

bool hadComponentWithHalfVertices = false;
bool needToUpdate = false;
bool aboveHalf = false;

void* notify_SCC_signification_change(void* arg) {
    while (true) {
        pthread_mutex_lock(&graph_mutex);

        while (!needToUpdate) {

            pthread_cond_wait(&graph_cond, &graph_mutex);
        }

        needToUpdate = false;
        pthread_mutex_unlock(&graph_mutex);
        
        if(aboveHalf){
            std::cout<<"At Least 50% of the graph belongs to the same SCC"<<std::endl;
        }else{
            std::cout<<"At Least 50% of the graph no longer belongs to the same SCC"<<std::endl;
        }
    }

    return nullptr;
}

void handle_sigint(int sig) {
    stop_server = 1;
}

// Function to handle client communication
void* handleClient(int client_fd) {
    char buffer[1024];
    int bytes_received;
    

    bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    
    if (bytes_received <= 0) {
        if (bytes_received == 0) {
            std::cout << "Client disconnected, fd: " << client_fd << std::endl;
            removeFdFromReactor(client_fd); // Remove client from Reactor
            close(client_fd);
            return nullptr;

        } else {
            perror("recv");
            return nullptr;
        }
    }
    
    buffer[bytes_received] = '\0';
    std::string message = buffer;
    std::cout << "Received: " << buffer;

    std::string response = "Server received your message: ";
    response += message;
    send(client_fd, response.c_str(), response.size(), 0);

    
    return nullptr;
}

bool handle_new_graph(int sender_fd, int n, int m){
    try{
        char buf[256];
        int numBytes;
        int count = m;

        while(count > 0) {
            std::string msg = "Enter edge: ";
            send(sender_fd, msg.c_str(), msg.size(), 0);

            memset(buf, 0, sizeof(buf)); // clear the buffer
            numBytes = recv(sender_fd, buf, sizeof(buf), 0);
            
            if(numBytes < 0) {
                memset(buf, 0, sizeof(buf)); // clear the buffer
                std::string errMsg = "Failed to receive data for edges";
                send(sender_fd, errMsg.c_str(), errMsg.size(), 0);
                throw  std::runtime_error("Failed to receive data for edges - Adding edge has Stopped");
            
            }else if(numBytes == 0){
                throw std::runtime_error("Connection closed by client when creating graph");
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
        return true;
    }
    catch(std::exception &e){
        std::cout<<"\nException in handle_new_graph: "<<e.what()<<std::endl;
        return false;
    }
    catch(...){
        std::cout<<"\nException in handle_new_graph"<<std::endl;
        return false;
    }
}

void handle_kosaraju(int sender_fd) {
    std::vector<std::vector<int>> SCCs = ptrGraph->getSCCs();
    std::ostringstream oss;

    int numVertices = ptrGraph->getSizeV(); 
    bool containsHalfVertices = false;
    bool isEven = numVertices % 2 == 0;
    int count = 0;
    
    for (const auto &component : SCCs) {
        count = 0;
        for (size_t i = 0; i < component.size(); i++) {
            oss << component[i] + 1;
            if (i != component.size() - 1) oss << " ";
            
            count++;

            if(isEven && count >= numVertices/2 && !containsHalfVertices){
                containsHalfVertices = true;

            }else if(count > numVertices/2 && !containsHalfVertices){
                containsHalfVertices = true;
            }
        }
        oss << "\n";
    }

    std::string msg = oss.str();
    send(sender_fd, msg.c_str(), msg.size(), 0);

    if (containsHalfVertices && !hadComponentWithHalfVertices) {
        hadComponentWithHalfVertices = true;
        needToUpdate = true;
        pthread_cond_signal(&graph_cond);  // Signal the condition variable
    
    } else if (!containsHalfVertices && hadComponentWithHalfVertices) {
        hadComponentWithHalfVertices = false;
        needToUpdate = true;
        aboveHalf = false;
        pthread_cond_signal(&graph_cond);  // Signal the condition variable
    }
}

void* handle_client_message(int sender_fd) {
    try{
        char buf[256];
        int nbytes = recv(sender_fd, buf, sizeof(buf), 0);

        if (nbytes > 0) {
            std::istringstream iss(std::string(buf, nbytes));
            std::string command;
            iss >> command;

            if (command == "Newgraph") {
                int n, m;
                if (iss >> n >> m && n > 0 && m > 0) {
                    pthread_mutex_lock(&graph_mutex);
                    delete ptrGraph;
                    ptrGraph = new GraphMatrix(n);
                    bool hasAddedSucceefully = handle_new_graph(sender_fd, n, m);
                    pthread_mutex_unlock(&graph_mutex);

                    if(!hasAddedSucceefully){
                        throw std::runtime_error("Failed to create graph");
                    }
                    std::string msg = "Graph created with " + std::to_string(n) + " vertices and " + std::to_string(m) + " edges\n";
                    send(sender_fd, msg.c_str(), msg.size(), 0);
                } else {
                    std::string msg = "Invalid command for Newgraph\n";
                    send(sender_fd, msg.c_str(), msg.size(), 0);
                }
                

            } else if (command == "Kosaraju") {
                if (ptrGraph) {
                    pthread_mutex_lock(&graph_mutex);
                    handle_kosaraju(sender_fd);
                    pthread_mutex_unlock(&graph_mutex);
                } else {
                    std::string msg = "Graph not initialized.\n";
                    send(sender_fd, msg.c_str(), msg.size(), 0);
                }

            } else if (command == "Newedge") {
                int i, j;
                if (iss >> i >> j && i > 0 && j > 0) {
                    if (ptrGraph) {
                        pthread_mutex_lock(&graph_mutex);
                        bool isEdgeAdded = ptrGraph->addEdge(i - 1, j - 1);
                        pthread_mutex_unlock(&graph_mutex);
                        if(isEdgeAdded){
                            std::string msg = "Edge added between " + std::to_string(i) + " and " + std::to_string(j) + "\n";
                            send(sender_fd, msg.c_str(), msg.size(), 0);
                        }else{
                            std::string msg = "Edge not found, it might be due to other client modification of the graph\n";
                            send(sender_fd, msg.c_str(), msg.size(), 0);
                        }
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
                        bool isEdgeRemoved = ptrGraph->removeEdge(i - 1, j - 1);
                        pthread_mutex_unlock(&graph_mutex);
                        if(isEdgeRemoved){
                             std::string msg = "Edge removed between " + std::to_string(i) + " and " + std::to_string(j) + "\n";
                            send(sender_fd, msg.c_str(), msg.size(), 0);
                        }else{
                            std::string msg = "Edge not found, it might be due to other client modification of the graph\n";
                            send(sender_fd, msg.c_str(), msg.size(), 0);
                        }
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
                std::cout<<"Client Thread Closed"<<std::endl;
                return nullptr;

            } else {
                std::string msg = "Invalid command\n";
                send(sender_fd, msg.c_str(), msg.size(), 0);
            }
        }
        else if (nbytes == 0) {
            printf("pollserver: socket %d hung up\n", sender_fd);
            throw std::runtime_error("Connection closed by client");
        } else {
            perror("recv");
        }
    }
    catch(...){
        removeFdFromReactor(sender_fd);
        close(sender_fd);
        
    }

    return nullptr;
}

int main() {
    signal(SIGINT, handle_sigint); // Register signal handler
    int listener_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listener_fd == -1) {
        perror("socket");
        return 1;
    }

    int enableReuseAddr = 1;
    if (setsockopt(listener_fd, SOL_SOCKET, SO_REUSEADDR, &enableReuseAddr, sizeof(enableReuseAddr)) == -1) {
        perror("setsockopt");
        close(listener_fd);
        return 1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(listener_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        close(listener_fd);
        return 1;
    }

    if (listen(listener_fd, 10) == -1) {
        perror("listen");
        close(listener_fd);
        return 1;
    }

    std::cout << "Server is listening on port 9034" << std::endl;

    // Start the Proactor to accept connections and add them to the Reactor
    std::cout << "Starting Proactor" << std::endl;
    pthread_t proactor_thread = startProactor(listener_fd, handle_client_message);
    
    // Start the Reactor
    std::cout << "Starting Reactor" << std::endl;
    pthread_t* reactor_thread = (pthread_t*)startReactor();
    
    // Create the notify thread
    pthread_t notifyThread;
    pthread_create(&notifyThread, NULL, notify_SCC_signification_change, NULL);


    // Server loop
    std::string stdin_command;
    while (!stop_server) {
        sleep(1);
        if(getline(std::cin, stdin_command)){
            if (stdin_command == "Exit") {
                stop_server = 1;
            }
        }
    }

    pthread_mutex_lock(&graph_mutex);

    // Clean up when done
    std::cout << "Stopping Proactor" << std::endl;
    if(stopProactor(proactor_thread) < 0){
        std::cerr << "Error stopping Proactor" << std::endl;
    }

    // Join the proactor thread to ensure proper cleanup
    if (pthread_join(proactor_thread, nullptr) != 0) {
        perror("Error joining proactor thread");
        return -1;
    }else{
        std::cout<<"Reactor thread joined"<<std::endl;
    }

    // Stop the Reactor
    if (stopReactor() < 0) {
        return -1;
    }
    
    if (pthread_join(*reactor_thread, nullptr) != 0) {
        perror("Error joining reactor thread in stopReactor");
        return -1;
    }else{
        std::cout<<"Reactor thread joined"<<std::endl;
    }

    std::cout << "Server stopped" << std::endl;
    close(listener_fd);

    pthread_mutex_unlock(&graph_mutex);
    delete ptrGraph;

    return 0;
}
